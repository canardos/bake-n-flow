/**
 * TFT LED backlight hardware functions.
 *
 * Backlight brightness is controlled via MCU PWM output.
 */
#ifndef SRC_TFT_LED_H_
#define SRC_TFT_LED_H_

#include <cstdint>

inline constexpr uint32_t led_resolution = 100; // brightness steps

/**
 * Initialize TFT backlight brightness control hardware.
 *
 * @param brightness 0->100
 */
void initTftLed(uint8_t brightness);

/**
 * Set TFT backlight brightness.
 *
 * @param brightness 0->100
 */
void setTftLed(uint8_t brightness);

#endif /* SRC_TFT_LED_H_ */
