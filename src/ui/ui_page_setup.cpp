#include "lvgl/lvgl.h"
#include "ui/ui_common.h"
#include "app_settings.h"
#include "devices/tft_led.h"
#include "devices/speaker.h"
#include "ui/ui_modal.h"
#include "ui/lvgl_tools.h"
#include "pid/pid_algo.h"

static lv_obj_t* cb_mute_;
static lv_obj_t* bright_slider_;
static lv_obj_t* cb_celsius_;
static lv_obj_t* cb_fahrenheit_;
static lv_obj_t* pid_p_;
static lv_obj_t* pid_i_;
static lv_obj_t* pid_d_;

static Libp::PidAlgo* pid_;

static bool dirty_ = false;
static TempUnit undo_units_;

static void commitChanges()
{
    AppSettings::Data& settings = getSettings();
    settings.brightness = lv_slider_get_value(bright_slider_);
    settings.mute = lv_cb_is_checked(cb_mute_);
    settings.pid_params.kp = intEditFieldGetValue(pid_p_);
    settings.pid_params.ki = intEditFieldGetValue(pid_i_);
    settings.pid_params.kd = intEditFieldGetValue(pid_d_);
    pid_->setPidParams(
            settings.pid_params.kp,
            settings.pid_params.ki,
            settings.pid_params.kd);
    undo_units_ = settings.units;
}

void pageSetupRefresh()
{
    undo_units_ = getSettings().units;
    // Set controls to current settings values
    AppSettings::Data& settings = getSettings();
    lv_cb_set_checked(cb_mute_, settings.mute);
    lv_slider_set_value(bright_slider_, settings.brightness, false);
    lv_cb_set_checked(cb_celsius_, settings.units == TempUnit::celsius);
    lv_cb_set_checked(cb_fahrenheit_, settings.units == TempUnit::fahrenheit);
    labelSetInt(static_cast<lv_obj_t*>(*lv_obj_get_user_data(pid_p_)), settings.pid_params.kp);
    labelSetInt(static_cast<lv_obj_t*>(*lv_obj_get_user_data(pid_i_)), settings.pid_params.ki);
    labelSetInt(static_cast<lv_obj_t*>(*lv_obj_get_user_data(pid_d_)), settings.pid_params.kd);
}

void pageSetupCancel()
{
    if (!dirty_) {
        showPage(Pages::main_menu);
        return;
    }
    createModalMbox(
            "Cancel changes?", ModalMboxType::yes_no, []()
            {
                // Revert units
                getSettings().units = undo_units_;
                // Revert device changes to saved settings
                setTftLed(getSettings().brightness);
                enableSpeaker(!getSettings().mute);
                dirty_ = false;
                showPage(Pages::main_menu);
            },
            nullptr);
}

void pageSetupInit(Libp::PidAlgo* pid)
{
    pid_ = pid;
    lv_obj_t* page = createPage(Pages::setup);

    // Labels

    lv_obj_t* lbl_units      = createDefaultStaticLabel(page, "Units:");
    lv_obj_t* lbl_brightness = createDefaultStaticLabel(page, "Brightness:");
    lv_obj_t* lbl_mute       = createDefaultStaticLabel(page, "Audio:");
    lv_obj_t* lbl_pid        = createDefaultStaticLabel(page, "PID vars.:");
    lv_obj_t* lbl_scn_calib  = createDefaultStaticLabel(page, "To calibrate touch, hold down\nscreen during power on");

    // Unit radio buttons

    cb_celsius_ = createDefaultRadioBtn(page, "celsius", true);
    cb_fahrenheit_ = createDefaultRadioBtn(page, "fahrenheit", false);

    // Checkboxes have a ptr to each other to allow deselecting the other
    // (i.e. emulate radio buttons)
    lv_obj_set_user_data(cb_celsius_, static_cast<void*>(cb_fahrenheit_));
    lv_obj_set_user_data(cb_fahrenheit_, static_cast<void*>(cb_celsius_));

    auto cb_action = [](_lv_obj_t * cb, lv_event_t event)
    {
        if (event != LV_EVENT_CLICKED)
            return;
        lv_cb_set_checked(cb, true);
        lv_cb_set_checked(static_cast<lv_obj_t*>(*lv_obj_get_user_data(cb)), false);

        // All other settings are committed on save but we commit here and undo
        // on back because we need to see live unit changes on status bar
        getSettings().units = (cb == cb_celsius_) ? TempUnit::celsius : TempUnit::fahrenheit;

        // We can't clear dirty flag on revert
        // because other changes may have been made
        if (getSettings().units != undo_units_)
            dirty_ = true;
    };
    lv_obj_set_event_cb(cb_celsius_, cb_action);
    lv_obj_set_event_cb(cb_fahrenheit_, cb_action);

    lv_obj_t* btn_save = createDefaultBtn(page, "Save "  LV_SYMBOL_RIGHT LV_SYMBOL_RIGHT);
    lv_obj_set_event_cb(btn_save,
            [] (struct _lv_obj_t * obj, lv_event_t event)
            {
                if (event == LV_EVENT_CLICKED) {
                    if (dirty_) {
                        commitChanges();
                        if (!AppSettings::get().writeToFlash()) {
                            createModalMbox( "Failed to persist settings.", ModalMboxType::okay, nullptr, nullptr);
                        }
                        dirty_ = false;
                    }
                    showPage(Pages::main_menu);
                }
            });

    // Brightness slider

    bright_slider_ = lv_slider_create(page, NULL);
    lv_slider_set_style(bright_slider_, LV_SLIDER_STYLE_BG, &style_droplist_body);
    lv_slider_set_style(bright_slider_, LV_SLIDER_STYLE_INDIC, &style_main_btn_press);
    lv_slider_set_style(bright_slider_, LV_SLIDER_STYLE_KNOB, &style_main_btn);
    lv_slider_set_range(bright_slider_, 0, led_resolution);

    lv_obj_set_event_cb(bright_slider_,
            [] (struct _lv_obj_t * obj, lv_event_t event)
            {
                if (event == LV_EVENT_VALUE_CHANGED) {
                    setTftLed(lv_slider_get_value(obj));
                    dirty_ = true;
                }
            });

    // PID values

    auto make_dirty = [](int16_t new_val)
    {
        if (new_val != getSettings().pid_params.kp)
            dirty_ = true;
    };

    static constexpr uint16_t max_pid_param_value = 30000;
    pid_p_ = intEditFieldCreate(page, 1, 0, max_pid_param_value, LV_DPI / 2, LV_DPI / 4, make_dirty);
    pid_i_ = intEditFieldCreate(page, 1, 0, max_pid_param_value, LV_DPI / 2, LV_DPI / 4, make_dirty);
    pid_d_ = intEditFieldCreate(page, 1, 0, max_pid_param_value, LV_DPI / 2, LV_DPI / 4, make_dirty);

    // Mute sound checkbox

    cb_mute_ =  createDefaultCb(page, "Mute sound", true);
    lv_obj_set_event_cb(cb_mute_,
            [] (struct _lv_obj_t * obj, lv_event_t event)
            {
                if (event == LV_EVENT_CLICKED) {
                    enableSpeaker(!lv_cb_is_checked(obj));
                    dirty_ = true;
                }
            });

    // Layout

    uint16_t btn_y_pos = lv_obj_get_height(page) - lv_obj_get_height(btn_save) - Padding::outer;
    uint16_t ctrls_x_pos = Padding::outer + lv_obj_get_width(lbl_brightness) + Padding::inner;

    // Buttons at the bottom
    lv_obj_align(btn_save, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -Padding::outer, -Padding::outer);

    // Units at the top
    lv_obj_set_pos(cb_celsius_, ctrls_x_pos, Padding::narrow);// label adds padding
    lv_obj_align(cb_fahrenheit_, cb_celsius_, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);// label padding is enough
    lv_obj_align(lbl_units, cb_celsius_, LV_ALIGN_OUT_LEFT_MID, Padding::narrow, 0);
    lv_obj_set_x(lbl_units, Padding::outer);

    // Mute above buttons
    lv_obj_set_pos(cb_mute_, ctrls_x_pos, btn_y_pos + Padding::narrow - lv_obj_get_height(cb_mute_));
    lv_obj_align(lbl_mute, cb_mute_, LV_ALIGN_OUT_LEFT_MID, -Padding::narrow, 0);
    lv_obj_set_x(lbl_mute, Padding::outer);

    // PID and brightness 1,2 thirds between units and mute
    lv_obj_set_size(bright_slider_, lv_obj_get_width(page) - ctrls_x_pos - Padding::outer, LV_DPI / 5);

    uint16_t y_top = lv_obj_get_y(cb_celsius_) + lv_obj_get_height(cb_celsius_)/2;
    uint16_t y_gap = lv_obj_get_y(cb_mute_) + lv_obj_get_height(cb_mute_)/2 - y_top;
    const uint16_t cb_padding = Padding::inner / 2;

    lv_obj_set_pos(bright_slider_, ctrls_x_pos, y_top + y_gap*1/3 -lv_obj_get_height(bright_slider_)/2);
    lv_obj_set_pos(pid_p_, ctrls_x_pos + cb_padding, y_top + y_gap*2/3 -lv_obj_get_height(pid_p_)/2);
    lv_obj_align(pid_i_, pid_p_, LV_ALIGN_OUT_RIGHT_MID, Padding::inner, 0);
    lv_obj_align(pid_d_, pid_i_, LV_ALIGN_OUT_RIGHT_MID, Padding::inner, 0);

    lv_obj_align(lbl_pid, pid_p_, LV_ALIGN_OUT_LEFT_MID, 0, 0);
    lv_obj_set_x(lbl_pid, Padding::outer);

    lv_obj_align(lbl_brightness, bright_slider_, LV_ALIGN_OUT_LEFT_MID, 0, 0);
    lv_obj_set_x(lbl_brightness, Padding::outer);

    lv_obj_align(lbl_scn_calib, btn_save, LV_ALIGN_OUT_LEFT_BOTTOM, 0, 0);
    lv_obj_set_x(lbl_scn_calib, Padding::outer);

    pageSetupRefresh();
}
