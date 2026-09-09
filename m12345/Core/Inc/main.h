/**
  ******************************************************************************
  * @file    main.h
  * @brief   Header for main.c - bare-metal three-phase inverter project.
  *
  * This project intentionally does NOT use the STM32 HAL library: every
  * register access goes through stm32f103_regs.h so the whole firmware is
  * self-contained and can be built on any machine with a plain Keil MDK
  * installation (no CubeMX, no HAL packages, no absolute paths).
  *
  * CMSIS core headers (core_cm3.h, ...) are vendored under Drivers/CMSIS/.
  ******************************************************************************
  */

#ifndef MAIN_H
#define MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* STM32F103xB selects the medium-density device (64K flash, 20K RAM). */
#ifndef STM32F103xB
#define STM32F103xB
#endif

#include <stdint.h>

#include "stm32f103_regs.h"    /* IRQn_Type + minimal peripheral registers */
#include "core_cm3.h"          /* CMSIS core: NVIC, SCB, intrinsics        */

void SystemClock_Config(void);
void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_H */
