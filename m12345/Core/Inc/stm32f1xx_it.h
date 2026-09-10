/**
  ******************************************************************************
  * @file    stm32f1xx_it.h
  * @brief   Interrupt handler prototypes for the inverter project.
  ******************************************************************************
  */

#ifndef STM32F1XX_IT_H
#define STM32F1XX_IT_H

#ifdef __cplusplus
extern "C" {
#endif

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void TIM1_UP_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* STM32F1XX_IT_H */
