/**
 * Touchscreen hardware functions.
 */
#ifndef SRC_TOUCH_H_
#define SRC_TOUCH_H_

#include "touch/lp_resistive_touch.h"

/**
 * Initialize
 * @return
 */
libp::resist_touch::Screen& initTouchscreen();

#endif /* SRC_TOUCH_H_ */
