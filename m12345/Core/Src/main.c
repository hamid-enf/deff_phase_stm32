/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "spwm_table.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* SPWM electrical-angle step per PWM period, in 16.16 fixed-point degrees:
 *   step = f_electrical * 360 * 65536 / f_carrier */
#define SPWM_ANGLE_STEP \
  ((uint32_t)(((uint64_t)SPWM_ELECTRICAL_HZ * 360UL * 65536UL) / SPWM_CARRIER_HZ))

/* Modulation amplitude in the same 0..65535 units as the sine table. */
#define SPWM_AMPLITUDE \
  ((uint32_t)((SPWM_MODULATION_PCT * 65535UL) / 100UL))

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

#if (INVERTER_MODE == INVERTER_MODE_SPWM)
/* Electrical angle accumulator (16.16 fixed point degrees). */
static uint32_t spwm_angle = 0;
#else
/* Current commutation sector, 0..5 (each sector spans 60 electrical
 * degrees). Sector timing is generated in hardware by the TIM1 update
 * event; the ISR only selects which phase leg is forced HIGH. */
static uint8_t sixstep_sector = 0;
#endif

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#if (INVERTER_MODE == INVERTER_MODE_SPWM)
/**
  * @brief Map one sine-table sample to a CCR duty value.
  * @param sample 0..65535, (sin(angle)+1)/2 * 65535
  * @retval Compare value in 0..(SPWM_ARR_VALUE + 1)
  */
static uint32_t spwm_duty_to_ccr(uint32_t sample)
{
  uint64_t scaled = (uint64_t)sample * SPWM_AMPLITUDE;   /* 0..65535^2   */
  scaled >>= 16;                                         /* 0..65535     */
  return (uint32_t)((scaled * (SPWM_ARR_VALUE + 1UL)) >> 16);
}
#endif /* INVERTER_MODE_SPWM */

/**
  * @brief TIM1 update interrupt: modulation tick for the inverter.
  *
  * SPWM mode   : called once per PWM period (20 kHz by default). Refreshes
  *               the three duty registers from the sine table so the phase
  *               fundamentals stay exactly 120 degrees apart.
  *
  * Six-step    : called once per 60-degree sector. Forces the active phase
  *               HIGH and the other two LOW; the TIM1 dead-time generator
  *               takes care of safe complementary transitions.
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance != TIM1)
  {
    return;
  }

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

  uint32_t ccmr1 = TIM1->CCMR1 & ~(TIM_CCMR1_OC1M | TIM_CCMR1_OC2M);
  ccmr1 |= (hi == 0U) ? TIM_OCMODE_FORCED_ACTIVE : TIM_OCMODE_FORCED_INACTIVE;
  ccmr1 |= (hi == 1U) ? (uint32_t)TIM_OCMODE_FORCED_ACTIVE << 8U
                      : (uint32_t)TIM_OCMODE_FORCED_INACTIVE << 8U;
  TIM1->CCMR1 = ccmr1;

  uint32_t ccmr2 = TIM1->CCMR2 & ~TIM_CCMR2_OC3M;
  ccmr2 |= (hi == 2U) ? TIM_OCMODE_FORCED_ACTIVE : TIM_OCMODE_FORCED_INACTIVE;
  TIM1->CCMR2 = ccmr2;

  sixstep_sector++;
  if (sixstep_sector >= 6U)
  {
    sixstep_sector = 0U;
  }
#endif /* INVERTER_MODE */
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */

  /* Start the three-phase inverter: 6 complementary gate outputs with
   * hardware dead time, break protection and the modulation ISR. */
  Inverter_Start();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error has occurred.
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
