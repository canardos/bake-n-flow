/**
 * These functions provide an LVGL compatible display driver interface for an
 * @p ITftDriver object.
 */
#ifndef LVGL_TFT_DRIVER_H_
#define LVGL_TFT_DRIVER_H_

#include <cstdint>
#include "lvgl/lvgl.h"

/**
 * LVGL display driver flush callback.
 *
 * Expects pointer to ITftDriver object in disp_drv->user_data
 *
 * @param disp_drv
 * @param area
 * @param color_p
 */
extern "C"
void lvgl_tftdriver_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);

/**
 * LVGL display driver flush callback. DMA version.
 *
 * @plvgl_tftdriver_set_dma_complete must be called once DMA transfer is
 * complete.
 *
 * Expects pointer to ITftDriver object in disp_drv->user_data
 *
 * @param disp_drv
 * @param area
 * @param color_p
 */
extern "C"
void lvgl_tftdriver_flush_dma(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);

/**
 * Call to notify the driver that DMA is complete after each flush operation
 * when using @p lvgl_tftdriver_flush_dma.
 */
extern "C"
void lvgl_tftdriver_set_dma_complete();

#endif /* LVGL_TFT_DRIVER_H_ */
