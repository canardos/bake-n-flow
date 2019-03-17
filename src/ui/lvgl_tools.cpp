#include "libpekin.h"
#include "string_util.h"
#include <cstdio>
#include <cinttypes>
#include <cstdint>
#include <cstdlib>
#include "lvgl/lvgl.h"

// ARM newlib-nano doesn 't support C99 "%lld" so using long
void labelSetInt(lv_obj_t* label, /*intmax_t*/long value)
{
    static constexpr uint8_t max_len = Libp::maxStrLen(value);
    char txt[max_len];
    snprintf(txt, max_len, "%ld", value);
//    snprintf(txt, max_len, "%" PRIdMAX, value); // do no remove whitespace
    lv_label_set_text(label, txt);
}
