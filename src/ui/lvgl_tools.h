#ifndef UI_LVGL_TOOLS_H_
#define UI_LVGL_TOOLS_H_

#include <cinttypes>
#include "lvgl/lvgl.h"

/**
 * Set label text from an integer value
 *
 * @param label
 * @param value
 */
void labelSetInt(lv_obj_t* label, long value);

#endif /* UI_LVGL_TOOLS_H_ */
