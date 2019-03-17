#include <stdio.h>
#include "ui/ui_common.h"
#include "ui/ui_shared_content.h"

static lv_obj_t* progress_;
static lv_obj_t* lbl_pct_complete_;
static lv_obj_t* label_elapsed_;
static lv_obj_t* label_remaining_;
static lv_obj_t* title_label_;
// TODO; change to secs and don't calc in refreshUI
static uint16_t bake_time_mins_ = 0;

/// bake_temp in pref units
static void updateTitleText(uint16_t bake_time_s, uint16_t bake_temp)
{
    static constexpr uint8_t max_len = sizeof("baking for 999 minutes at 999°C");
    char buf[max_len];
    snprintf(buf, max_len, "Baking for %d minutes at %d°%c", bake_time_mins_, bake_temp, true ? 'C' : 'F');
    lv_label_set_text(title_label_, buf);
}

void pageBakerunSetBakeParams(uint16_t time_mins, uint16_t temp)
{
    bake_time_mins_ = time_mins;
    lv_bar_set_range(progress_, 0, time_mins * 60);
    updateTitleText(time_mins, temp);
}

void pageBakerunRefreshUi(uint16_t elapsed_time_sec)
{
    // Max time is <1000 mins so uint16_t is safe
    uint16_t total_time_secs = bake_time_mins_ * 60;

    // progress bar
    uint8_t pct_complete = (uint32_t)elapsed_time_sec * 100 / total_time_secs;
    static constexpr uint8_t max_len = sizeof("100%");
    char buf[max_len];
    snprintf(buf, max_len, "%d%%", (int) pct_complete);
    lv_label_set_text(lbl_pct_complete_, buf);
    lv_bar_set_value(progress_, elapsed_time_sec, false);

    updateTimeStrings(elapsed_time_sec, total_time_secs, true);
    // force call of lv_label_refr_text;
    lv_label_set_static_text(label_elapsed_, getElapsedTimeString());
    lv_obj_invalidate(label_remaining_);
}

void pageBakerunInit()
{
    lv_obj_t* page = createPage(Pages::bake_run);

    title_label_ = lv_label_create(page, NULL);

    lv_obj_set_pos(title_label_, Padding::outer, Padding::outer);

    progress_ = lv_bar_create(page, NULL);
    lv_obj_set_size(progress_, lv_obj_get_width(page) - Padding::outer - Padding::outer, LV_DPI / 2);
    lv_bar_set_style(progress_, LV_BAR_STYLE_BG, &style_droplist_body);
    lv_bar_set_style(progress_, LV_BAR_STYLE_INDIC, &style_main_btn);
    lv_obj_align(progress_, title_label_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, Padding::narrow);
    lbl_pct_complete_ = lv_label_create(progress_, NULL);
    lv_obj_align(lbl_pct_complete_, progress_, LV_ALIGN_CENTER, 0, 0);

    label_elapsed_ = createDefaultStaticLabel(page, getElapsedTimeString());
    lv_obj_align(label_elapsed_, progress_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, Padding::narrow);

    label_remaining_ = createDefaultStaticLabel(page, getRemainingTimeString());
    lv_label_set_align(label_remaining_, LV_LABEL_ALIGN_RIGHT);
    lv_obj_align(label_remaining_, progress_, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, Padding::narrow);
}
