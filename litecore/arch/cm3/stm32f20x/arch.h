#ifndef __ARCH_ARCH_H__
#define __ARCH_ARCH_H__

#ifdef __cplusplus
extern "C" {
#endif


//Header Files
#if ARCH_TYPE == ARCH_T_STM32F20X
#define STM32F2XX
#define USE_USB_OTG_FS
#endif

#define USE_STDPERIPH_DRIVER

#include "stm32f2xx.h"

#include <arch/cm3/stm32f20x/adc.h>
#include <arch/cm3/stm32f20x/bkp.h>
#include <arch/cm3/stm32f20x/emac.h>
#include <arch/cm3/stm32f20x/fsmc.h>
#include <arch/cm3/stm32f20x/flash.h>
#include <arch/cm3/stm32f20x/gpio.h>
#include <arch/cm3/stm32f20x/i2c.h>
#include <arch/cm3/stm32f20x/iwdg.h>
#include <arch/cm3/stm32f20x/nand.h>
#include <arch/cm3/stm32f20x/rtc.h>
#include <arch/cm3/stm32f20x/spi.h>
#include <arch/cm3/stm32f20x/timer.h>
#if OS_TYPE
#include <arch/cm3/stm32f20x/uart.h>
#endif

#include <arch/cm3/stm32f20x/it.h>



#if MCU_FREQUENCY == MCU_SPEED_LOW
#define MCU_CLOCK			25000000
#elif MCU_FREQUENCY == MCU_SPEED_HALF
#define MCU_CLOCK			60000000
#else
#define MCU_CLOCK			120000000
#endif







//External Functions
void arch_Init(void);
void arch_IdleEntry(void);
void arch_Reset(void);



#ifdef __cplusplus
}
#endif


#endif

