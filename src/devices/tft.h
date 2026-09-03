#ifndef SRC_TFT_H_
#define SRC_TFT_H_

#include "graphics/lp_idrawing_surface.h"

/**
 * Initialize the TFT display, setup the LVGL driver.
 */
libp::IDrawingSurface<uint16_t>& initTftDisplay();

/**
 * The returned object has static duration.
 *
 * @return
 */
libp::IDrawingSurface<uint16_t>& getTftDisplay();

#endif /* SRC_TFT_H_ */
