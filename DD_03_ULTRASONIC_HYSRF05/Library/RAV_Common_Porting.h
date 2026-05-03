#ifndef _RAV_COMMON_PORTING_H
#define _RAV_COMMON_PORTING_H

#if defined(STM32F1xx)
 #include "stm32f1xx_hal.h"
#elif defined(STM32F4xx)
 #include "stm32f4xx_hal.h"
#elif defined(STM32F7xx)
 #include "stm32f7xx_hal.h"
#elif defined(STM32H7xx)
 #include "stm32h7xx_hal.h"
#else
 #error "Unsupported STM32 family. Please include the correct HAL header in RAV_Port.h."
#endif

#endif /* _RAV_COMMON_PORTING_H */