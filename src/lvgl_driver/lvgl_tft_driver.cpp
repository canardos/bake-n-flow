#include <graphics/idrawing_surface.h>
#include <cstdint>
#include "lvgl/lvgl.h"

extern "C"
void lvgl_tftdriver_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    Libp::IDrawingSurface<uint16_t>* tft = static_cast<Libp::IDrawingSurface<uint16_t>*>(disp_drv->user_data);
    tft->copyRect(
            static_cast<int16_t>(area->x1), static_cast<int16_t>(area->y1),
            static_cast<int16_t>(area->x2 - area->x1 + 1), static_cast<int16_t>(area->y2 - area->y1 + 1),
            reinterpret_cast<uint16_t*>(color_p));
    lv_disp_flush_ready(disp_drv);
}

static lv_disp_drv_t * driver = nullptr;

extern "C"
void lvgl_tftdriver_flush_dma(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    driver = disp_drv;

    Libp::IDrawingSurface<uint16_t>* tft = static_cast<Libp::IDrawingSurface<uint16_t>*>(disp_drv->user_data);
    tft->copyRect(
            static_cast<int16_t>(area->x1), static_cast<int16_t>(area->y1),
            static_cast<int16_t>(area->x2 - area->x1 + 1), static_cast<int16_t>(area->y2 - area->y1 + 1),
            reinterpret_cast<uint16_t*>(color_p));
}

extern "C"
void lvgl_tftdriver_set_dma_complete()
{
    lv_disp_flush_ready(driver);
}
