/*
 * File:   RAV_RC_Servo.c
 *
 * Author: Raven Pham
 *
 * Comments:
 * - This file is part of the RAV RC servo driver.
 * - This file provides APIs to control RC servo motors using STM32 timer PWM output.
 * - The PWM timer configuration is expected to be prepared by the user, typically by CubeMX.
 * - This implementation follows an FSP-like structure for consistency and readability.
 *
 * Revision history: Version 0.1
 */

/**********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/

#include "RAV_RC_Servo.h"

/**********************************************************************************************************************
 * Private functions
 **********************************************************************************************************************/

/**
 * @brief Provides a blocking delay for servo timing requirements.
 *
 * @param[in] Time Delay time in milliseconds.
 */
static void RAV_RC_Servo_Delay(uint16_t Time)
{
    //HAL of STM32
    HAL_Delay(Time);
}

/**
 * @brief Maps STM32 HAL status to RAV error code.
 *
 * @param[in] status STM32 HAL status.
 *
 * @retval RAV_SUCCESS          HAL operation was successful.
 * @retval RAV_ERR_TIMEOUT      HAL operation timed out.
 * @retval RAV_ERR_WRITE_FAILED HAL operation failed.
 */
static rav_err_t RAV_RC_Servo_HAL_Status_Map(HAL_StatusTypeDef status)
{
    if (HAL_OK == status)
    {
        return RAV_SUCCESS;
    }

    if (HAL_TIMEOUT == status)
    {
        return RAV_ERR_TIMEOUT;
    }

    return RAV_ERR_WRITE_FAILED;
}

/**
 * @brief Updates the PWM compare value for the RC servo.
 *
 * This function writes the given compare value to the configured timer PWM channel.
 * The compare value determines the output pulse width used to control the servo position.
 *
 * @param[in] Servo Pointer to the RC servo object.
 * @param[in] Pulse PWM compare value to be written to the timer channel.
 *
 * @retval RAV_SUCCESS              PWM compare value was updated successfully.
 * @retval RAV_ERR_INVALID_POINTER  Servo object or timer handle is NULL.
 */
static rav_err_t RAV_RC_Servo_PWM_Output(RAV_RC_Servo_Obj * Servo, uint32_t Pulse)
{
    if ((Servo == NULL) || (Servo->TIM == NULL))
    {
        return RAV_ERR_INVALID_POINTER;
    }

    __HAL_TIM_SET_COMPARE(Servo->TIM, Servo->CHANNEL, Pulse);

    return RAV_SUCCESS;
}

/**********************************************************************************************************************
 * Public functions
 **********************************************************************************************************************/

/**
 * @brief Initializes an RC servo object and starts PWM output.
 *
 * This function stores the timer handle, PWM channel, servo period, angle range,
 * and pulse width range into the RC servo object. It starts PWM output and moves
 * the servo to the configured home angle.
 *
 * @param[in,out] Servo       Pointer to the RC servo object.
 * @param[in]     TIM         Pointer to the STM32 HAL timer handle.
 * @param[in]     Channel     Timer PWM channel. Use STM32 HAL channel macros such as TIM_CHANNEL_1.
 * @param[in]     TimerPeriod Servo PWM period in milliseconds, typically 20 ms.
 * @param[in]     MaxAngle    Maximum supported servo angle in degrees.
 * @param[in]     HomeAngle   Default home angle in degrees.
 * @param[in]     MaxPulse    Compare value corresponding to MaxAngle.
 * @param[in]     MinPulse    Compare value corresponding to 0 degrees.
 *
 * @retval RAV_SUCCESS              Servo was initialized successfully.
 * @retval RAV_ERR_INVALID_POINTER  Servo object or timer handle is NULL.
 * @retval RAV_ERR_INVALID_ARGUMENT Invalid configuration parameter.
 * @retval RAV_ERR_TIMEOUT          PWM start operation timed out.
 * @retval RAV_ERR_WRITE_FAILED     PWM start operation failed.
 */
rav_err_t RAV_RC_Servo_Init(RAV_RC_Servo_Obj * Servo,
                            TIM_HandleTypeDef * TIM,
                            uint32_t Channel,
                            uint32_t TimerPeriod,
                            uint8_t MaxAngle,
                            uint8_t HomeAngle,
                            uint32_t MaxPulse,
                            uint32_t MinPulse)
{
    rav_err_t err;
    HAL_StatusTypeDef hal_status;

    if ((Servo == NULL) || (TIM == NULL))
    {
        return RAV_ERR_INVALID_POINTER;
    }

    if ((TimerPeriod == 0U) ||
        (MaxAngle == 0U) ||
        (HomeAngle > MaxAngle) ||
        (MaxPulse <= MinPulse))
    {
        return RAV_ERR_INVALID_ARGUMENT;
    }

    Servo->TIM          = TIM;
    Servo->CHANNEL      = Channel;
    Servo->TIMER_PERIOD = TimerPeriod;
    Servo->MAX_ANGLE    = MaxAngle;
    Servo->HOME_ANGLE   = HomeAngle;
    Servo->MAX_PULSE    = MaxPulse;
    Servo->MIN_PULSE    = MinPulse;

    hal_status = HAL_TIM_PWM_Start(Servo->TIM, Servo->CHANNEL);
    err = RAV_RC_Servo_HAL_Status_Map(hal_status);
    if (RAV_SUCCESS != err)
    {
        return err;
    }

    RAV_RC_Servo_Delay(100U);

    err = RAV_RC_Servo_Rotate(Servo, Servo->HOME_ANGLE);
    if (RAV_SUCCESS != err)
    {
        return err;
    }

    return RAV_SUCCESS;
}

/**
 * @brief Rotates the RC servo to the specified angle.
 *
 * This function converts the target angle to a PWM compare value and updates
 * the timer compare register for the configured PWM channel.
 *
 * @param[in,out] Servo Pointer to the RC servo object.
 * @param[in]     Angle Target angle in degrees.
 *
 * @retval RAV_SUCCESS              Servo angle was updated successfully.
 * @retval RAV_ERR_INVALID_POINTER  Servo object or timer handle is NULL.
 * @retval RAV_ERR_INVALID_ARGUMENT Target angle or servo configuration is invalid.
 */
rav_err_t RAV_RC_Servo_Rotate(RAV_RC_Servo_Obj * Servo, uint8_t Angle)
{
    uint32_t pulse;
    uint32_t delta_pulse;

    if ((Servo == NULL) || (Servo->TIM == NULL))
    {
        return RAV_ERR_INVALID_POINTER;
    }

    if ((Servo->MAX_ANGLE == 0U) ||
        (Servo->MAX_PULSE <= Servo->MIN_PULSE))
    {
        return RAV_ERR_INVALID_ARGUMENT;
    }

    if (Angle > Servo->MAX_ANGLE)
    {
        return RAV_ERR_INVALID_ARGUMENT;
    }

    delta_pulse = Servo->MAX_PULSE - Servo->MIN_PULSE;

    pulse = (uint32_t)((((uint64_t)Angle * (uint64_t)delta_pulse) /
                        (uint64_t)Servo->MAX_ANGLE) +
                        Servo->MIN_PULSE);

    return RAV_RC_Servo_PWM_Output(Servo, pulse);
}