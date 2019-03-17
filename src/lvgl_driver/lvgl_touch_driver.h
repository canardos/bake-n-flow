#ifndef SRC_LVGL_DRIVER_LVGL_TOUCH_DRIVER_H_
#define SRC_LVGL_DRIVER_LVGL_TOUCH_DRIVER_H_

#include "touch/resistive_touch.h"
#include "lvgl/lvgl.h"

extern "C"
bool lvgl_touchdriver_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);

#endif /* SRC_LVGL_DRIVER_LVGL_TOUCH_DRIVER_H_ */
