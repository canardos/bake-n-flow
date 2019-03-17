#include <cstdio>
#include "lvgl/lvgl.h"
#include "ui/ui_common.h"
#include "oven/oven_operation.h"

static lv_obj_t* power_slider_;
static lv_obj_t* cb_off_;
static lv_obj_t* cb_power_;
static lv_obj_t* cb_temp_;
static lv_obj_t* value_cont;

static OvenOperation* oven_operation_;
static lv_obj_t* lbl_level;

enum class ManualState : uint8_t {
    off, fixed_power, fixed_temp
};
static ManualState manual_state_ = ManualState::off;

static void setState(ManualState state)
{
    static constexpr uint8_t label_max_len = sizeof("999°C");
    static char label_text[label_max_len];

    uint16_t val = lv_slider_get_value(power_slider_);
    switch (state) {
    case ManualState::off:
        oven_operation_->stop();
        lv_obj_set_hidden(power_slider_, true);
        lv_obj_set_hidden(value_cont, true);
        manual_state_ = ManualState::off;
        break;

    case ManualState::fixed_power:
        if (manual_state_ != ManualState::fixed_power) {
            lv_slider_set_range(power_slider_, 0, 100);
            lv_slider_set_value(power_slider_, 0, false);
            lv_obj_set_hidden(power_slider_, false);
            lv_obj_set_hidden(value_cont, false);
            manual_state_ = ManualState::fixed_power;
        }
        oven_operation_->startManualPower(val);
        snprintf(label_text, label_max_len, "%d%%", (int)val);
        // TODO: we should just set once and trigger refresh
        lv_label_set_static_text(lbl_level, label_text);
        lv_obj_align(lbl_level, value_cont, LV_ALIGN_CENTER, 0, 0);
        break;

    case ManualState::fixed_temp:
        if (manual_state_ != ManualState::fixed_temp) {
            lv_slider_set_range(power_slider_, 0, 250);
            lv_slider_set_value(power_slider_, 0, false);
            lv_obj_set_hidden(power_slider_, false);
            lv_obj_set_hidden(value_cont, false);
            manual_state_ = ManualState::fixed_temp;
        }
        oven_operation_->startManualTemp(val * 10);
        snprintf(label_text, label_max_len, "%d°C", (int)val);
        // TODO: we should just set once and trigger refresh
        lv_label_set_static_text(lbl_level, label_text);
        lv_obj_align(lbl_level, value_cont, LV_ALIGN_CENTER, 0, 0);
        break;
    }
}

void pageManualOvenOp(OvenOperation* oven_operation)
{
    oven_operation_ = oven_operation;
    lv_obj_t* page = createPage(Pages::advanced);

    cb_off_ = createDefaultRadioBtn(page, "Off", true);
    cb_power_ =  createDefaultRadioBtn(page, "Fixed power", false);
    cb_temp_ = createDefaultRadioBtn(page, "Fixed temp.", false);

    auto cb_action = [](_lv_obj_t * cb, lv_event_t event)
    {
        lv_cb_set_checked(cb_off_, cb == cb_off_);
        lv_cb_set_checked(cb_power_, cb == cb_power_);
        lv_cb_set_checked(cb_temp_, cb == cb_temp_);

        if (event != LV_EVENT_CLICKED)
        return;
        if (cb == cb_off_) {
            setState(ManualState::off);
        }
        else if (cb == cb_power_) {
            setState(ManualState::fixed_power);
        }
        else if (cb == cb_temp_) {
            setState(ManualState::fixed_temp);
        }
    };

    lv_obj_set_event_cb(cb_off_, cb_action);
    lv_obj_set_event_cb(cb_power_, cb_action);
    lv_obj_set_event_cb(cb_temp_, cb_action);

    power_slider_ = lv_slider_create(page, NULL);
    lv_slider_set_style(power_slider_, LV_SLIDER_STYLE_BG, &style_droplist_body);
    lv_slider_set_style(power_slider_, LV_SLIDER_STYLE_INDIC, &style_main_btn_press);
    lv_slider_set_style(power_slider_, LV_SLIDER_STYLE_KNOB, &style_main_btn);

    lv_obj_set_event_cb(power_slider_, [] (struct _lv_obj_t * obj, lv_event_t event)
    {
        if (event == LV_EVENT_VALUE_CHANGED) {
            setState(manual_state_);
        }
    });

    value_cont = lv_cont_create(page, NULL);
    lbl_level = lv_label_create(value_cont, NULL);
    lv_obj_set_style(value_cont, &style_chart);
    lv_obj_set_style(lbl_level, &style_chart);

    lv_obj_set_size(power_slider_, lv_obj_get_width(page) - Padding::outer - Padding::outer, LV_DPI / 5);

    lv_obj_set_pos(cb_off_, Padding::outer, Padding::outer);
    lv_obj_align(cb_power_, cb_off_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_obj_align(cb_temp_, cb_power_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_obj_align(power_slider_, cb_temp_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, Padding::inner);

    uint16_t x_left = lv_obj_get_x(cb_temp_) + lv_obj_get_width(cb_temp_);

    lv_obj_align(value_cont, cb_power_, LV_ALIGN_OUT_RIGHT_MID, Padding::inner*2, 0);
    lv_obj_set_x(value_cont, x_left + (lv_obj_get_width(page) - x_left + Padding::outer)/2 - lv_obj_get_width(lbl_level)/2);
    lv_obj_align(lbl_level, value_cont, LV_ALIGN_CENTER, 0, 0);

    setState(ManualState::off);
}

void cancelManualOvenOp()
{
    oven_operation_->stop();
    showPage(Pages::main_menu);
}
