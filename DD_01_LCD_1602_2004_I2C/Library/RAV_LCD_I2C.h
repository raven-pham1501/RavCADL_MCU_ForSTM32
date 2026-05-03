/*
 * File:   RAV_LCD_I2C.h
 *
 * Author: Raven Pham
 *
 * Comments:
 * - This file is part of the RAV LCD I2C driver.
 * - This file provides definitions and public APIs for controlling
 *   HD44780-compatible character LCD modules through an I2C backpack.
 * - The I2C backpack is typically based on a GPIO expander such as PCF8574.
 * - This file follows an FSP-like structure for consistency and readability.
 *
 * Revision history: Version 0.1
 */

#ifndef _RAV_LCD_I2C_H
#define _RAV_LCD_I2C_H

/**********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/

#include "RAV_Common_Porting.h"
#include "RAV_Common_Define.h"

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/** LCD transfer mode: command instruction. */
#define CLCD_COMMAND 				(0x00)

/** LCD transfer mode: display data. */
#define CLCD_DATA 					(0x01)

/** LCD enable control bit. */
#define LCD_EN 						(0x04)  

/** LCD read/write control bit. This driver only uses write mode. */
#define LCD_RW 						(0x02)  

/** LCD register select control bit. */
#define LCD_RS 						(0x01)

/** Clear display command. */
#define LCD_CLEARDISPLAY 			(0x01)

/** Return cursor to home position command. */
#define LCD_RETURNHOME 				(0x02)

/** Entry mode set command base. */
#define LCD_ENTRYMODESET 			(0x04)

/** Display control command base. */
#define LCD_DISPLAYCONTROL 	    	(0x08)

/** Cursor or display shift command base. */
#define LCD_CURSORSHIFT 			(0x10)

/** Function set command base. */
#define LCD_FUNCTIONSET 			(0x20)

/** Set CGRAM address command base. */
#define LCD_SETCGRAMADDR 			(0x40)

/** Set DDRAM address command base. */
#define LCD_SETDDRAMADDR	 		(0x80)

/** Entry mode: decrement cursor position after each character. */
#define LCD_ENTRYRIGHT 				(0x00)

/** Entry mode: increment cursor position after each character. */
#define LCD_ENTRYLEFT 				(0x02)

/** Entry mode: shift display after each character. */
#define LCD_ENTRYSHIFTINCREMENT 	(0x01)

/** Entry mode: do not shift display after each character. */
#define LCD_ENTRYSHIFTDECREMENT 	(0x00)

/** Display control: turn display on. */
#define LCD_DISPLAYON 				(0x04)

/** Display control: turn display off. */
#define LCD_DISPLAYOFF 				(0x00)

/** Display control: show cursor. */
#define LCD_CURSORON 			  	(0x02)

/** Display control: hide cursor. */
#define LCD_CURSOROFF 				(0x00)

/** Display control: enable cursor blinking. */
#define LCD_BLINKON 			 	(0x01)

/** Display control: disable cursor blinking. */
#define LCD_BLINKOFF 				(0x00)

/** Shift control: move entire display. */
#define LCD_DISPLAYMOVE             (0x08U)

/** Shift control: move cursor only. */
#define LCD_CURSORMOVE              (0x00U)

/** Shift control: move right. */
#define LCD_MOVERIGHT               (0x04U)

/** Shift control: move left. */
#define LCD_MOVELEFT                (0x00U)

/** Function set: 8-bit interface mode. */
#define LCD_8BITMODE                (0x10U)

/** Function set: 4-bit interface mode. */
#define LCD_4BITMODE                (0x00U)

/** Function set: 2-line display mode. */
#define LCD_2LINE                   (0x08U)

/** Function set: 1-line display mode. */
#define LCD_1LINE                   (0x00U)

/** Function set: 5x10 dot character font. */
#define LCD_5x10DOTS                (0x04U)

/** Function set: 5x8 dot character font. */
#define LCD_5x8DOTS                 (0x00U)

/** Backlight control: turn backlight on. */
#define LCD_BACKLIGHT               (0x08U)

/** Backlight control: turn backlight off. */
#define LCD_NOBACKLIGHT             (0x00U)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/**
 * @brief LCD I2C driver object.
 *
 * This structure stores the LCD I2C handle, display configuration, and command state.
 * The user should create one object for each LCD instance.
 */
typedef struct
{
    /** Pointer to STM32 HAL I2C handle. */
    I2C_HandleTypeDef * I2C;

    /** LCD I2C address shifted left by 1 bit for STM32 HAL API usage. */
    uint8_t ADDRESS;

    /** Number of display columns. */
    uint8_t COLUMNS;

    /** Number of display rows. */
    uint8_t ROWS;

    /** Current entry mode command value. */
    uint8_t ENTRYMODE;

    /** Current display control command value. */
    uint8_t DISPLAYCTRL;

    /** Current cursor shift command value. */
    uint8_t CURSORSHIFT;

    /** Current function set command value. */
    uint8_t FUNCTIONSET;

    /** Current backlight state. */
    uint8_t BACKLIGHT;
} RAV_LCD_I2C_Obj;

/**********************************************************************************************************************
 * Public API function prototypes
 **********************************************************************************************************************/

RAV_HEADER

 /**
 * @brief Initializes the LCD module in 4-bit mode over I2C.
 *
 * This function configures the LCD object, enables the backlight by default,
 * and sends the initialization command sequence required by HD44780-compatible LCDs.
 *
 * @param[in,out] LCD       Pointer to the LCD I2C object.
 * @param[in]     hi2c_CLCD Pointer to the STM32 HAL I2C handle.
 * @param[in]     Address   7-bit I2C address of the LCD backpack.
 * @param[in]     Columns   Number of display columns.
 * @param[in]     Rows      Number of display rows.
 *
 * @retval RAV_SUCCESS              LCD was initialized successfully.
 * @retval RAV_ERR_INVALID_POINTER  LCD object or I2C handle is NULL.
 * @retval RAV_ERR_TIMEOUT          I2C transmission timed out.
 * @retval RAV_ERR_WRITE_FAILED     I2C transmission failed.
 */
rav_err_t RAV_LCD_I2C_Init(RAV_LCD_I2C_Obj* LCD, I2C_HandleTypeDef* hi2c_CLCD, uint8_t Address, uint8_t Columns, uint8_t Rows);

/**
 * @brief Sets the LCD cursor position.
 *
 * The input position is clamped to the configured display size. The function maps
 * the row and column position to the corresponding HD44780 DDRAM address.
 *
 * @param[in] LCD  Pointer to the LCD I2C object.
 * @param[in] Xpos Cursor column position, starting from 0.
 * @param[in] Ypos Cursor row position, starting from 0.
 *
 * @retval RAV_SUCCESS              Cursor position was updated successfully.
 * @retval RAV_ERR_INVALID_POINTER  LCD object or I2C handle is NULL.
 * @retval RAV_ERR_INVALID_ARGUMENT Unsupported row position.
 * @retval RAV_ERR_TIMEOUT          I2C transmission timed out.
 * @retval RAV_ERR_WRITE_FAILED     I2C transmission failed.
 */
rav_err_t RAV_LCD_I2C_SetCursor(RAV_LCD_I2C_Obj* LCD, uint8_t Xpos, uint8_t YPos);

/**
 * @brief Writes a single character to the LCD.
 *
 * @param[in] LCD       Pointer to the LCD I2C object.
 * @param[in] character Character to be displayed.
 *
 * @retval RAV_SUCCESS              Character was written successfully.
 * @retval RAV_ERR_INVALID_POINTER  LCD object or I2C handle is NULL.
 * @retval RAV_ERR_INVALID_ARGUMENT Invalid transfer mode was detected.
 * @retval RAV_ERR_TIMEOUT          I2C transmission timed out.
 * @retval RAV_ERR_WRITE_FAILED     I2C transmission failed.
 */
rav_err_t RAV_LCD_I2C_WriteChar(RAV_LCD_I2C_Obj* LCD, const char character);

/**
 * @brief Writes a null-terminated string to the LCD.
 *
 * Characters are sent one by one until the null terminator is reached.
 * The LCD cursor advances automatically according to the configured entry mode.
 *
 * @param[in] LCD    Pointer to the LCD I2C object.
 * @param[in] String Pointer to a null-terminated string.
 *
 * @retval RAV_SUCCESS              String was written successfully.
 * @retval RAV_ERR_INVALID_POINTER  LCD object, I2C handle, or string pointer is NULL.
 * @retval RAV_ERR_INVALID_ARGUMENT Invalid transfer mode was detected.
 * @retval RAV_ERR_TIMEOUT          I2C transmission timed out.
 * @retval RAV_ERR_WRITE_FAILED     I2C transmission failed.
 */
rav_err_t RAV_LCD_I2C_WriteString(RAV_LCD_I2C_Obj* LCD, const char *String);

/**
 * @brief Clears the LCD display.
 *
 * This function sends the clear display command and waits for the LCD to complete
 * the operation.
 *
 * @param[in] LCD Pointer to the LCD I2C object.
 *
 * @retval RAV_SUCCESS              Display was cleared successfully.
 * @retval RAV_ERR_INVALID_POINTER  LCD object or I2C handle is NULL.
 * @retval RAV_ERR_INVALID_ARGUMENT Invalid transfer mode was detected.
 * @retval RAV_ERR_TIMEOUT          I2C transmission timed out.
 * @retval RAV_ERR_WRITE_FAILED     I2C transmission failed.
 */
rav_err_t RAV_LCD_I2C_Clear(RAV_LCD_I2C_Obj* LCD);

/**
 * @brief Returns the LCD cursor to the home position.
 *
 * This function sends the return home command and waits for the LCD to complete
 * the operation.
 *
 * @param[in] LCD Pointer to the LCD I2C object.
 *
 * @retval RAV_SUCCESS              Cursor returned home successfully.
 * @retval RAV_ERR_INVALID_POINTER  LCD object or I2C handle is NULL.
 * @retval RAV_ERR_INVALID_ARGUMENT Invalid transfer mode was detected.
 * @retval RAV_ERR_TIMEOUT          I2C transmission timed out.
 * @retval RAV_ERR_WRITE_FAILED     I2C transmission failed.
 */
rav_err_t RAV_LCD_I2C_ReturnHome(RAV_LCD_I2C_Obj* LCD);

/**
 * @brief Enables the LCD cursor.
 *
 * @param[in] LCD Pointer to the LCD I2C object.
 *
 * @retval RAV_SUCCESS              Cursor was enabled successfully.
 * @retval RAV_ERR_INVALID_POINTER  LCD object or I2C handle is NULL.
 * @retval RAV_ERR_INVALID_ARGUMENT Invalid transfer mode was detected.
 * @retval RAV_ERR_TIMEOUT          I2C transmission timed out.
 * @retval RAV_ERR_WRITE_FAILED     I2C transmission failed.
 */
rav_err_t RAV_LCD_I2C_CursorOn(RAV_LCD_I2C_Obj* LCD);

/**
 * @brief Disables the LCD cursor.
 *
 * @param[in] LCD Pointer to the LCD I2C object.
 *
 * @retval RAV_SUCCESS              Cursor was disabled successfully.
 * @retval RAV_ERR_INVALID_POINTER  LCD object or I2C handle is NULL.
 * @retval RAV_ERR_INVALID_ARGUMENT Invalid transfer mode was detected.
 * @retval RAV_ERR_TIMEOUT          I2C transmission timed out.
 * @retval RAV_ERR_WRITE_FAILED     I2C transmission failed.
 */
rav_err_t RAV_LCD_I2C_CursorOff(RAV_LCD_I2C_Obj* LCD);

/**
 * @brief Enables cursor blinking.
 *
 * @param[in] LCD Pointer to the LCD I2C object.
 *
 * @retval RAV_SUCCESS              Cursor blinking was enabled successfully.
 * @retval RAV_ERR_INVALID_POINTER  LCD object or I2C handle is NULL.
 * @retval RAV_ERR_INVALID_ARGUMENT Invalid transfer mode was detected.
 * @retval RAV_ERR_TIMEOUT          I2C transmission timed out.
 * @retval RAV_ERR_WRITE_FAILED     I2C transmission failed.
 */
rav_err_t RAV_LCD_I2C_BlinkOn(RAV_LCD_I2C_Obj* LCD);

/**
 * @brief Disables cursor blinking.
 *
 * @param[in] LCD Pointer to the LCD I2C object.
 *
 * @retval RAV_SUCCESS              Cursor blinking was disabled successfully.
 * @retval RAV_ERR_INVALID_POINTER  LCD object or I2C handle is NULL.
 * @retval RAV_ERR_INVALID_ARGUMENT Invalid transfer mode was detected.
 * @retval RAV_ERR_TIMEOUT          I2C transmission timed out.
 * @retval RAV_ERR_WRITE_FAILED     I2C transmission failed.
 */
rav_err_t RAV_LCD_I2C_BlinkOff(RAV_LCD_I2C_Obj* LCD);

RAV_FOOTER

#endif /* _RAV_LCD_I2C_H */
