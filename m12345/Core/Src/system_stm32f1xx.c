/**
  ******************************************************************************
  * @file    system_stm32f1xx.c
  * @brief   SystemInit() for the bare-metal inverter project.
  *
  * Called from the startup file before main(). Sets the vector table base
  * address; the full 72 MHz clock configuration lives in
  * SystemClock_Config() (main.c) in the same style as CubeMX projects.
  ******************************************************************************
  */

#include "main.h"

uint32_t SystemCoreClock = 8000000UL;   /* updated to 72 MHz at run time */

/**
  * @brief  Setup the microcontroller system: vector table relocation.
  */
void SystemInit(void)
{
  /* Relocate the vector table to the start of flash (0x08000000).
   * This is the reset value, kept explicit for clarity and for possible
   * bootloader use. */
  SCB->VTOR = 0x08000000UL;
}
