/**
  ******************************************************************************
  * @file    tim.c
  * @brief   Bare-metal TIM1 driver: six complementary gate outputs with
  *          hardware dead time and break input for a three-phase inverter.
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
  *  half-bridge can never be commanded ON at the same time: the turning-on
  *  side of every transition is delayed by INVERTER_DEADTIME_TICKS.
  ******************************************************************************
  */

#include "tim.h"

/* ------------------------------------------------------------------------- */
/*  Register map recap (what each field below does):                         */
/*    PSC/ARR  -> timer counts 0..ARR at f = 72 MHz / (PSC + 1)              */
/*    CCMRx    -> per-channel mode: PWM mode 1 or forced output              */
/*    CCRx     -> compare value = duty cycle in PWM mode                     */
/*    CCER     -> output enable + polarity for CHx and CHxN                  */
/*    BDTR     -> dead time, break input, main output enable (MOE)           */
/* ------------------------------------------------------------------------- */

void Inverter_Init(void)
{
  /* 1. TIM1 peripheral clock on (APB2, 72 MHz) */
  RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

  /* 2. Time base */
  TIM1->CR1 = 0;                    /* up-counting, edge-aligned, CEN = 0  */
#if (INVERTER_MODE == INVERTER_MODE_SPWM)
  /* 72 MHz / (3599 + 1) = 20 kHz switching frequency. One update event per
   * PWM period, used as the SPWM modulation tick. */
  TIM1->PSC = 0;
  TIM1->ARR = SPWM_ARR_VALUE;
#else
  /* Six-step mode: the timer acts as an electrical-angle clock.
   * One update event = one 60-degree sector. Counter rate =
   * 360 * f_electrical so one counter period (60 counts) equals 60
   * electrical degrees. */
  TIM1->PSC = SIXSTEP_PSC_VALUE;
  TIM1->ARR = 59;
#endif
  TIM1->RCR = 0;                    /* update event on every overflow      */

  /* 3. Channels 1..3 in PWM mode 1, output-compare preload disabled so that
   *    writes made inside the update ISR apply to the very next period. */
  TIM1->CCMR1 = (TIM_OCM_PWM1 << TIM_CCMR1_OC1M_Pos)
              | (TIM_OCM_PWM1 << TIM_CCMR1_OC2M_Pos);
  TIM1->CCMR2 = (TIM_OCM_PWM1 << TIM_CCMR2_OC3M_Pos);

#if (INVERTER_MODE == INVERTER_MODE_SPWM)
  TIM1->CCR1 = (SPWM_ARR_VALUE + 1UL) / 2UL;
  TIM1->CCR2 = (SPWM_ARR_VALUE + 1UL) / 2UL;
  TIM1->CCR3 = (SPWM_ARR_VALUE + 1UL) / 2UL;
#else
  TIM1->CCR1 = 0;
  TIM1->CCR2 = 0;
  TIM1->CCR3 = 0;
#endif

  /* 4. Output enable, active-high polarity for CHx and CHxN.
   *    (Idle state of all outputs is LOW: safe when MOE is off.) */
  TIM1->CCER = TIM_CCER_CC1E | TIM_CCER_CC1NE
             | TIM_CCER_CC2E | TIM_CCER_CC2NE
             | TIM_CCER_CC3E | TIM_CCER_CC3NE;

  /* 5. Break and dead-time register.
   *    DTG[7:0] = 36 -> 36 x 13.9 ns = 500 ns dead time (see tim.h).
   *    OSSR/OSSI = 1 -> outputs forced to idle (LOW) whenever MOE = 0.
   *    BKE/BKP    -> BKIN pin active LOW.
   *    AOE        -> automatic MOE re-arm after a break (if configured). */
  {
    uint32_t bdtr = (INVERTER_DEADTIME_TICKS << TIM_BDTR_DTG_Pos)
                  | TIM_BDTR_OSSR | TIM_BDTR_OSSI;
#if (INVERTER_ENABLE_BREAK == 1)
    bdtr |= TIM_BDTR_BKE;           /* polarity: BKP = 0 -> active LOW     */
#endif
#if (INVERTER_AUTO_REARM == 1)
    bdtr |= TIM_BDTR_AOE;
#endif
    TIM1->BDTR = bdtr;              /* MOE still 0 -> all outputs idle     */
  }

  /* 6. Generate an update event to load the shadow registers, then clear
   *    the pending update flag before the interrupt is enabled. */
  TIM1->EGR = TIM_EGR_UG;
  TIM1->SR  = 0;

  /* 7. NVIC: moderate priority; nothing else time-critical runs. */
  NVIC_SetPriority(TIM1_UP_IRQn, 1);
}

void Inverter_Start(void)
{
#if (INVERTER_MODE == INVERTER_MODE_SIX_STEP)
  /* Force all outputs LOW before the first commutation ISR fires, so the
   * bridge starts from a safe, well-defined state. */
  TIM1->CCMR1 = (TIM_OCM_FORCED_INACTIVE << TIM_CCMR1_OC1M_Pos)
              | (TIM_OCM_FORCED_INACTIVE << TIM_CCMR1_OC2M_Pos);
  TIM1->CCMR2 = (TIM_OCM_FORCED_INACTIVE << TIM_CCMR2_OC3M_Pos);
#endif

  /* Main output enable: routes the OC signals (through the dead-time
   * generator) to the six pins. With break enabled, MOE can only be set
   * while BKIN is inactive (pin HIGH). */
  TIM1->BDTR |= TIM_BDTR_MOE;

  /* Modulation tick: clear any stale flag, enable the update interrupt and
   * the counter. */
  TIM1->SR   = (uint32_t)~TIM_SR_UIF;
  TIM1->DIER = TIM_DIER_UIE;
  NVIC_EnableIRQ(TIM1_UP_IRQn);
  TIM1->CR1 |= TIM_CR1_CEN;
}

void Inverter_RearmAfterBreak(void)
{
  TIM1->BDTR |= TIM_BDTR_MOE;
}
