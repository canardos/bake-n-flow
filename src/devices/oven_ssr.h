/**
 * Oven SSR (solid state relay) hardware functions.
 *
 * Oven power is controlled via an SSR, which is in turn controlled via the PWM
 * output of the MCU.
 */
#ifndef SRC_OVEN_SSR_H_
#define SRC_OVEN_SSR_H_

#include <cstdint>

/**
 * Initialize the oven SSR hardware.
 *
 * SSR will initially be off.
 */
void initOvenSsr();

/**
 * Set oven SSR output level.
 *
 * @param power 0 -> 100.
 */
void setOvenSsr(uint8_t power);

/**
 * Return the current oven SSR power level (0->100)
 */
uint8_t getOvenSsr();

#endif /* SRC_OVEN_SSR_H_ */
