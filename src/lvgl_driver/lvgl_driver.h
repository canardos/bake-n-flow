#ifndef SRC_LVGL_DRIVER_LVGL_DRIVER_H_
#define SRC_LVGL_DRIVER_LVGL_DRIVER_H_

#include "main.h"
#include "lvgl/lvgl.h"
#include <lvgl_driver/lvgl_tft_driver.h>
#include <graphics/idrawing_surface.h>
#include <lvgl_driver/lvgl_touch_driver.h>

static constexpr uint16_t buffer_size = App::ui_width * 40;
static lv_disp_buf_t disp_buf;
static lv_color_t rows_buf[buffer_size];

/**
 * Setup LVGL touch and display drivers. Provided references will be stored in
 * the driver objects for use during execution.
 *
 * @param touch_screen
 * @param tft_driver
 */
inline
void initLvglHalDrivers(Libp::ResistiveTouch::Screen* touch_screen, Libp::IDrawingSurface<uint16_t>* display)
{
    lv_disp_buf_init(&disp_buf, rows_buf, NULL, buffer_size);

    // display driver

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = App::ui_width;
    disp_drv.ver_res = App::ui_height;
    disp_drv.buffer = &disp_buf;
    disp_drv.flush_cb = lvgl_tftdriver_flush_dma;
    disp_drv.user_data = static_cast<void*>(display);
    lv_disp_drv_register(&disp_drv);

    // touch input driver

    lv_indev_drv_t indev_drv;lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = lvgl_touchdriver_read;
    indev_drv.user_data = static_cast<void*>(touch_screen);
    lv_indev_drv_register(&indev_drv);
}

#endif /* SRC_LVGL_DRIVER_LVGL_DRIVER_H_ */
