/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   Bare-metal GPIO setup for the three-phase inverter.
  *
  *   PA8 / PA9 / PA10   -> TIM1_CH1..3   (high-side gates, AF push-pull)
  *   PB13 / PB14 / PB15 -> TIM1_CH1N..3N (low-side gates,  AF push-pull)
  *   PB12               -> TIM1_BKIN     (input with pull-up, active LOW)
  ******************************************************************************
  */

#include "main.h"
#include "gpio.h"

/**
  * @brief Configure one pin as alternate-function push-pull output, 50 MHz.
  * @param port GPIO port
  * @param pin  pin number 0..15
  */
static void gpio_af_pp(GPIO_TypeDef *port, uint32_t pin)
{
  volatile uint32_t *cr = (pin < 8U) ? &port->CRL : &port->CRH;
  uint32_t pos = (pin % 8U) * 4U;
  *cr = (*cr & ~(0xFUL << pos)) | (GPIO_CFG_AF_PP_50MHZ << pos);
}

/**
  * @brief Configure one pin as digital input with internal pull-up.
  */
#if (INVERTER_ENABLE_BREAK == 1)
static void gpio_input_pullup(GPIO_TypeDef *port, uint32_t pin)
{
  volatile uint32_t *cr = (pin < 8U) ? &port->CRL : &port->CRH;
  uint32_t pos = (pin % 8U) * 4U;
  *cr = (*cr & ~(0xFUL << pos)) | (GPIO_CFG_INPUT_PU_PD << pos);
  port->ODR |= (1UL << pin);       /* ODR bit = 1 selects pull-up          */
}
#endif

void MX_GPIO_Init(void)
{
  /* Port clocks (GPIOD covers the HSE oscillator pins) */
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN
                | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_IOPDEN;

  /* TIM1 gate outputs */
  gpio_af_pp(GPIOA, 8);             /* phase A high  */
  gpio_af_pp(GPIOA, 9);             /* phase B high  */
  gpio_af_pp(GPIOA, 10);            /* phase C high  */
  gpio_af_pp(GPIOB, 13);            /* phase A low   */
  gpio_af_pp(GPIOB, 14);            /* phase B low   */
  gpio_af_pp(GPIOB, 15);            /* phase C low   */

#if (INVERTER_ENABLE_BREAK == 1)
  /* Emergency shut-down input: internal pull-up keeps the inverter running
   * when the pin is open; pull it to GND to kill all outputs in hardware. */
  gpio_input_pullup(GPIOB, 12);
#endif
}
