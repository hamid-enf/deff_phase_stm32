/**
  ******************************************************************************
  * @file    stm32f103_regs.h
  * @brief   Minimal register map for STM32F103 (medium density) - bare metal.
  *
  * Only the peripherals actually used by this project are defined:
  *   RCC, FLASH, GPIO (A/B/C/D), AFIO base, TIM1 and the TIM register set.
  * Core peripherals (NVIC, SCB, SysTick) come from the CMSIS core_cm3.h.
  *
  * Reference: RM0008 "STM32F101xx, STM32F102xx, STM32F103xx reference manual".
  * Every register and bit below is documented in README.md as well.
  ******************************************************************************
  */

#ifndef STM32F103_REGS_H
#define STM32F103_REGS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* STM32F1 implements 4 priority bits (16 levels) - required by core_cm3.h */
#define __NVIC_PRIO_BITS          4
#define __Vendor_SysTickConfig    0
#define __MPU_PRESENT             0

/* ========================================================================= */
/*  Base addresses (RM0008 chapter 3, memory map)                            */
/* ========================================================================= */
#define PERIPH_BASE         0x40000000UL
#define APB1PERIPH_BASE     (PERIPH_BASE)
#define APB2PERIPH_BASE     (PERIPH_BASE + 0x00010000UL)
#define AHBPERIPH_BASE      (PERIPH_BASE + 0x00020000UL)

#define TIM1_BASE           (APB2PERIPH_BASE + 0x2C00UL)
#define AFIO_BASE           (APB2PERIPH_BASE + 0x0000UL)
#define GPIOA_BASE          (APB2PERIPH_BASE + 0x0800UL)
#define GPIOB_BASE          (APB2PERIPH_BASE + 0x0C00UL)
#define GPIOC_BASE          (APB2PERIPH_BASE + 0x1000UL)
#define GPIOD_BASE          (APB2PERIPH_BASE + 0x1400UL)
#define RCC_BASE            (AHBPERIPH_BASE  + 0x1000UL)
#define FLASH_R_BASE        (AHBPERIPH_BASE  + 0x2000UL)

/* ========================================================================= */
/*  RCC - Reset and Clock Control (RM0008 section 7.3)                       */
/* ========================================================================= */
typedef struct
{
  volatile uint32_t CR;         /*!< Clock control register,                0x00 */
  volatile uint32_t CFGR;       /*!< Clock configuration register,        0x04 */
  volatile uint32_t CIR;        /*!< Clock interrupt register,            0x08 */
  volatile uint32_t APB2RSTR;   /*!< APB2 peripheral reset register,      0x0C */
  volatile uint32_t APB1RSTR;   /*!< APB1 peripheral reset register,      0x10 */
  volatile uint32_t AHBENR;     /*!< AHB periph. clock enable register,   0x14 */
  volatile uint32_t APB2ENR;    /*!< APB2 periph. clock enable register,  0x18 */
  volatile uint32_t APB1ENR;    /*!< APB1 periph. clock enable register,  0x1C */
  volatile uint32_t BDCR;       /*!< Backup domain control register,      0x20 */
  volatile uint32_t CSR;        /*!< Control/status register,             0x24 */
} RCC_TypeDef;

#define RCC                     ((RCC_TypeDef *)RCC_BASE)

/* RCC_CR bits */
#define RCC_CR_HSION            (1UL << 0)   /*!< Internal 8 MHz RC oscillator ON      */
#define RCC_CR_HSIRDY           (1UL << 1)   /*!< HSI ready flag                       */
#define RCC_CR_HSEON            (1UL << 16)  /*!< External crystal (8 MHz) ON          */
#define RCC_CR_HSERDY           (1UL << 17)  /*!< HSE ready flag                       */
#define RCC_CR_PLLON            (1UL << 24)  /*!< PLL enable                           */
#define RCC_CR_PLLRDY           (1UL << 25)  /*!< PLL ready flag                       */

/* RCC_CFGR bits */
#define RCC_CFGR_SW_Pos         0            /*!< System clock switch                    */
#define RCC_CFGR_SW_Msk         (3UL << RCC_CFGR_SW_Pos)
#define RCC_CFGR_SW_PLL         (2UL << RCC_CFGR_SW_Pos)   /*!< PLL as SYSCLK       */
#define RCC_CFGR_SWS_Pos        2            /*!< System clock switch status             */
#define RCC_CFGR_SWS_Msk        (3UL << RCC_CFGR_SWS_Pos)
#define RCC_CFGR_SWS_PLL        (2UL << RCC_CFGR_SWS_Pos)  /*!< PLL used as SYSCLK  */
#define RCC_CFGR_HPRE_DIV1      (0UL << 4)   /*!< AHB prescaler  = 1  -> HCLK  72 MHz   */
#define RCC_CFGR_PPRE1_DIV2     (4UL << 8)   /*!< APB1 prescaler = 2  -> PCLK1 36 MHz   */
#define RCC_CFGR_PPRE2_DIV1     (0UL << 11)  /*!< APB2 prescaler = 1  -> PCLK2 72 MHz   */
#define RCC_CFGR_PLLSRC_HSE     (1UL << 16)  /*!< PLL source = HSE (8 MHz crystal)      */
#define RCC_CFGR_PLLMUL_Pos     18           /*!< PLL multiplication factor             */
#define RCC_CFGR_PLLMUL_x9      (7UL << RCC_CFGR_PLLMUL_Pos) /*!< x9 -> 72 MHz      */

/* RCC_APB2ENR bits - peripheral clock enables (APB2 runs at 72 MHz) */
#define RCC_APB2ENR_AFIOEN      (1UL << 0)   /*!< Alternate function IO clock           */
#define RCC_APB2ENR_IOPAEN      (1UL << 2)   /*!< GPIOA clock                           */
#define RCC_APB2ENR_IOPBEN      (1UL << 3)   /*!< GPIOB clock                           */
#define RCC_APB2ENR_IOPCEN      (1UL << 4)   /*!< GPIOC clock                           */
#define RCC_APB2ENR_IOPDEN      (1UL << 5)   /*!< GPIOD clock (OSC pins)                */
#define RCC_APB2ENR_TIM1EN      (1UL << 11)  /*!< TIM1 clock                            */

/* ========================================================================= */
/*  FLASH registers (RM0008 section 3.2)                                     */
/* ========================================================================= */
typedef struct
{
  volatile uint32_t ACR;        /*!< Flash access control register,         0x00 */
} FLASH_TypeDef;

#define FLASH                   ((FLASH_TypeDef *)FLASH_R_BASE)

#define FLASH_ACR_LATENCY_Pos   0            /*!< Wait states for 72 MHz HCLK           */
#define FLASH_ACR_LATENCY_2WS   (2UL << FLASH_ACR_LATENCY_Pos)
#define FLASH_ACR_PRFTBE        (1UL << 4)   /*!< Prefetch buffer enable                */

/* ========================================================================= */
/*  GPIO (RM0008 section 9.2)                                                */
/*                                                                             */
/*  Each pin occupies 4 bits in CRL (pins 0-7) or CRH (pins 8-15):           */
/*    MODE[1:0] : 00 input | 01 out 10 MHz | 10 out 2 MHz | 11 out 50 MHz    */
/*    CNF[1:0]  : output -> 00 GP-PP, 01 GP-OD, 10 AF-PP, 11 AF-OD           */
/*                input  -> 00 analog, 01 floating, 10 with pull up/down     */
/*  For input + pull, the pin's ODR bit selects pull-up(1)/pull-down(0).     */
/* ========================================================================= */
typedef struct
{
  volatile uint32_t CRL;        /*!< Port configuration low  (pins 0-7),    0x00 */
  volatile uint32_t CRH;        /*!< Port configuration high (pins 8-15),   0x04 */
  volatile uint32_t IDR;        /*!< Port input data register,              0x08 */
  volatile uint32_t ODR;        /*!< Port output data register,             0x0C */
  volatile uint32_t BSRR;       /*!< Port bit set/reset register,           0x10 */
  volatile uint32_t BRR;        /*!< Port bit reset register,               0x14 */
  volatile uint32_t LCKR;       /*!< Port configuration lock register,      0x18 */
} GPIO_TypeDef;

#define GPIOA                   ((GPIO_TypeDef *)GPIOA_BASE)
#define GPIOB                   ((GPIO_TypeDef *)GPIOB_BASE)
#define GPIOC                   ((GPIO_TypeDef *)GPIOC_BASE)
#define GPIOD                   ((GPIO_TypeDef *)GPIOD_BASE)

/* 4-bit pin configuration nibbles */
#define GPIO_CFG_INPUT_PU_PD    0x8UL        /*!< input, pull up/down (ODR selects)     */
#define GPIO_CFG_AF_PP_50MHZ    0xBUL        /*!< alt. function push-pull, 50 MHz       */

/* ========================================================================= */
/*  TIM - advanced timer register set (RM0008 section 13.4 / 15.4)           */
/*  Same layout for TIM1..TIM4; only TIM1 is used here.                      */
/* ========================================================================= */
typedef struct
{
  volatile uint32_t CR1;        /*!< Control register 1,                    0x00 */
  volatile uint32_t CR2;        /*!< Control register 2,                    0x04 */
  volatile uint32_t SMCR;       /*!< Slave mode control register,           0x08 */
  volatile uint32_t DIER;       /*!< DMA / interrupt enable register,       0x0C */
  volatile uint32_t SR;         /*!< Status register,                       0x10 */
  volatile uint32_t EGR;        /*!< Event generation register,             0x14 */
  volatile uint32_t CCMR1;      /*!< Capture/compare mode 1 (ch1, ch2),     0x18 */
  volatile uint32_t CCMR2;      /*!< Capture/compare mode 2 (ch3, ch4),     0x1C */
  volatile uint32_t CCER;       /*!< Capture/compare enable register,       0x20 */
  volatile uint32_t CNT;        /*!< Counter,                               0x24 */
  volatile uint32_t PSC;        /*!< Prescaler,                             0x28 */
  volatile uint32_t ARR;        /*!< Auto-reload,                           0x2C */
  volatile uint32_t RCR;        /*!< Repetition counter (TIM1 only),        0x30 */
  volatile uint32_t CCR1;       /*!< Capture/compare 1,                     0x34 */
  volatile uint32_t CCR2;       /*!< Capture/compare 2,                     0x38 */
  volatile uint32_t CCR3;       /*!< Capture/compare 3,                     0x3C */
  volatile uint32_t CCR4;       /*!< Capture/compare 4,                     0x40 */
  volatile uint32_t BDTR;       /*!< Break and dead-time (TIM1 only),       0x44 */
  volatile uint32_t DCR;        /*!< DMA control register,                  0x48 */
  volatile uint32_t DMAR;       /*!< DMA address for full transfer,         0x4C */
} TIM_TypeDef;

#define TIM1                    ((TIM_TypeDef *)TIM1_BASE)

/* TIM_CR1 */
#define TIM_CR1_CEN             (1UL << 0)   /*!< Counter enable                        */
#define TIM_CR1_ARPE            (1UL << 7)   /*!< Auto-reload preload enable            */

/* TIM_DIER / TIM_SR / TIM_EGR */
#define TIM_DIER_UIE            (1UL << 0)   /*!< Update interrupt enable               */
#define TIM_SR_UIF              (1UL << 0)   /*!< Update interrupt flag                 */
#define TIM_EGR_UG              (1UL << 0)   /*!< Re-initialise counter, generate update*/

/* TIM_CCMR1 (identical layout for OC1/OC2; OC2 shifted by 8 bits) */
#define TIM_CCMR1_OC1PE         (1UL << 3)   /*!< Output compare 1 preload enable       */
#define TIM_CCMR1_OC1M_Pos      4            /*!< Output compare 1 mode                 */
#define TIM_CCMR1_OC1M_Msk      (7UL << TIM_CCMR1_OC1M_Pos)
#define TIM_CCMR1_OC2PE         (1UL << 11)
#define TIM_CCMR1_OC2M_Pos      12
#define TIM_CCMR1_OC2M_Msk      (7UL << TIM_CCMR1_OC2M_Pos)
/* TIM_CCMR2 */
#define TIM_CCMR2_OC3PE         (1UL << 3)
#define TIM_CCMR2_OC3M_Pos      4
#define TIM_CCMR2_OC3M_Msk      (7UL << TIM_CCMR2_OC3M_Pos)

/* Output compare modes (written into OCxM) */
#define TIM_OCM_PWM1            (6UL)        /*!< active  while CNT <  CCR              */
#define TIM_OCM_FORCED_ACTIVE   (4UL)        /*!< force output high                     */
#define TIM_OCM_FORCED_INACTIVE (5UL)        /*!< force output low                      */

/* TIM_CCER - one 4-bit group per channel */
#define TIM_CCER_CC1E           (1UL << 0)   /*!< CH1 output enable                     */
#define TIM_CCER_CC1P           (1UL << 1)   /*!< CH1 polarity (0 = active high)        */
#define TIM_CCER_CC1NE          (1UL << 2)   /*!< CH1 complementary output enable       */
#define TIM_CCER_CC2E           (1UL << 4)
#define TIM_CCER_CC2P           (1UL << 5)
#define TIM_CCER_CC2NE          (1UL << 6)
#define TIM_CCER_CC3E           (1UL << 8)
#define TIM_CCER_CC3P           (1UL << 9)
#define TIM_CCER_CC3NE          (1UL << 10)

/* TIM_BDTR - break and dead-time register (advanced timers only) */
#define TIM_BDTR_DTG_Pos        0            /*!< Dead-time generator value             */
#define TIM_BDTR_DTG_Msk        (0xFFUL << 0)
#define TIM_BDTR_OSSI           (1UL << 10)  /*!< Off-state selection for Idle mode     */
#define TIM_BDTR_OSSR           (1UL << 11)  /*!< Off-state selection for Run mode      */
#define TIM_BDTR_BKE            (1UL << 12)  /*!< Break enable (BKIN pin)               */
#define TIM_BDTR_BKP            (1UL << 13)  /*!< Break polarity (0 = active low)       */
#define TIM_BDTR_AOE            (1UL << 14)  /*!< Automatic output enable (re-arm MOE)  */
#define TIM_BDTR_MOE            (1UL << 15)  /*!< Main output enable                    */

/* ========================================================================= */
/*  IRQ numbers (RM0008 section 9.2, medium-density interrupt table)         */
/*  Negative values are Cortex-M3 core exceptions. This enum is required    */
/*  by the CMSIS core headers (core_cm3.h).                                  */
/* ========================================================================= */
typedef enum
{
  /* Cortex-M3 processor exceptions */
  NonMaskableInt_IRQn   = -14,   /*!< 2 Non Maskable Interrupt              */
  HardFault_IRQn        = -13,   /*!< 3 Hard Fault Interrupt                */
  MemoryManagement_IRQn = -12,   /*!< 4 Memory Management Interrupt         */
  BusFault_IRQn         = -11,   /*!< 5 Bus Fault Interrupt                 */
  UsageFault_IRQn       = -10,   /*!< 6 Usage Fault Interrupt               */
  SVCall_IRQn           = -5,    /*!< 11 SV Call Interrupt                  */
  DebugMonitor_IRQn     = -4,    /*!< 12 Debug Monitor Interrupt            */
  PendSV_IRQn           = -2,    /*!< 14 Pend SV Interrupt                  */
  SysTick_IRQn          = -1,    /*!< 15 System Tick Interrupt              */

  /* STM32F103 medium-density device interrupts */
  WWDG_IRQn             = 0,
  PVD_IRQn              = 1,
  TAMPER_IRQn           = 2,
  RTC_IRQn              = 3,
  FLASH_IRQn            = 4,
  RCC_IRQn              = 5,
  EXTI0_IRQn            = 6,
  EXTI1_IRQn            = 7,
  EXTI2_IRQn            = 8,
  EXTI3_IRQn            = 9,
  EXTI4_IRQn            = 10,
  DMA1_Channel1_IRQn    = 11,
  DMA1_Channel2_IRQn    = 12,
  DMA1_Channel3_IRQn    = 13,
  DMA1_Channel4_IRQn    = 14,
  DMA1_Channel5_IRQn    = 15,
  DMA1_Channel6_IRQn    = 16,
  DMA1_Channel7_IRQn    = 17,
  ADC1_2_IRQn           = 18,
  USB_HP_CAN1_TX_IRQn   = 19,
  USB_LP_CAN1_RX0_IRQn  = 20,
  CAN1_RX1_IRQn         = 21,
  CAN1_SCE_IRQn         = 22,
  EXTI9_5_IRQn          = 23,
  TIM1_BRK_IRQn         = 24,
  TIM1_UP_IRQn          = 25,    /*!< TIM1 Update - the inverter tick       */
  TIM1_TRG_COM_IRQn     = 26,
  TIM1_CC_IRQn          = 27,
  TIM2_IRQn             = 28,
  TIM3_IRQn             = 29,
  TIM4_IRQn             = 30,
  I2C1_EV_IRQn          = 31,
  I2C1_ER_IRQn          = 32,
  I2C2_EV_IRQn          = 33,
  I2C2_ER_IRQn          = 34,
  SPI1_IRQn             = 35,
  SPI2_IRQn             = 36,
  USART1_IRQn           = 37,
  USART2_IRQn           = 38,
  USART3_IRQn           = 39,
  EXTI15_10_IRQn        = 40,
  RTC_Alarm_IRQn        = 41,
  USBWakeUp_IRQn        = 42
} IRQn_Type;

/* System core clock, set by SystemClock_Config() in main.c */
extern uint32_t SystemCoreClock;

#ifdef __cplusplus
}
#endif

#endif /* STM32F103_REGS_H */
