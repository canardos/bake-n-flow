#ifndef SRC_TFT_H_
#define SRC_TFT_H_

#include "graphics/idrawing_surface.h"

/**
 * Initialize the TFT display, setup the LVGL driver.
 */
Libp::IDrawingSurface<uint16_t>& initTftDisplay();

/**
 * The returned object has static duration.
 *
 * @return
 */
Libp::IDrawingSurface<uint16_t>& getTftDisplay();

#endif /* SRC_TFT_H_ */
