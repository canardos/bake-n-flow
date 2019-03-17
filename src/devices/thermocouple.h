/**
 * Thermocouple hardware functions.
 *
 * The thermocouple is connected to a MAX31856, which is read via SPI.
 */
#ifndef SRC_DEVICES_THERMOCOUPLE_H_
#define SRC_DEVICES_THERMOCOUPLE_H_

#include <cstdint>

/**
 * Initialize thermocouple hawdware.
 */
void initThermocouple();

/**
 * Read thermocouple temperature
 *
 * @return temperature in tenths of a degree Celcius.
 */
int16_t readTemp();

#include "drivers/max31856/max_31856.h"

void testMax();


#endif /* SRC_DEVICES_THERMOCOUPLE_H_ */
