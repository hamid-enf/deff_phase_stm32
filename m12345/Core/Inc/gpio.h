/**
  ******************************************************************************
  * @file    gpio.h
  * @brief   GPIO setup for the three-phase inverter project.
  ******************************************************************************
  */

#ifndef GPIO_H
#define GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "tim.h"      /* INVERTER_ENABLE_BREAK */

void MX_GPIO_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* GPIO_H */
