/* 
 * File:   RAV_Common_Define_h
 * 
 * Author: Raven Pham
 * 
 * Comments: 
 * - This file header is part of the RAV library developed personally.
 * - This file follows an FSP-like structure for consistency and familiarity.
 * - This file defines common return codes used by RAV APIs.
 *
 * Revision history: Version 0.1
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef _RAV_COMMON_DEFINE_H
#define _RAV_COMMON_DEFINE_H

// Including C Standard Libraries
#include <assert.h>
#include <stdint.h>
#include <stddef.h>

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/** This macro is used to suppress compiler messages about a parameter not being used in a function. The nice thing
 * about using this implementation is that it does not take any extra RAM or ROM. */
#define RAV_PARAMETER_NOT_USED(p)       (void) ((p))

/** Determine if a C++ compiler is being used.
 * If so, ensure that standard C is used to process the API information.  */
#if defined(__cplusplus)
 #define RAV_CPP_HEADER    extern "C" {
 #define RAV_CPP_FOOTER    }
#else
 #define RAV_CPP_HEADER
 #define RAV_CPP_FOOTER
#endif

/** RAV Header and Footer definitions */
#define RAV_HEADER    RAV_CPP_HEADER
#define RAV_FOOTER    RAV_CPP_FOOTER

/* Common macro for RAV header files. There is also a corresponding RAV_FOOTER macro at the end of this file. */
RAV_HEADER

/** Macro to be used when argument to function is ignored since function call is NSC and the parameter is statically
 *  defined on the Secure side. */
#define RAV_SECURE_ARGUMENT             (NULL)   

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
typedef enum e_rav_err   
{
    RAV_SUCCESS = 0,
            
    RAV_ERR_ASSERTION             = 1,                      ///< A critical assertion has failed
    RAV_ERR_INVALID_POINTER       = 2,                      ///< Pointer points to invalid memory location
    RAV_ERR_INVALID_ARGUMENT      = 3,                      ///< Invalid input parameter
    RAV_ERR_INVALID_CHANNEL       = 4,                      ///< Selected channel does not exist
    RAV_ERR_INVALID_MODE          = 5,                      ///< Unsupported or incorrect mode
    RAV_ERR_UNSUPPORTED           = 6,                      ///< Selected mode not supported by this API
    RAV_ERR_NOT_OPEN              = 7,                      ///< Requested channel is not configured or API not open
    RAV_ERR_IN_USE                = 8,                      ///< Channel/peripheral is running/busy
    RAV_ERR_OUT_OF_MEMORY         = 9,                      ///< Allocate more memory in the driver's cfg.h
    RAV_ERR_HW_LOCKED             = 10,                     ///< Hardware is locked
    RAV_ERR_IRQ_BSP_DISABLED      = 11,                     ///< IRQ not enabled in BSP
    RAV_ERR_OVERFLOW              = 12,                     ///< Hardware overflow
    RAV_ERR_UNDERFLOW             = 13,                     ///< Hardware underflow
    RAV_ERR_ALREADY_OPEN          = 14,                     ///< Requested channel is already open in a different configuration
    RAV_ERR_APPROXIMATION         = 15,                     ///< Could not set value to exact result
    RAV_ERR_CLAMPED               = 16,                     ///< Value had to be limited for some reason
    RAV_ERR_INVALID_RATE          = 17,                     ///< Selected rate could not be met
    RAV_ERR_ABORTED               = 18,                     ///< An operation was aborted
    RAV_ERR_NOT_ENABLED           = 19,                     ///< Requested operation is not enabled
    RAV_ERR_TIMEOUT               = 20,                     ///< Timeout error
    RAV_ERR_INVALID_BLOCKS        = 21,                     ///< Invalid number of blocks supplied
    RAV_ERR_INVALID_ADDRESS       = 22,                     ///< Invalid address supplied
    RAV_ERR_INVALID_SIZE          = 23,                     ///< Invalid size/length supplied for operation
    RAV_ERR_WRITE_FAILED          = 24,                     ///< Write operation failed
    RAV_ERR_ERASE_FAILED          = 25,                     ///< Erase operation failed
    RAV_ERR_INVALID_CALL          = 26,                     ///< Invalid function call is made
    RAV_ERR_INVALID_HW_CONDITION  = 27,                     ///< Detected hardware is in invalid condition
    RAV_ERR_INVALID_FACTORY_FLASH = 28,                     ///< Factory flash is not available on this MCU
    RAV_ERR_INVALID_STATE         = 29,                     ///< API or command not valid in the current state
    RAV_ERR_NOT_ERASED            = 30,                     ///< Erase verification failed
    RAV_ERR_SECTOR_RELEASE_FAILED = 31,                     ///< Sector release failed
    RAV_ERR_NOT_INITIALIZED       = 32,                     ///< Required initialization not complete
    RAV_ERR_NOT_FOUND             = 33,                     ///< The requested item could not be found
    RAV_ERR_NO_CALLBACK_MEMORY    = 34,                     ///< Non-secure callback memory not provided for non-secure callback
    RAV_ERR_BUFFER_EMPTY          = 35,                     ///< No data available in buffer
    RAV_ERR_INVALID_DATA          = 36,                     ///< Accuracy of data is not guaranteed
            
} rav_err_t;

RAV_FOOTER

#endif /* _RAV_COMMON_DEFINE_H */