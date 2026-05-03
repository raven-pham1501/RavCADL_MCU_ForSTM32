/*
 * File:   RAV_RC_Servo.h
 *
 * Author: Raven Pham
 *
 * Comments:
 * - This file is part of the RAV RC servo driver.
 * - This file provides definitions and public APIs for controlling RC servo motors
 *   using STM32 timer PWM output.
 * - This implementation follows an FSP-like structure for consistency and readability.
 *
 * Revision history: Version 0.1
 */

#ifndef RAV_RC_SERVO_H
#define RAV_RC_SERVO_H

/**********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/

#include "RAV_Common_Porting.h"
#include "RAV_Common_Define.h"

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/**
 * @brief RC servo driver object.
 *
 * This structure stores the timer handle, PWM channel, servo angle range,
 * and pulse width range used to control an RC servo motor.
 *
 * @note The PWM timer period should be configured by the user, typically by CubeMX.
 *       This driver does not calculate or validate the timer clock configuration.
 */
typedef struct
{
    /** Pointer to STM32 HAL timer handle used for PWM generation. */
    TIM_HandleTypeDef * TIM;

    /** Timer PWM output channel. Use STM32 HAL channel macros such as TIM_CHANNEL_1. */
    uint32_t CHANNEL;

    /** Servo PWM period in milliseconds. This value is stored for reference. */
    uint32_t TIMER_PERIOD;

    /** Maximum supported servo angle in degrees. */
    uint8_t MAX_ANGLE;

    /** Default home angle in degrees. */
    uint8_t HOME_ANGLE;

    /** Pulse width compare value corresponding to the maximum angle. */
    uint32_t MAX_PULSE;

    /** Pulse width compare value corresponding to 0 degrees. */
    uint32_t MIN_PULSE;
} RAV_RC_Servo_Obj;

/**********************************************************************************************************************
 * Public API function prototypes
 **********************************************************************************************************************/

RAV_HEADER

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
                            uint32_t MinPulse);

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
rav_err_t RAV_RC_Servo_Rotate(RAV_RC_Servo_Obj * Servo, uint8_t Angle);

RAV_FOOTER

#endif /* RAV_RC_SERVO_H */