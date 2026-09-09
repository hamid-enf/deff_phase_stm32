/**
  ******************************************************************************
  * @file    stm32f1xx_it.c
  * @brief   Interrupt handlers for the three-phase inverter project.
  ******************************************************************************
  */

#include "main.h"
#include "tim.h"

/* Cortex-M3 system exception handlers. Unused ones are left as the default
 * infinite loop so a fault is visible instead of silently re-entering code. */

void NMI_Handler(void)
{
  while (1)
  {
  }
}

void HardFault_Handler(void)
{
  while (1)
  {
  }
}

void MemManage_Handler(void)
{
  while (1)
  {
  }
}

void BusFault_Handler(void)
{
  while (1)
  {
  }
}

void UsageFault_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief TIM1 update interrupt - the inverter modulation tick.
  *
  * Fires once per PWM period (SPWM mode) or once per 60-degree sector
  * (six-step mode). Clears the update flag and dispatches to the
  * modulation engine in main.c.
  */
void TIM1_UP_IRQHandler(void)
{
  TIM1->SR = (uint32_t)~TIM_SR_UIF;
  Inverter_OnUpdate();
}
