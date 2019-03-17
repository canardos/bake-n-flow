#ifndef SRC_LVGL_DRIVER_LVGL_TOUCH_DRIVER_CPP_
#define SRC_LVGL_DRIVER_LVGL_TOUCH_DRIVER_CPP_

#include "lvgl/lvgl.h"
#include "touch/resistive_touch.h"
#include "devices/speaker.h"

static bool pressed = false;

extern "C"
bool lvgl_touchdriver_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    auto touchscreen = static_cast<Libp::ResistiveTouch::Screen*>(indev_drv->user_data);
    static Libp::ResistiveTouch::Point touch_pos;

    if (touchscreen->getTouchPosition(&touch_pos)) {
        data->point.x = touch_pos.x;
        data->point.y = touch_pos.y;
        data->state = LV_INDEV_STATE_PR;
        if (!pressed)
            playSound(Sound::click);
        pressed = true;
    }
    else {
        data->state = LV_INDEV_STATE_REL;
        pressed = false;
    }
    return false; //No buffering so no more data read
}

#endif /* SRC_LVGL_DRIVER_LVGL_TOUCH_DRIVER_CPP_ */
