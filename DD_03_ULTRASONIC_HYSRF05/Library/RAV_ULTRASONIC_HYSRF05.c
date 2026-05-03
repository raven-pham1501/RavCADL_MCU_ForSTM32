/*
 * File:   RAV_Ultrasonic.c
 *
 * Author: Raven Pham
 *
 * Comments:
 * - This file is part of the RAV ultrasonic sensor driver.
 * - This file provides APIs for measuring distance using ultrasonic modules
 *   such as HY-SRF05, SRF05, or HC-SR04.
 * - The ECHO pulse width is measured using STM32 timer input capture interrupt.
 * - This implementation follows an FSP-like structure for consistency and readability.
 *
 * Revision history: Version 0.1
 */

/**********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/

#include "RAV_ULTRASONIC_HYSRF05.h"

/**********************************************************************************************************************
 * Private functions
 **********************************************************************************************************************/

/**
 * @brief Converts timer channel macro to HAL active channel value.
 *
 * @param[in] Channel Timer channel macro.
 *
 * @return HAL active channel value.
 */
static HAL_TIM_ActiveChannel RAV_Ultrasonic_Channel_To_Active(uint32_t Channel)
{
    switch (Channel)
    {
        case TIM_CHANNEL_1:
            return HAL_TIM_ACTIVE_CHANNEL_1;

        case TIM_CHANNEL_2:
            return HAL_TIM_ACTIVE_CHANNEL_2;

        case TIM_CHANNEL_3:
            return HAL_TIM_ACTIVE_CHANNEL_3;

        case TIM_CHANNEL_4:
            return HAL_TIM_ACTIVE_CHANNEL_4;

        default:
            return HAL_TIM_ACTIVE_CHANNEL_CLEARED;
    }
}

/**
 * @brief Reads captured timer value from the configured input capture channel.
 *
 * @param[in] Sensor Pointer to the ultrasonic sensor object.
 *
 * @return Captured timer value.
 */
static uint32_t RAV_Ultrasonic_ReadCapturedValue(RAV_Ultrasonic_Obj * Sensor)
{
    return HAL_TIM_ReadCapturedValue(Sensor->TIM, Sensor->CHANNEL);
}

/**
 * @brief Calculates elapsed timer ticks with overflow handling.
 *
 * @param[in] Sensor Pointer to the ultrasonic sensor object.
 * @param[in] Start  Start counter value.
 * @param[in] End    End counter value.
 *
 * @return Elapsed ticks.
 */
static uint32_t RAV_Ultrasonic_CalculateElapsed(RAV_Ultrasonic_Obj * Sensor, uint32_t Start, uint32_t End)
{
    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(Sensor->TIM);
    uint32_t elapsed;

    if (End >= Start)
    {
        elapsed = End - Start;
    }
    else
    {
        elapsed = (arr - Start) + End + 1U;
    }

    return elapsed;
}

/**
 * @brief Provides a blocking delay in microseconds using the running input capture timer.
 *
 * @param[in] Sensor Pointer to the ultrasonic sensor object.
 * @param[in] TimeUs Delay time in microseconds.
 *
 * @retval RAV_SUCCESS              Delay completed successfully.
 * @retval RAV_ERR_INVALID_POINTER  Sensor object or timer handle is NULL.
 */
static rav_err_t RAV_Ultrasonic_DelayUs(RAV_Ultrasonic_Obj * Sensor, uint32_t TimeUs)
{
    uint32_t start;
    uint32_t now;
    uint32_t elapsed;

    if ((Sensor == NULL) || (Sensor->TIM == NULL))
    {
        return RAV_ERR_INVALID_POINTER;
    }

    start = __HAL_TIM_GET_COUNTER(Sensor->TIM);

    do
    {
        now = __HAL_TIM_GET_COUNTER(Sensor->TIM);
        elapsed = RAV_Ultrasonic_CalculateElapsed(Sensor, start, now);
    } while (elapsed < TimeUs);

    return RAV_SUCCESS;
}

/**
 * @brief Sends a trigger pulse to the ultrasonic sensor.
 *
 * @param[in] Sensor Pointer to the ultrasonic sensor object.
 *
 * @retval RAV_SUCCESS              Trigger pulse was sent successfully.
 * @retval RAV_ERR_INVALID_POINTER  Sensor object, GPIO port, or timer handle is NULL.
 */
static rav_err_t RAV_Ultrasonic_SendTrigger(RAV_Ultrasonic_Obj * Sensor)
{
    rav_err_t err;

    if ((Sensor == NULL) || (Sensor->TRIG_PORT == NULL) || (Sensor->TIM == NULL))
    {
        return RAV_ERR_INVALID_POINTER;
    }

    HAL_GPIO_WritePin(Sensor->TRIG_PORT, Sensor->TRIG_PIN, GPIO_PIN_RESET);

    err = RAV_Ultrasonic_DelayUs(Sensor, 2U);
    if (RAV_SUCCESS != err)
    {
        return err;
    }

    HAL_GPIO_WritePin(Sensor->TRIG_PORT, Sensor->TRIG_PIN, GPIO_PIN_SET);

    err = RAV_Ultrasonic_DelayUs(Sensor, RAV_ULTRASONIC_TRIGGER_PULSE_US);
    if (RAV_SUCCESS != err)
    {
        return err;
    }

    HAL_GPIO_WritePin(Sensor->TRIG_PORT, Sensor->TRIG_PIN, GPIO_PIN_RESET);

    return RAV_SUCCESS;
}

/**********************************************************************************************************************
 * Public functions
 **********************************************************************************************************************/

/**
 * @brief Initializes an ultrasonic sensor object.
 *
 * @param[in,out] Sensor    Pointer to the ultrasonic sensor object.
 * @param[in]     TrigPort  Trigger GPIO port.
 * @param[in]     TrigPin   Trigger GPIO pin.
 * @param[in]     TIM       Pointer to STM32 HAL timer handle used for input capture.
 * @param[in]     Channel   Timer input capture channel.
 * @param[in]     TimeoutUs Measurement timeout in microseconds.
 *
 * @retval RAV_SUCCESS              Sensor object was initialized successfully.
 * @retval RAV_ERR_INVALID_POINTER  Sensor object, GPIO port, or timer handle is NULL.
 * @retval RAV_ERR_INVALID_ARGUMENT Invalid timeout value.
 */
rav_err_t RAV_Ultrasonic_Init(RAV_Ultrasonic_Obj * Sensor,
                              GPIO_TypeDef * TrigPort,
                              uint16_t TrigPin,
                              TIM_HandleTypeDef * TIM,
                              uint32_t Channel,
                              uint32_t TimeoutUs)
{
    if ((Sensor == NULL) || (TrigPort == NULL) || (TIM == NULL))
    {
        return RAV_ERR_INVALID_POINTER;
    }

    if (0U == TimeoutUs)
    {
        return RAV_ERR_INVALID_ARGUMENT;
    }

    Sensor->TRIG_PORT       = TrigPort;
    Sensor->TRIG_PIN        = TrigPin;
    Sensor->TIM             = TIM;
    Sensor->CHANNEL         = Channel;
    Sensor->TIMEOUT_US      = TimeoutUs;
    Sensor->RISING_CAPTURE  = 0U;
    Sensor->FALLING_CAPTURE = 0U;
    Sensor->ECHO_TIME_US    = 0U;
    Sensor->STATE           = RAV_ULTRASONIC_STATE_IDLE;

    HAL_GPIO_WritePin(Sensor->TRIG_PORT, Sensor->TRIG_PIN, GPIO_PIN_RESET);

    return RAV_SUCCESS;
}

/**
 * @brief Starts a non-blocking ultrasonic measurement using timer input capture interrupt.
 *
 * @param[in,out] Sensor Pointer to the ultrasonic sensor object.
 *
 * @retval RAV_SUCCESS              Measurement was started successfully.
 * @retval RAV_ERR_INVALID_POINTER  Sensor object, GPIO port, or timer handle is NULL.
 * @retval RAV_ERR_IN_USE           A measurement is already in progress.
 * @retval RAV_ERR_WRITE_FAILED     Input capture could not be started.
 */
rav_err_t RAV_Ultrasonic_Start_IT(RAV_Ultrasonic_Obj * Sensor)
{
    HAL_StatusTypeDef hal_status;
    rav_err_t err;

    if ((Sensor == NULL) || (Sensor->TRIG_PORT == NULL) || (Sensor->TIM == NULL))
    {
        return RAV_ERR_INVALID_POINTER;
    }

    if ((RAV_ULTRASONIC_STATE_WAIT_RISING == Sensor->STATE) ||
        (RAV_ULTRASONIC_STATE_WAIT_FALLING == Sensor->STATE))
    {
        return RAV_ERR_IN_USE;
    }

    Sensor->RISING_CAPTURE  = 0U;
    Sensor->FALLING_CAPTURE = 0U;
    Sensor->ECHO_TIME_US    = 0U;
    Sensor->STATE           = RAV_ULTRASONIC_STATE_WAIT_RISING;

    __HAL_TIM_SET_COUNTER(Sensor->TIM, 0U);
    __HAL_TIM_SET_CAPTUREPOLARITY(Sensor->TIM, Sensor->CHANNEL, TIM_INPUTCHANNELPOLARITY_RISING);

    hal_status = HAL_TIM_IC_Start_IT(Sensor->TIM, Sensor->CHANNEL);
    if (HAL_OK != hal_status)
    {
        Sensor->STATE = RAV_ULTRASONIC_STATE_ERROR;
        return RAV_ERR_WRITE_FAILED;
    }

    err = RAV_Ultrasonic_SendTrigger(Sensor);
    if (RAV_SUCCESS != err)
    {
        Sensor->STATE = RAV_ULTRASONIC_STATE_ERROR;
        HAL_TIM_IC_Stop_IT(Sensor->TIM, Sensor->CHANNEL);
        return err;
    }

    return RAV_SUCCESS;
}

/**
 * @brief Handles timer input capture callback for ultrasonic measurement.
 *
 * @param[in,out] Sensor Pointer to the ultrasonic sensor object.
 * @param[in]     htim   Pointer to STM32 HAL timer handle from HAL callback.
 *
 * @retval RAV_SUCCESS              Callback was handled successfully.
 * @retval RAV_ERR_INVALID_POINTER  Sensor object or timer handle is NULL.
 * @retval RAV_ERR_INVALID_ARGUMENT Callback does not belong to this sensor.
 */
rav_err_t RAV_Ultrasonic_TIM_IC_Callback(RAV_Ultrasonic_Obj * Sensor, TIM_HandleTypeDef * htim)
{
    HAL_TIM_ActiveChannel active_channel;
    uint32_t captured_value;

    if ((Sensor == NULL) || (Sensor->TIM == NULL) || (htim == NULL))
    {
        return RAV_ERR_INVALID_POINTER;
    }

    if (htim != Sensor->TIM)
    {
        return RAV_ERR_INVALID_ARGUMENT;
    }

    active_channel = RAV_Ultrasonic_Channel_To_Active(Sensor->CHANNEL);

    if (htim->Channel != active_channel)
    {
        return RAV_ERR_INVALID_ARGUMENT;
    }

    captured_value = RAV_Ultrasonic_ReadCapturedValue(Sensor);

    if (RAV_ULTRASONIC_STATE_WAIT_RISING == Sensor->STATE)
    {
        Sensor->RISING_CAPTURE = captured_value;
        Sensor->STATE = RAV_ULTRASONIC_STATE_WAIT_FALLING;

        __HAL_TIM_SET_CAPTUREPOLARITY(Sensor->TIM, Sensor->CHANNEL, TIM_INPUTCHANNELPOLARITY_FALLING);

        return RAV_SUCCESS;
    }

    if (RAV_ULTRASONIC_STATE_WAIT_FALLING == Sensor->STATE)
    {
        Sensor->FALLING_CAPTURE = captured_value;
        Sensor->ECHO_TIME_US = RAV_Ultrasonic_CalculateElapsed(Sensor,
                                                               Sensor->RISING_CAPTURE,
                                                               Sensor->FALLING_CAPTURE);

        Sensor->STATE = RAV_ULTRASONIC_STATE_DONE;

        HAL_TIM_IC_Stop_IT(Sensor->TIM, Sensor->CHANNEL);
        __HAL_TIM_SET_CAPTUREPOLARITY(Sensor->TIM, Sensor->CHANNEL, TIM_INPUTCHANNELPOLARITY_RISING);

        return RAV_SUCCESS;
    }

    return RAV_SUCCESS;
}

/**
 * @brief Checks measurement timeout.
 *
 * @param[in,out] Sensor Pointer to the ultrasonic sensor object.
 *
 * @retval RAV_SUCCESS              No timeout occurred.
 * @retval RAV_ERR_INVALID_POINTER  Sensor object or timer handle is NULL.
 * @retval RAV_ERR_TIMEOUT          Measurement timed out.
 */
rav_err_t RAV_Ultrasonic_CheckTimeout(RAV_Ultrasonic_Obj * Sensor)
{
    uint32_t counter;

    if ((Sensor == NULL) || (Sensor->TIM == NULL))
    {
        return RAV_ERR_INVALID_POINTER;
    }

    if ((RAV_ULTRASONIC_STATE_WAIT_RISING != Sensor->STATE) &&
        (RAV_ULTRASONIC_STATE_WAIT_FALLING != Sensor->STATE))
    {
        return RAV_SUCCESS;
    }

    counter = __HAL_TIM_GET_COUNTER(Sensor->TIM);

    if (counter >= Sensor->TIMEOUT_US)
    {
        Sensor->STATE = RAV_ULTRASONIC_STATE_TIMEOUT;
        HAL_TIM_IC_Stop_IT(Sensor->TIM, Sensor->CHANNEL);
        __HAL_TIM_SET_CAPTUREPOLARITY(Sensor->TIM, Sensor->CHANNEL, TIM_INPUTCHANNELPOLARITY_RISING);

        return RAV_ERR_TIMEOUT;
    }

    return RAV_SUCCESS;
}

/**
 * @brief Checks whether the measurement is complete.
 *
 * @param[in] Sensor Pointer to the ultrasonic sensor object.
 *
 * @retval true  Measurement is complete.
 * @retval false Measurement is not complete.
 */
bool RAV_Ultrasonic_IsDone(RAV_Ultrasonic_Obj * Sensor)
{
    if (Sensor == NULL)
    {
        return false;
    }

    return (RAV_ULTRASONIC_STATE_DONE == Sensor->STATE);
}

/**
 * @brief Gets measured echo time in microseconds.
 *
 * @param[in]  Sensor     Pointer to the ultrasonic sensor object.
 * @param[out] EchoTimeUs Pointer to store echo pulse width in microseconds.
 *
 * @retval RAV_SUCCESS              Echo time was returned successfully.
 * @retval RAV_ERR_INVALID_POINTER  Sensor object or output pointer is NULL.
 * @retval RAV_ERR_NOT_ENABLED      Measurement is not complete.
 */
rav_err_t RAV_Ultrasonic_GetEchoTimeUs(RAV_Ultrasonic_Obj * Sensor, uint32_t * EchoTimeUs)
{
    if ((Sensor == NULL) || (EchoTimeUs == NULL))
    {
        return RAV_ERR_INVALID_POINTER;
    }

    if (RAV_ULTRASONIC_STATE_DONE != Sensor->STATE)
    {
        return RAV_ERR_NOT_ENABLED;
    }

    *EchoTimeUs = Sensor->ECHO_TIME_US;

    return RAV_SUCCESS;
}

/**
 * @brief Gets measured distance in millimeters.
 *
 * @param[in]  Sensor     Pointer to the ultrasonic sensor object.
 * @param[out] DistanceMm Pointer to store measured distance in millimeters.
 *
 * @retval RAV_SUCCESS              Distance was returned successfully.
 * @retval RAV_ERR_INVALID_POINTER  Sensor object or output pointer is NULL.
 * @retval RAV_ERR_NOT_ENABLED      Measurement is not complete.
 */
rav_err_t RAV_Ultrasonic_GetDistanceMm(RAV_Ultrasonic_Obj * Sensor, uint32_t * DistanceMm)
{
    rav_err_t err;
    uint32_t echo_time_us;

    if (DistanceMm == NULL)
    {
        return RAV_ERR_INVALID_POINTER;
    }

    err = RAV_Ultrasonic_GetEchoTimeUs(Sensor, &echo_time_us);
    if (RAV_SUCCESS != err)
    {
        return err;
    }

    *DistanceMm = (echo_time_us * 10U) / 58U;

    return RAV_SUCCESS;
}

/**
 * @brief Gets measured distance in centimeters.
 *
 * @param[in]  Sensor     Pointer to the ultrasonic sensor object.
 * @param[out] DistanceCm Pointer to store measured distance in centimeters.
 *
 * @retval RAV_SUCCESS              Distance was returned successfully.
 * @retval RAV_ERR_INVALID_POINTER  Sensor object or output pointer is NULL.
 * @retval RAV_ERR_NOT_ENABLED      Measurement is not complete.
 */
rav_err_t RAV_Ultrasonic_GetDistanceCm(RAV_Ultrasonic_Obj * Sensor, uint32_t * DistanceCm)
{
    rav_err_t err;
    uint32_t echo_time_us;

    if (DistanceCm == NULL)
    {
        return RAV_ERR_INVALID_POINTER;
    }

    err = RAV_Ultrasonic_GetEchoTimeUs(Sensor, &echo_time_us);
    if (RAV_SUCCESS != err)
    {
        return err;
    }

    *DistanceCm = echo_time_us / 58U;

    return RAV_SUCCESS;
}