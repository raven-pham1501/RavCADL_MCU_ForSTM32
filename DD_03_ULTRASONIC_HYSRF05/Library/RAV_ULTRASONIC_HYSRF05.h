/*
 * File:   RAV_Ultrasonic.h
 *
 * Author: Raven Pham
 *
 * Comments:
 * - This file is part of the RAV ultrasonic sensor driver.
 * - This file provides definitions and public APIs for measuring distance
 *   using ultrasonic modules such as HY-SRF05, SRF05, or HC-SR04.
 * - This implementation uses STM32 timer input capture to measure the ECHO pulse width.
 * - This implementation follows an FSP-like structure for consistency and readability.
 *
 * Revision history: Version 0.1
 */

#ifndef _RAV_ULTRASONIC_HYSRF05_H
#define _RAV_ULTRASONIC_HYSRF05_H

/**********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/

#include "RAV_Common_Porting.h"
#include "RAV_Common_Define.h"

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/** Trigger pulse width in microseconds. */
#define RAV_ULTRASONIC_TRIGGER_PULSE_US           (10U)

/** Default measurement timeout in microseconds. */
#define RAV_ULTRASONIC_DEFAULT_TIMEOUT_US         (30000U)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/**
 * @brief Ultrasonic measurement state.
 */
typedef enum e_rav_ultrasonic_state
{
    RAV_ULTRASONIC_STATE_IDLE = 0,
    RAV_ULTRASONIC_STATE_WAIT_RISING,
    RAV_ULTRASONIC_STATE_WAIT_FALLING,
    RAV_ULTRASONIC_STATE_DONE,
    RAV_ULTRASONIC_STATE_TIMEOUT,
    RAV_ULTRASONIC_STATE_ERROR,
} rav_ultrasonic_state_t;

/**
 * @brief Ultrasonic sensor driver object.
 *
 * This structure stores the trigger GPIO pin, timer input capture configuration,
 * captured values, and measurement state.
 *
 * @note The input capture timer should be configured so that 1 timer tick = 1 us.
 */
typedef struct
{
    /** Trigger GPIO port. */
    GPIO_TypeDef * TRIG_PORT;

    /** Trigger GPIO pin. */
    uint16_t TRIG_PIN;

    /** Pointer to STM32 HAL timer handle used for input capture. */
    TIM_HandleTypeDef * TIM;

    /** Timer input capture channel. Use STM32 HAL channel macros such as TIM_CHANNEL_1. */
    uint32_t CHANNEL;

    /** Measurement timeout in microseconds. */
    uint32_t TIMEOUT_US;

    /** Captured timer value at ECHO rising edge. */
    volatile uint32_t RISING_CAPTURE;

    /** Captured timer value at ECHO falling edge. */
    volatile uint32_t FALLING_CAPTURE;

    /** Measured ECHO pulse width in microseconds. */
    volatile uint32_t ECHO_TIME_US;

    /** Current measurement state. */
    volatile rav_ultrasonic_state_t STATE;
} RAV_Ultrasonic_Obj;

/**********************************************************************************************************************
 * Public API function prototypes
 **********************************************************************************************************************/

RAV_HEADER

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
                              uint32_t TimeoutUs);

/**
 * @brief Starts a non-blocking ultrasonic measurement using timer input capture interrupt.
 *
 * This function starts the input capture interrupt, sends a trigger pulse, and waits
 * for the capture callback to complete the measurement.
 *
 * @param[in,out] Sensor Pointer to the ultrasonic sensor object.
 *
 * @retval RAV_SUCCESS              Measurement was started successfully.
 * @retval RAV_ERR_INVALID_POINTER  Sensor object, GPIO port, or timer handle is NULL.
 * @retval RAV_ERR_IN_USE           A measurement is already in progress.
 * @retval RAV_ERR_WRITE_FAILED     Input capture could not be started.
 */
rav_err_t RAV_Ultrasonic_Start_IT(RAV_Ultrasonic_Obj * Sensor);

/**
 * @brief Handles timer input capture callback for ultrasonic measurement.
 *
 * This function should be called from HAL_TIM_IC_CaptureCallback().
 *
 * @param[in,out] Sensor Pointer to the ultrasonic sensor object.
 * @param[in]     htim   Pointer to STM32 HAL timer handle from HAL callback.
 *
 * @retval RAV_SUCCESS              Callback was handled successfully.
 * @retval RAV_ERR_INVALID_POINTER  Sensor object or timer handle is NULL.
 * @retval RAV_ERR_INVALID_ARGUMENT Callback does not belong to this sensor.
 */
rav_err_t RAV_Ultrasonic_TIM_IC_Callback(RAV_Ultrasonic_Obj * Sensor, TIM_HandleTypeDef * htim);

/**
 * @brief Checks measurement timeout.
 *
 * This function should be called periodically while a measurement is in progress.
 *
 * @param[in,out] Sensor Pointer to the ultrasonic sensor object.
 *
 * @retval RAV_SUCCESS              No timeout occurred.
 * @retval RAV_ERR_INVALID_POINTER  Sensor object or timer handle is NULL.
 * @retval RAV_ERR_TIMEOUT          Measurement timed out.
 */
rav_err_t RAV_Ultrasonic_CheckTimeout(RAV_Ultrasonic_Obj * Sensor);

/**
 * @brief Checks whether the measurement is complete.
 *
 * @param[in] Sensor Pointer to the ultrasonic sensor object.
 *
 * @retval true  Measurement is complete.
 * @retval false Measurement is not complete.
 */
bool RAV_Ultrasonic_IsDone(RAV_Ultrasonic_Obj * Sensor);

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
rav_err_t RAV_Ultrasonic_GetEchoTimeUs(RAV_Ultrasonic_Obj * Sensor, uint32_t * EchoTimeUs);

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
rav_err_t RAV_Ultrasonic_GetDistanceMm(RAV_Ultrasonic_Obj * Sensor, uint32_t * DistanceMm);

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
rav_err_t RAV_Ultrasonic_GetDistanceCm(RAV_Ultrasonic_Obj * Sensor, uint32_t * DistanceCm);

RAV_FOOTER

#endif /* _RAV_ULTRASONIC_HYSRF05_H */