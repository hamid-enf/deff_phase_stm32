/**
  ******************************************************************************
  * @file    tim.h
  * @brief   Three-phase inverter configuration and TIM1 driver API.
  *
  * All user-adjustable parameters are in the block below. Each one can also
  * be overridden from the compiler command line (they use #ifndef guards).
  ******************************************************************************
  */

#ifndef TIM_H
#define TIM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* ========================================================================= */
/*  THREE-PHASE INVERTER CONFIGURATION                                       */
/* ========================================================================= */

/* Modulation mode selection:
 *
 *   INVERTER_MODE_SPWM     : sinusoidal PWM (SPWM). The three phase outputs
 *                            are continuous complementary PWM waveforms whose
 *                            fundamentals are 120 electrical degrees apart.
 *                            This is the standard way to feed a 3-phase load
 *                            (motor, transformer, ...) through 3 half-bridges.
 *
 *   INVERTER_MODE_SIX_STEP : three literal square waves, each phase HIGH for
 *                            120 degrees and LOW for the remaining 240,
 *                            shifted by 120 degrees from each other, with
 *                            complementary low-side outputs and hardware dead
 *                            time at every transition (trapezoidal pattern).
 */
#define INVERTER_MODE_SPWM      0
#define INVERTER_MODE_SIX_STEP  1

#ifndef INVERTER_MODE
#define INVERTER_MODE           INVERTER_MODE_SPWM
#endif

/* Timer input clock in Hz. TIM1 sits on APB2 -> 72 MHz with the clock tree
 * configured in SystemClock_Config() (HSE 8 MHz x9 PLL = SYSCLK 72 MHz). */
#define INVERTER_TIM_CLK_HZ     72000000UL

/* --- SPWM mode parameters ------------------------------------------------ */
/* PWM carrier (switching) frequency. 20 kHz is common for motor drives
 * (above audible range). ARR is derived from this value. */
#ifndef SPWM_CARRIER_HZ
#define SPWM_CARRIER_HZ         20000UL
#endif

/* Electrical (fundamental output) frequency in Hz, e.g. 50 for a 50 Hz
 * three-phase AC output. */
#ifndef SPWM_ELECTRICAL_HZ
#define SPWM_ELECTRICAL_HZ      50UL
#endif

/* Modulation index in percent (0..100). 90% is a safe default; up to 100%
 * is possible in linear SPWM. */
#ifndef SPWM_MODULATION_PCT
#define SPWM_MODULATION_PCT     90UL
#endif

/* --- Six-step mode parameters -------------------------------------------- */
/* Electrical frequency of the 120-degree square waves in Hz.
 * The update ISR runs 6 times per electrical cycle, so keep this modest
 * (recommended <= 5 kHz). */
#ifndef SIXSTEP_ELECTRICAL_HZ
#define SIXSTEP_ELECTRICAL_HZ   1000UL
#endif

/* --- Dead time ----------------------------------------------------------- */
/* Dead time inserted by the TIM1 dead-time generator between the
 * complementary outputs of each half-bridge, in nanoseconds.
 * Must be longer than the turn-off delay of your gate driver + MOSFETs.
 * Typical values: 200..1000 ns. With a 72 MHz timer clock one tick is
 * 13.9 ns; values up to ~1.76 us use the linear DTG encoding. */
#ifndef INVERTER_DEADTIME_NS
#define INVERTER_DEADTIME_NS    500UL
#endif
#define INVERTER_DEADTIME_TICKS ((uint32_t)((INVERTER_DEADTIME_NS * 72UL) / 1000UL))
#if ((INVERTER_DEADTIME_NS * 72UL / 1000UL) > 127UL)
#error "Dead time too large for linear DTG encoding (max 127 ticks = 1.76 us)"
#endif

/* --- Break (emergency shut-down) input ----------------------------------- */
/* PB12 is TIM1_BKIN, active LOW, configured with internal pull-up:
 *   - pin left open or held high ......... inverter runs
 *   - pin pulled low (fault contact, ..... all 6 outputs switched off in
 *     over-current detector, button)        hardware, within one timer cycle
 * Set INVERTER_ENABLE_BREAK to 0 only if you do not want the BKIN pin used. */
#ifndef INVERTER_ENABLE_BREAK
#define INVERTER_ENABLE_BREAK   1
#endif

/* When 1, outputs resume automatically at the next update event after the
 * break condition is cleared. When 0, firmware must call
 * Inverter_RearmAfterBreak() to restart (safer for real motor drives). */
#ifndef INVERTER_AUTO_REARM
#define INVERTER_AUTO_REARM     1
#endif

/* --- Derived values (do not edit) ---------------------------------------- */
/* Frequencies that do not divide the 72 MHz timer clock exactly are
 * approximated with rounding (typical error << 1%). */
#define SPWM_ARR_VALUE (((INVERTER_TIM_CLK_HZ + SPWM_CARRIER_HZ / 2UL) / SPWM_CARRIER_HZ) - 1UL)
#define SIXSTEP_COUNT_RATE (360UL * SIXSTEP_ELECTRICAL_HZ)
#define SIXSTEP_PSC_VALUE (((INVERTER_TIM_CLK_HZ + SIXSTEP_COUNT_RATE / 2UL) / SIXSTEP_COUNT_RATE) - 1UL)

#if (SPWM_ARR_VALUE > 65535UL)
#error "SPWM_CARRIER_HZ is too low for a 72 MHz timer clock"
#endif
#if (SIXSTEP_PSC_VALUE > 65535UL)
#error "SIXSTEP_ELECTRICAL_HZ is too low for a 72 MHz timer clock"
#endif

/* ========================================================================= */
/*  Driver API                                                               */
/* ========================================================================= */

/* Configures TIM1: three complementary PWM channels, dead time, break. */
void Inverter_Init(void);

/* Starts all three complementary output pairs + update interrupt. */
void Inverter_Start(void);

/* Only needed when INVERTER_AUTO_REARM == 0: re-enables the main output
 * (MOE) after a break event has been cleared. */
void Inverter_RearmAfterBreak(void);

/* Modulation tick, called from TIM1_UP_IRQHandler. Implements SPWM duty
 * refresh or six-step commutation depending on INVERTER_MODE. */
void Inverter_OnUpdate(void);

#ifdef __cplusplus
}
#endif

#endif /* TIM_H */
