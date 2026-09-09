/**
  ******************************************************************************
  * @file    main.c
  * @brief   Three-phase inverter firmware for STM32F103C8 ("Blue Pill").
  *
  * Drives 6 MOSFETs arranged as three half-bridges through TIM1:
  *   - six complementary gate outputs with hardware dead time,
  *   - emergency shut-down input (PB12 / TIM1_BKIN),
  *   - the three phases are 120 electrical degrees apart, either as
  *     sinusoidal PWM (default) or as 120-degree square waves (six-step).
  *
  * Bare metal: no HAL, no external libraries. See README.md.
  ******************************************************************************
  */

#include "main.h"
#include "tim.h"
#include "gpio.h"
#include "spwm_table.h"

/* ------------------------------------------------------------------------- */
/*  SPWM state                                                               */
/* ------------------------------------------------------------------------- */
#if (INVERTER_MODE == INVERTER_MODE_SPWM)

/* Electrical-angle step per PWM period, in 16.16 fixed-point degrees:
 *   step = f_electrical * 360 * 65536 / f_carrier */
#define SPWM_ANGLE_STEP \
  ((uint32_t)(((uint64_t)SPWM_ELECTRICAL_HZ * 360UL * 65536UL) / SPWM_CARRIER_HZ))

/* Modulation amplitude in the same 0..65535 units as the sine table. */
#define SPWM_AMPLITUDE \
  ((uint32_t)((SPWM_MODULATION_PCT * 65535UL) / 100UL))

/* Electrical angle accumulator (16.16 fixed point degrees). */
static uint32_t spwm_angle = 0;

/**
  * @brief Map one sine-table sample to a CCR duty value.
  * @param sample 0..65535, (sin(angle)+1)/2 * 65535
  * @retval Compare value in 0..(SPWM_ARR_VALUE + 1)
  */
static uint32_t spwm_duty_to_ccr(uint32_t sample)
{
  uint64_t scaled = (uint64_t)sample * SPWM_AMPLITUDE;   /* 0..65535^2     */
  scaled >>= 16;                                         /* 0..65535       */
  return (uint32_t)((scaled * (SPWM_ARR_VALUE + 1UL)) >> 16);
}
#endif /* INVERTER_MODE_SPWM */

#if (INVERTER_MODE == INVERTER_MODE_SIX_STEP)
/* Current commutation sector, 0..5 (each sector spans 60 electrical
 * degrees). Sector timing is generated in hardware by the TIM1 update
 * event; the ISR only selects which phase leg is forced HIGH. */
static uint8_t sixstep_sector = 0;
#endif

/* ------------------------------------------------------------------------- */
/*  Modulation tick (called from TIM1_UP_IRQHandler at every update event)   */
/* ------------------------------------------------------------------------- */
void Inverter_OnUpdate(void)
{
#if (INVERTER_MODE == INVERTER_MODE_SPWM)
  /* Table index for phase A; phases B and C are shifted by -120/+120 deg
   * (positive phase sequence A -> B -> C). */
  uint32_t idx_a = (spwm_angle >> 16) % 360U;
  uint32_t idx_b = (idx_a + 240U) % 360U;   /* lags  A by 120 degrees */
  uint32_t idx_c = (idx_a + 120U) % 360U;   /* leads A by 120 degrees */

  TIM1->CCR1 = spwm_duty_to_ccr(spwm_sine_table[idx_a]);
  TIM1->CCR2 = spwm_duty_to_ccr(spwm_sine_table[idx_b]);
  TIM1->CCR3 = spwm_duty_to_ccr(spwm_sine_table[idx_c]);

  spwm_angle += SPWM_ANGLE_STEP;
#else
  /* Which phase leg is HIGH during this sector (0=A, 1=B, 2=C).
   * A is HIGH for sectors 0,1 (0..120 deg), B for 2,3 (120..240 deg),
   * C for 4,5 (240..360 deg) -> three square waves 120 degrees apart. */
  static const uint8_t high_phase_of_sector[6] = { 0U, 0U, 1U, 1U, 2U, 2U };

  uint8_t hi = high_phase_of_sector[sixstep_sector];

  TIM1->CCMR1 = ((hi == 0U) ? TIM_OCM_FORCED_ACTIVE : TIM_OCM_FORCED_INACTIVE)
                    << TIM_CCMR1_OC1M_Pos
              | ((hi == 1U) ? TIM_OCM_FORCED_ACTIVE : TIM_OCM_FORCED_INACTIVE)
                    << TIM_CCMR1_OC2M_Pos;

  TIM1->CCMR2 = ((hi == 2U) ? TIM_OCM_FORCED_ACTIVE : TIM_OCM_FORCED_INACTIVE)
                    << TIM_CCMR2_OC3M_Pos;

  sixstep_sector++;
  if (sixstep_sector >= 6U)
  {
    sixstep_sector = 0U;
  }
#endif /* INVERTER_MODE */
}

/* ------------------------------------------------------------------------- */
/*  System clock: HSE 8 MHz crystal -> PLL x9 -> SYSCLK 72 MHz               */
/*  AHB = 72 MHz, APB2 = 72 MHz (TIM1 clock), APB1 = 36 MHz                  */
/* ------------------------------------------------------------------------- */
void SystemClock_Config(void)
{
  volatile uint32_t timeout;

  /* 72 MHz operation needs two flash wait states + prefetch buffer */
  FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_2WS;

  /* Start the external 8 MHz crystal and wait for it to stabilise */
  RCC->CR |= RCC_CR_HSEON;
  timeout = 100000UL;
  while (((RCC->CR & RCC_CR_HSERDY) == 0U) && (timeout != 0U))
  {
    timeout--;
  }
  if (timeout == 0U)
  {
    Error_Handler();                /* no / broken crystal on the board     */
  }

  /* PLL = HSE x9 = 72 MHz; bus dividers as listed above.
   * Written while SYSCLK still runs on the internal 8 MHz RC oscillator. */
  RCC->CFGR = RCC_CFGR_HPRE_DIV1
            | RCC_CFGR_PPRE1_DIV2
            | RCC_CFGR_PPRE2_DIV1
            | RCC_CFGR_PLLSRC_HSE
            | RCC_CFGR_PLLMUL_x9;

  RCC->CR |= RCC_CR_PLLON;
  timeout = 100000UL;
  while (((RCC->CR & RCC_CR_PLLRDY) == 0U) && (timeout != 0U))
  {
    timeout--;
  }
  if (timeout == 0U)
  {
    Error_Handler();
  }

  /* Switch SYSCLK to the PLL and wait for the switch to complete */
  RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW_Msk) | RCC_CFGR_SW_PLL;
  timeout = 100000UL;
  while (((RCC->CFGR & RCC_CFGR_SWS_Msk) != RCC_CFGR_SWS_PLL)
         && (timeout != 0U))
  {
    timeout--;
  }
  if (timeout == 0U)
  {
    Error_Handler();
  }

  SystemCoreClock = 72000000UL;
}

/* ------------------------------------------------------------------------- */
int main(void)
{
  SystemClock_Config();             /* 72 MHz from the 8 MHz crystal        */
  MX_GPIO_Init();                   /* six AF outputs + BKIN input          */
  Inverter_Init();                  /* TIM1: PWM + dead time + break        */
  Inverter_Start();                 /* MOE on, update IRQ on, counter on    */

  while (1)
  {
    /* Everything happens inside TIM1 update interrupts. */
  }
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
