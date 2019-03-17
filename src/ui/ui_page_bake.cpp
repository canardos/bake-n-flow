#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cinttypes>
#include "ui/ui_common.h"
#include "oven/oven_operation.h"
#include "error_handler.h"
#include "app_settings.h"
#include "devices/speaker.h"

static constexpr uint16_t default_time = 120;
static constexpr uint16_t default_temp_c = 500;
static constexpr uint16_t default_temp_f = 1200;

static OvenOperation* oven_operation_;
static AppSettings& app_settings_ = AppSettings::get();
static AppSettings::Data& settings_ = app_settings_.settings();

static lv_obj_t* temp_input_ctrl_;
static lv_obj_t* time_input_ctrl_;

static TempUnit current_temp_units_;
static char temp_label_text_[] = { "Temperature (°C)" };

static void updateTempLabelText(TempUnit units) {
    static constexpr uint8_t symbol_idx = strlen("Temperature (°C") - 1;
    temp_label_text_[symbol_idx] = units == TempUnit::celsius ? 'C' : 'F';
    // TODO: invalidate label
}

inline
static uint16_t getTempInput()
{
    return intEditFieldGetValue(temp_input_ctrl_);
}

inline
static uint16_t getTimeInput()
{
    return intEditFieldGetValue(time_input_ctrl_);
}

static void finishBake()
{
    createModalMbox("Bake operation complete.", ModalMboxType::okay, []()
    {
        showPage(Pages::bake);
    },
    nullptr);
    // After modal create to avoid being cutoff by alert sound
    playSound(Sound::completed);
}

void pageBakeRefresh()
{
    //assert_param(temp_input_ctrl_ != nullptr);

    TempUnit new_temp_units = settings_.units;
    if (new_temp_units == current_temp_units_)
        // Nothing to do
        return;

    updateTempLabelText(new_temp_units);
    uint16_t new_temp = settings_.unitsToCurrentUnits(getTempInput() * 10, current_temp_units_) / 10;
    intEditFieldSetValue(temp_input_ctrl_, new_temp);
    current_temp_units_ = new_temp_units;
}

static constexpr uint16_t min_bake_duration_mins = 1;

void pageBakeInit(OvenOperation* oven_operation)
{
    oven_operation_ = oven_operation;
    lv_obj_t* page = createPage(Pages::bake);

    // Create labels and inputs

    lv_obj_t* time_label = createDefaultStaticLabel(page, "Time (minutes)");
    lv_obj_t* temp_label = createDefaultStaticLabel(page, temp_label_text_);

    const uint8_t input_height = LV_DPI / 2;
    current_temp_units_ = settings_.units;
    updateTempLabelText(current_temp_units_);
    time_input_ctrl_ /*lv_obj_t* time_input */= intEditFieldCreate( page,
            default_time, min_bake_duration_mins, OvenOperation::max_bake_duration_s / 60,
            LV_DPI / 2, input_height, nullptr);

    temp_input_ctrl_/*    lv_obj_t* temp_input */= intEditFieldCreate( page,
            (current_temp_units_ == TempUnit::celsius ? default_temp_c : default_temp_f) / 10,
            OvenOperation::min_bake_temp / 10, OvenOperation::max_bake_temp / 10,
            LV_DPI / 2, input_height, nullptr);

    // Align

    lv_obj_set_pos(time_input_ctrl_, Padding::outer + lv_obj_get_width(temp_label) + Padding::narrow, Padding::outer);
    lv_obj_set_pos(temp_input_ctrl_, lv_obj_get_x(time_input_ctrl_), lv_obj_get_y(time_input_ctrl_) + input_height + Padding::narrow);
    lv_obj_align(temp_label, temp_input_ctrl_, LV_ALIGN_OUT_LEFT_MID, -Padding::narrow, 0);
    lv_obj_set_pos(time_label, lv_obj_get_x(temp_label), lv_obj_get_y(temp_label) - input_height - Padding::narrow);

    // Start button

    lv_obj_t* btn_start = createDefaultBtn(page, "Start " LV_SYMBOL_RIGHT LV_SYMBOL_RIGHT);
    lv_obj_align(btn_start, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -Padding::outer, -Padding::outer);

    lv_obj_set_event_cb(btn_start, [] (struct _lv_obj_t * obj, lv_event_t event)
    {
        if (event == LV_EVENT_CLICKED) {
            static constexpr uint8_t max_len = sizeof("bake for 999 minutes at 999°C?");
            char buf[max_len];
            snprintf(buf, max_len, "Bake for %d minutes at %d°%c?", getTimeInput(), getTempInput(),
                    settings_.units == TempUnit::celsius ? 'C' : 'F');
            createModalMbox(buf, ModalMboxType::yes_no, []()
            {
                bool started = oven_operation_->startBake(
                        getTimeInput() * 60,
                        settings_.convertUnits(getTempInput() * 10, settings_.units, TempUnit::celsius), finishBake );
                if (started) {
                    pageBakerunSetBakeParams(getTimeInput(), getTempInput());
                    showPage(Pages::bake_run);
                }
                else {
                    createModalMbox("Failed to start.\nOven too hot?", ModalMboxType::okay, nullptr, nullptr);
                }
            },
            nullptr);
        }
    });
}

void cancelBake()
{
    oven_operation_->stop();
    showPage(Pages::bake);
}
