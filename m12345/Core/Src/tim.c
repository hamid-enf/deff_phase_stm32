/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.c
  * @brief   This file provides code for the configuration
  *          of the TIM instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "tim.h"

/* USER CODE BEGIN 0 */
/*
 * ============================================================================
 *  TIM1 - three-phase inverter timer (6 complementary gate outputs)
 * ============================================================================
 *
 *  Output map (STM32F103C8, default TIM1 mapping):
 *
 *      PA8  -> TIM1_CH1   high-side gate, phase A
 *      PA9  -> TIM1_CH2   high-side gate, phase B
 *      PA10 -> TIM1_CH3   high-side gate, phase C
 *      PB13 -> TIM1_CH1N  low-side  gate, phase A (complement of CH1)
 *      PB14 -> TIM1_CH2N  low-side  gate, phase B (complement of CH2)
 *      PB15 -> TIM1_CH3N  low-side  gate, phase C (complement of CH3)
 *      PB12 -> TIM1_BKIN  emergency shut-down input (active LOW, pull-up)
 *
 *  The dead-time generator (BDTR.DTG) guarantees that the two MOSFETs of a
 *  half-bridge can never be commanded ON at the same time: every transition
 *  of a compare/forced output is delayed by INVERTER_DEADTIME_TICKS on the
 *  turning-on side.
 */

/* Derived constants (SPWM_ARR_VALUE, SIXSTEP_PSC_VALUE) and compile-time
 * checks live in tim.h next to the user configuration. */
/* USER CODE END 0 */

TIM_HandleTypeDef htim1;

/* TIM1 init function */
void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
#if (INVERTER_MODE == INVERTER_MODE_SPWM)
  /* Edge-aligned up-counter, one update event per PWM period.
   * 72 MHz / (3599 + 1) = 20 kHz switching frequency. */
  htim1.Init.Prescaler = 0;
  htim1.Init.Period = SPWM_ARR_VALUE;
#else
  /* Six-step mode: the timer acts as an electrical-angle clock.
   * One update event = one 60-degree sector. Counter rate =
   * 360 * f_electrical so that one full counter period (60 counts)
   * equals 60 electrical degrees. */
  htim1.Init.Prescaler = SIXSTEP_PSC_VALUE;
  htim1.Init.Period = 59;
#endif
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
#if (INVERTER_MODE == INVERTER_MODE_SPWM)
  sConfigOC.Pulse = (SPWM_ARR_VALUE + 1UL) / 2UL;
#else
  sConfigOC.Pulse = 0;
#endif
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* Output-compare preload is intentionally DISABLED for all channels:
   * - SPWM writes new duty values inside the update ISR; they must apply to
   *   the very next period without one extra period of latency.
   * - six-step switches channels to forced-output mode inside the update
   *   ISR; CCMR writes must act immediately. */
  CLEAR_BIT(TIM1->CCMR1, TIM_CCMR1_OC1PE | TIM_CCMR1_OC2PE);
  CLEAR_BIT(TIM1->CCMR2, TIM_CCMR2_OC3PE);

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

  /** TIM1 HAL_TIMEx_ConfigBreakDeadTime
  */
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_ENABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_ENABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = INVERTER_DEADTIME_TICKS;
#if (INVERTER_ENABLE_BREAK == 1)
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_ENABLE;
#else
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
#endif
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_LOW;
#if (INVERTER_AUTO_REARM == 1)
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_ENABLE;
#else
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
#endif
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN TIM1_Init 3 */

  /* USER CODE END TIM1_Init 3 */
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM1)
  {
  /* USER CODE BEGIN TIM1_MspInit 0 */

  /* USER CODE END TIM1_MspInit 0 */
    /* TIM1 clock enable */
    __HAL_RCC_TIM1_CLK_ENABLE();
  /* USER CODE BEGIN TIM1_MspInit 1 */

    /* TIM1 update interrupt (modulation ISR) */
    HAL_NVIC_SetPriority(TIM1_UP_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(TIM1_UP_IRQn);

  /* USER CODE END TIM1_MspInit 1 */
  }
}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* timHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(timHandle->Instance==TIM1)
  {
  /* USER CODE BEGIN TIM1_MspPostInit 0 */

  /* USER CODE END TIM1_MspPostInit 0 */

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**TIM1 GPIO Configuration
    PA8     ------> TIM1_CH1   (phase A high)
    PA9     ------> TIM1_CH2   (phase B high)
    PA10    ------> TIM1_CH3   (phase C high)
    PB13    ------> TIM1_CH1N  (phase A low)
    PB14    ------> TIM1_CH2N  (phase B low)
    PB15    ------> TIM1_CH3N  (phase C low)
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN TIM1_MspPostInit 1 */

  /* USER CODE END TIM1_MspPostInit 1 */
  }

}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM1)
  {
  /* USER CODE BEGIN TIM1_MspDeInit 0 */

  /* USER CODE END TIM1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM1_CLK_DISABLE();
  /* USER CODE BEGIN TIM1_MspDeInit 1 */

    HAL_NVIC_DisableIRQ(TIM1_UP_IRQn);

  /* USER CODE END TIM1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/**
  * @brief Start the three-phase inverter.
  *
  * Enables all three complementary output pairs (CCxE + CCxNE), sets the
  * main output enable (MOE) through the HAL and arms the update interrupt
  * that runs the modulation (SPWM sine refresh or six-step commutation).
  */
void Inverter_Start(void)
{
#if (INVERTER_MODE == INVERTER_MODE_SIX_STEP)
  /* Force all outputs LOW before the first commutation ISR fires, so the
   * bridge starts from a safe, well-defined state (PWM compare values are
   * irrelevant in six-step mode). Forced inactive = OCxM = 0b101. */
  MODIFY_REG(TIM1->CCMR1,
             TIM_CCMR1_OC1M | TIM_CCMR1_OC2M,
             (TIM_OCMODE_FORCED_INACTIVE & TIM_CCMR1_OC1M) |
             ((TIM_OCMODE_FORCED_INACTIVE << 8) & TIM_CCMR1_OC2M));
  MODIFY_REG(TIM1->CCMR2, TIM_CCMR2_OC3M, TIM_OCMODE_FORCED_INACTIVE);
#endif /* INVERTER_MODE_SIX_STEP */

  /* High-side channels */
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);

  /* Complementary low-side channels (also sets MOE on break instances) */
  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);

  /* Modulation tick: TIM1 update interrupt */
  __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_UPDATE);
  __HAL_TIM_ENABLE_IT(&htim1, TIM_IT_UPDATE);
}

/**
  * @brief Re-enable the main output (MOE) after a break event.
  *
  * Only required when INVERTER_AUTO_REARM == 0. Call it after the fault
  * condition on PB12 has been cleared.
  */
void Inverter_RearmAfterBreak(void)
{
  __HAL_TIM_MOE_ENABLE(&htim1);
}

/* USER CODE END 1 */
