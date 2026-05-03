/*
 * File:   RAV_LCD_I2C.c
 *
 * Author: Raven Pham
 *
 * Comments:
 * - This file is part of the RAV LCD I2C driver.
 * - This file provides high-level APIs to control HD44780-compatible LCD modules
 *   through an I2C GPIO expander such as PCF8574.
 * - This implementation follows an FSP-like structure for consistency and readability.
 *
 * Revision history: Version 0.1
 */

/**********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "RAV_LCD_I2C.h"

/**********************************************************************************************************************
 * Private functions
 **********************************************************************************************************************/

/**
 * @brief Provides a blocking delay for LCD timing requirements.
 *
 * @param[in] Time Delay time in milliseconds.
 */
static void RAV_LCD_I2C_Delay(uint16_t Time)
{
	//HAL of STM32
	HAL_Delay(Time);
}

/**
 * @brief Sends one byte to the LCD through the I2C backpack.
 *
 * This function splits the input byte into high and low nibbles, applies control bits
 * such as RS, EN, and backlight, then transmits the resulting 4-byte sequence over I2C.
 *
 * @param[in] LCD  Pointer to the LCD I2C object.
 * @param[in] Data Byte to be sent to the LCD.
 * @param[in] Mode Transfer mode. Use CLCD_COMMAND for commands or CLCD_DATA for display data.
 *
 * @retval RAV_SUCCESS              Data was transmitted successfully.
 * @retval RAV_ERR_INVALID_POINTER  LCD object or I2C handle is NULL.
 * @retval RAV_ERR_INVALID_ARGUMENT Invalid transfer mode.
 * @retval RAV_ERR_TIMEOUT          I2C transmission timed out.
 * @retval RAV_ERR_WRITE_FAILED     I2C transmission failed.
 */
static rav_err_t RAV_LCD_I2C_WriteI2C(RAV_LCD_I2C_Obj* LCD, uint8_t Data, uint8_t Mode)
{
	if ((LCD == NULL) || (LCD->I2C == NULL))
	{
    	return RAV_ERR_INVALID_POINTER;
	}

	HAL_StatusTypeDef status;

	uint8_t Data_H;
	uint8_t Data_L;
	
	uint8_t Data_I2C[4];
	
	Data_H 	= (uint8_t)(Data	  &	0xF0);
	Data_L 	= (uint8_t)((Data<<4) &	0xF0);
	
	if(LCD->BACKLIGHT)
	{
		Data_H |= (uint8_t)LCD_BACKLIGHT; 
		Data_L |= (uint8_t)LCD_BACKLIGHT; 
	}
	if(Mode == CLCD_DATA)
	{
		Data_H |= (uint8_t)LCD_RS;
		Data_L |= (uint8_t)LCD_RS;
	}
	else if(Mode == CLCD_COMMAND)
	{
		Data_H &= (uint8_t)~LCD_RS;
		Data_L &= (uint8_t)~LCD_RS;
	}
	else
	{
		return RAV_ERR_INVALID_ARGUMENT;
	}

	Data_I2C[0] = Data_H|LCD_EN;
	Data_I2C[1] = Data_H;
	Data_I2C[2] = Data_L|LCD_EN;
	Data_I2C[3] = Data_L;

	status = HAL_I2C_Master_Transmit(LCD->I2C, LCD->ADDRESS, (uint8_t *)Data_I2C, sizeof(Data_I2C), 100);

    if (HAL_TIMEOUT == status)
    {
        return RAV_ERR_TIMEOUT;
    }

    if (HAL_OK != status)
    {
        return RAV_ERR_WRITE_FAILED;
    }

	return RAV_SUCCESS;
}

/**********************************************************************************************************************
 * Public functions
 **********************************************************************************************************************/

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
 rav_err_t RAV_LCD_I2C_Init(RAV_LCD_I2C_Obj* LCD, I2C_HandleTypeDef* hi2c_CLCD, uint8_t Address, uint8_t Columns, uint8_t Rows)
{
    if ((LCD == NULL) || (hi2c_CLCD == NULL))
    {
        return RAV_ERR_INVALID_POINTER;
    }

	rav_err_t err = RAV_SUCCESS;

	LCD->I2C 			= hi2c_CLCD;
	LCD->ADDRESS 		= (uint8_t)(Address << 1);
	LCD->COLUMNS 		= Columns;
	LCD->ROWS 			= Rows;
	
	LCD->FUNCTIONSET  	= LCD_FUNCTIONSET	|LCD_4BITMODE	|LCD_2LINE				 |LCD_5x8DOTS ;     //    0x20|0x00|0x08|0x00 = 0x28
	LCD->ENTRYMODE    	= LCD_ENTRYMODESET	|LCD_ENTRYLEFT	|LCD_ENTRYSHIFTDECREMENT			  ;     //    0x04|0x02|0x00      = 0x06
	LCD->DISPLAYCTRL  	= LCD_DISPLAYCONTROL|LCD_DISPLAYON	|LCD_CURSOROFF			 |LCD_BLINKOFF;     //    0x08|0x04|0x00|0x00 = 0x0c
	LCD->CURSORSHIFT  	= LCD_CURSORSHIFT	|LCD_CURSORMOVE	|LCD_MOVERIGHT						  ;     //    0x10|0x00|0x04      = 0x14
	LCD->BACKLIGHT    	= LCD_BACKLIGHT;

	RAV_LCD_I2C_Delay(50U);
	err = RAV_LCD_I2C_WriteI2C(LCD, 0x33, CLCD_COMMAND);
    if (RAV_SUCCESS != err)
    {
        return err;
    }

  	RAV_LCD_I2C_Delay(10U);
	err = RAV_LCD_I2C_WriteI2C(LCD, 0x33, CLCD_COMMAND);
    if (RAV_SUCCESS != err)
    {
        return err;
    }

	RAV_LCD_I2C_Delay(10U);
	err = RAV_LCD_I2C_WriteI2C(LCD, 0x32, CLCD_COMMAND);
    if (RAV_SUCCESS != err)
    {
        return err;
    }

	RAV_LCD_I2C_Delay(10U);
	err = RAV_LCD_I2C_WriteI2C(LCD, 0x20, CLCD_COMMAND);
    if (RAV_SUCCESS != err)
    {
        return err;
    }

	RAV_LCD_I2C_Delay(10U);
	err = RAV_LCD_I2C_WriteI2C(LCD, LCD->FUNCTIONSET, CLCD_COMMAND);    //0x28
    if (RAV_SUCCESS != err)
    {
        return err;
    }

	RAV_LCD_I2C_Delay(10U);
	err = RAV_LCD_I2C_WriteI2C(LCD, LCD_CLEARDISPLAY, CLCD_COMMAND);    //0x01
    if (RAV_SUCCESS != err)
    {
        return err;
    }

	RAV_LCD_I2C_Delay(10U);
	err = RAV_LCD_I2C_WriteI2C(LCD, LCD->ENTRYMODE, CLCD_COMMAND);      //0x06
    if (RAV_SUCCESS != err)
    {
        return err;
    }

	RAV_LCD_I2C_Delay(10U);
	err = RAV_LCD_I2C_WriteI2C(LCD, LCD->DISPLAYCTRL, CLCD_COMMAND);    //0x0c
    if (RAV_SUCCESS != err)
    {
        return err;
    }

	RAV_LCD_I2C_Delay(10U);
	err = RAV_LCD_I2C_WriteI2C(LCD, LCD->CURSORSHIFT,CLCD_COMMAND);     //0x14
    if (RAV_SUCCESS != err)
    {
        return err;
    }

	RAV_LCD_I2C_Delay(10U);
	err = RAV_LCD_I2C_WriteI2C(LCD, LCD_RETURNHOME,CLCD_COMMAND);       //0x02
    if (RAV_SUCCESS != err)
    {
        return err;
    }

	RAV_LCD_I2C_Delay(10U);

	return RAV_SUCCESS;
}

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
rav_err_t RAV_LCD_I2C_SetCursor(RAV_LCD_I2C_Obj* LCD, uint8_t Xpos, uint8_t Ypos)
{
    if ((LCD == NULL) || (LCD->I2C == NULL))
    {
        return RAV_ERR_INVALID_POINTER;
    }

	uint8_t DRAM_ADDRESS = 0x00;

	if(Xpos >= LCD->COLUMNS)
	{
		Xpos = LCD->COLUMNS - 1;
	}

	if(Ypos >= LCD->ROWS)
	{
		Ypos = LCD->ROWS -1;
	}

    switch (Ypos)
    {
        case 0U:
            DRAM_ADDRESS  = 0x00U + Xpos;
            break;

        case 1U:
            DRAM_ADDRESS  = 0x40U + Xpos;
            break;

        case 2U:
            DRAM_ADDRESS  = 0x14U + Xpos;
            break;

        case 3U:
            DRAM_ADDRESS  = 0x54U + Xpos;
            break;

        default:
            return RAV_ERR_INVALID_ARGUMENT;
    }

	return RAV_LCD_I2C_WriteI2C(LCD, LCD_SETDDRAMADDR|DRAM_ADDRESS, CLCD_COMMAND);
}

/**
 * @brief Writes a single character to the LCD.
 *
 * @param[in] LCD       Pointer to the LCD I2C object.
 * @param[in] character Character to be displayed.
 *
 * @retval RAV_SUCCESS              Character was written successfully.
 * @retval RAV_ERR_INVALID_POINTER  LCD object or I2C handle is NULL.
 * @retval RAV_ERR_TIMEOUT          I2C transmission timed out.
 * @retval RAV_ERR_WRITE_FAILED     I2C transmission failed.
 */
rav_err_t RAV_LCD_I2C_WriteChar(RAV_LCD_I2C_Obj* LCD, const char character)
{
    if ((LCD == NULL) || (LCD->I2C == NULL))
    {
        return RAV_ERR_INVALID_POINTER;
    }

	return RAV_LCD_I2C_WriteI2C(LCD, character, CLCD_DATA);
}

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
 * @retval RAV_ERR_TIMEOUT          I2C transmission timed out.
 * @retval RAV_ERR_WRITE_FAILED     I2C transmission failed.
 */
rav_err_t RAV_LCD_I2C_WriteString(RAV_LCD_I2C_Obj* LCD, const char *String)
{
    if ((LCD == NULL) || (LCD->I2C == NULL) || (String == NULL))
    {
        return RAV_ERR_INVALID_POINTER;
    }

    while (*String != '\0')
    {
        rav_err_t err = RAV_LCD_I2C_WriteChar(LCD, *String++);
        if (RAV_SUCCESS != err)
        {
            return err;
        }
    }

    return RAV_SUCCESS;
}

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
 * @retval RAV_ERR_TIMEOUT          I2C transmission timed out.
 * @retval RAV_ERR_WRITE_FAILED     I2C transmission failed.
 */
rav_err_t RAV_LCD_I2C_Clear(RAV_LCD_I2C_Obj* LCD)
{
    if ((LCD == NULL) || (LCD->I2C == NULL))
    {
        return RAV_ERR_INVALID_POINTER;
    }

    rav_err_t err = RAV_LCD_I2C_WriteI2C(LCD, LCD_CLEARDISPLAY, CLCD_COMMAND);
    if (RAV_SUCCESS != err)
    {
        return err;
    }

    RAV_LCD_I2C_Delay(5U);

    return RAV_SUCCESS;
}

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
 * @retval RAV_ERR_TIMEOUT          I2C transmission timed out.
 * @retval RAV_ERR_WRITE_FAILED     I2C transmission failed.
 */
rav_err_t RAV_LCD_I2C_ReturnHome(RAV_LCD_I2C_Obj* LCD)
{
    if ((LCD == NULL) || (LCD->I2C == NULL))
    {
        return RAV_ERR_INVALID_POINTER;
    }

    rav_err_t err = RAV_LCD_I2C_WriteI2C(LCD, LCD_RETURNHOME, CLCD_COMMAND);
    if (RAV_SUCCESS != err)
    {
        return err;
    }

    RAV_LCD_I2C_Delay(5U);

    return RAV_SUCCESS;
}

/**
 * @brief Enables the LCD cursor.
 *
 * @param[in] LCD Pointer to the LCD I2C object.
 *
 * @retval RAV_SUCCESS              Cursor was enabled successfully.
 * @retval RAV_ERR_INVALID_POINTER  LCD object or I2C handle is NULL.
 * @retval RAV_ERR_TIMEOUT          I2C transmission timed out.
 * @retval RAV_ERR_WRITE_FAILED     I2C transmission failed.
 */
rav_err_t RAV_LCD_I2C_CursorOn(RAV_LCD_I2C_Obj* LCD)
{
    if ((LCD == NULL) || (LCD->I2C == NULL))
    {
        return RAV_ERR_INVALID_POINTER;
    }

	LCD->DISPLAYCTRL |= (uint8_t)LCD_CURSORON;
	return RAV_LCD_I2C_WriteI2C(LCD, LCD->DISPLAYCTRL, CLCD_COMMAND);
}

/**
 * @brief Disables the LCD cursor.
 *
 * @param[in] LCD Pointer to the LCD I2C object.
 *
 * @retval RAV_SUCCESS              Cursor was disabled successfully.
 * @retval RAV_ERR_INVALID_POINTER  LCD object or I2C handle is NULL.
 * @retval RAV_ERR_TIMEOUT          I2C transmission timed out.
 * @retval RAV_ERR_WRITE_FAILED     I2C transmission failed.
 */
rav_err_t RAV_LCD_I2C_CursorOff(RAV_LCD_I2C_Obj* LCD)
{
    if ((LCD == NULL) || (LCD->I2C == NULL))
    {
        return RAV_ERR_INVALID_POINTER;
    }

	LCD->DISPLAYCTRL &= (uint8_t)~LCD_CURSORON;
	return RAV_LCD_I2C_WriteI2C(LCD, LCD->DISPLAYCTRL, CLCD_COMMAND);
}

/**
 * @brief Enables cursor blinking.
 *
 * @param[in] LCD Pointer to the LCD I2C object.
 *
 * @retval RAV_SUCCESS              Cursor blinking was enabled successfully.
 * @retval RAV_ERR_INVALID_POINTER  LCD object or I2C handle is NULL.
 * @retval RAV_ERR_TIMEOUT          I2C transmission timed out.
 * @retval RAV_ERR_WRITE_FAILED     I2C transmission failed.
 */
rav_err_t RAV_LCD_I2C_BlinkOn(RAV_LCD_I2C_Obj* LCD)
{
    if ((LCD == NULL) || (LCD->I2C == NULL))
    {
        return RAV_ERR_INVALID_POINTER;
    }

	LCD->DISPLAYCTRL |= (uint8_t)LCD_BLINKON;
	return RAV_LCD_I2C_WriteI2C(LCD, LCD->DISPLAYCTRL, CLCD_COMMAND);
}

/**
 * @brief Disables cursor blinking.
 *
 * @param[in] LCD Pointer to the LCD I2C object.
 *
 * @retval RAV_SUCCESS              Cursor blinking was disabled successfully.
 * @retval RAV_ERR_INVALID_POINTER  LCD object or I2C handle is NULL.
 * @retval RAV_ERR_TIMEOUT          I2C transmission timed out.
 * @retval RAV_ERR_WRITE_FAILED     I2C transmission failed.
 */
rav_err_t RAV_LCD_I2C_BlinkOff(RAV_LCD_I2C_Obj* LCD)
{
    if ((LCD == NULL) || (LCD->I2C == NULL))
    {
        return RAV_ERR_INVALID_POINTER;
    }

	LCD->DISPLAYCTRL &= (uint8_t)~LCD_BLINKON;
	return RAV_LCD_I2C_WriteI2C(LCD, LCD->DISPLAYCTRL, CLCD_COMMAND);
}
