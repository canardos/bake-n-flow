#include <cstdio>

#include "lvgl/lvgl.h"
#include "ui/ui_common.h"
#include "app_settings.h"
#include "string_util.h"

/// The reflow edit page container
static lv_obj_t* page_ = nullptr;
static lv_obj_t* profile_name_label_ = NULL;

namespace FieldId {
    constexpr uint8_t preheat_time = 0;
    constexpr uint8_t preheat_temp = 1;
    constexpr uint8_t soak_time = 2;
    constexpr uint8_t soak_temp = 3;
    constexpr uint8_t reflow_ramp_time = 4;
    constexpr uint8_t reflow_ramp_temp = 5;
    constexpr uint8_t reflow_dwell_time = 6;
    constexpr uint8_t cool_time = 7;
}
static lv_obj_t* edit_ctrls_[8]; // TODO: constant

/// Returned string has static duration
static char* getTempHeadingText(TempUnit units)
{
    static constexpr uint8_t max_len = sizeof("Final Temp.(0.1°C)");
    static char temp_label_text[max_len];
    snprintf(temp_label_text, max_len, "Final Temp.(0.1°%c)", units == TempUnit::celsius ? 'C' : 'F');
    return temp_label_text;
}


/// Edit boxes <--> reflow profile.
enum class DataDir : uint8_t { load, save };
static void readWriteFields(ReflowProfiles::Profile& prof, DataDir direction)
{
    if (page_ == nullptr)
        getErrHndlr().halt(Libp::ErrCode::illegal_state);

    if (direction == DataDir::load) {
        intEditFieldSetValue(edit_ctrls_[FieldId::preheat_time], prof.preheat.duration);
        intEditFieldSetValue(edit_ctrls_[FieldId::reflow_ramp_time], prof.reflow_ramp.duration);
        intEditFieldSetValue(edit_ctrls_[FieldId::soak_time], prof.soak.duration);
        intEditFieldSetValue(edit_ctrls_[FieldId::reflow_dwell_time], prof.reflow_dwell_duration);
        intEditFieldSetValue(edit_ctrls_[FieldId::cool_time], prof.cool_duration);

        uint16_t converted_temp;
        converted_temp = getSettings().unitsToCurrentUnits(prof.preheat.final_temp, prof.units);
        intEditFieldSetValue(edit_ctrls_[FieldId::preheat_temp], converted_temp);

        converted_temp = getSettings().unitsToCurrentUnits(prof.soak.final_temp, prof.units);
        intEditFieldSetValue(edit_ctrls_[FieldId::soak_temp], converted_temp);

        converted_temp = getSettings().unitsToCurrentUnits(prof.reflow_ramp.final_temp, prof.units);
        intEditFieldSetValue(edit_ctrls_[FieldId::reflow_ramp_temp], converted_temp);

        lv_label_set_static_text(profile_name_label_, prof.name);
    }
    else {
        prof.preheat.duration       = intEditFieldGetValue(edit_ctrls_[FieldId::preheat_time]);
        prof.preheat.duration       = intEditFieldGetValue(edit_ctrls_[FieldId::preheat_time]);
        prof.reflow_ramp.duration   = intEditFieldGetValue(edit_ctrls_[FieldId::reflow_ramp_time]);
        prof.soak.duration          = intEditFieldGetValue(edit_ctrls_[FieldId::soak_time]);
        prof.reflow_dwell_duration  = intEditFieldGetValue(edit_ctrls_[FieldId::reflow_dwell_time]);
        prof.cool_duration          = intEditFieldGetValue(edit_ctrls_[FieldId::cool_time]);
        prof.preheat.final_temp     = intEditFieldGetValue(edit_ctrls_[FieldId::preheat_temp]);
        prof.soak.final_temp        = intEditFieldGetValue(edit_ctrls_[FieldId::soak_temp]);
        prof.reflow_ramp.final_temp = intEditFieldGetValue(edit_ctrls_[FieldId::reflow_ramp_temp]);
        prof.units = getSettings().units;
        Libp::strcpy_safe(prof.name, lv_label_get_text(profile_name_label_), ReflowProfiles::max_name_len + 1);
    }
}

/// Refresh data using currently selected profile
void pageRefloweditRefresh()
{
    readWriteFields(getReflowProfiles().getActiveProfile(), DataDir::load);
    getTempHeadingText(getSettings().units);
}

static void pageRefloweditSave()
{
    getModalInputOneLine(KbInputMask::string, ReflowProfiles::max_name_len, profile_name_label_,
            [](lv_obj_t* label, const char* edited_text) -> bool
            {
                if (strlen(edited_text) == 0) {
                    createModalMbox("Profile name may not be empty.", ModalMboxType::okay, nullptr, nullptr);
                    return false;
                }
                // label won't be set updated until this function returns, so update here
                lv_label_set_text(label, edited_text);
                readWriteFields(getReflowProfiles().getActiveProfile(), DataDir::save);
                AppSettings::get().writeToFlash();
                pageReflowRefresh();
                showPage(Pages::reflow);
                return true;
            });
}

void pageRefloweditInit()
{
    if (page_ != NULL)
        getErrHndlr().halt(Libp::ErrCode::illegal_state);

    page_ = createPage(Pages::reflow_edit_profile);

    // TODO: fix hardcoded x positions
    lv_obj_t* dur_label = createDefaultStaticLabel(page_, "Duration (S)");
    lv_obj_set_pos(dur_label, 100, Padding::outer);

    char* txt = getTempHeadingText(getSettings().units);
    lv_obj_t* temp_label = createDefaultStaticLabel(page_, txt);
    lv_obj_set_pos(temp_label, 220, Padding::outer);

    uint16_t avail_height = lv_obj_get_height(page_) - Padding::narrow - lv_obj_get_height(dur_label) - Padding::narrow - Padding::outer;


    uint16_t box_height = (avail_height - 4 * Padding::narrow) / 5;


    // Use a hidden label to store/edit/save the profile name

    profile_name_label_ = lv_label_create(page_, NULL);
    lv_obj_set_hidden(profile_name_label_, true);
    
    // Duration edit controls

    static constexpr uint16_t min_dur_s = 10;
    static constexpr uint16_t max_dur_s = 600;

    lv_obj_t* box_above;
    lv_obj_t* edit_ctrl;

    edit_ctrl = intEditFieldCreate( page_, 0, min_dur_s, max_dur_s, LV_DPI / 2, box_height, nullptr);
    lv_obj_align(edit_ctrl, dur_label, LV_ALIGN_OUT_BOTTOM_MID, 0, Padding::narrow);
    edit_ctrls_[FieldId::preheat_time] = edit_ctrl;
    box_above = edit_ctrl;

    edit_ctrl = intEditFieldCreate( page_, 0, min_dur_s, max_dur_s, LV_DPI / 2, box_height, nullptr);
    lv_obj_align(edit_ctrl, box_above, LV_ALIGN_OUT_BOTTOM_LEFT, 0, Padding::narrow);
    edit_ctrls_[FieldId::soak_time] = edit_ctrl;
    box_above = edit_ctrl;

    edit_ctrl = intEditFieldCreate( page_, 0, min_dur_s, max_dur_s, LV_DPI / 2, box_height, nullptr);
    lv_obj_align(edit_ctrl, box_above, LV_ALIGN_OUT_BOTTOM_LEFT, 0, Padding::narrow);
    edit_ctrls_[FieldId::reflow_ramp_time] = edit_ctrl;
    box_above = edit_ctrl;

    edit_ctrl = intEditFieldCreate( page_, 0, min_dur_s, max_dur_s, LV_DPI / 2, box_height, nullptr);
    lv_obj_align(edit_ctrl, box_above, LV_ALIGN_OUT_BOTTOM_LEFT, 0, Padding::narrow);
    edit_ctrls_[FieldId::reflow_dwell_time] = edit_ctrl;
    box_above = edit_ctrl;

    edit_ctrl = intEditFieldCreate( page_, 0, min_dur_s, max_dur_s, LV_DPI / 2, box_height, nullptr);
    lv_obj_align(edit_ctrl, box_above, LV_ALIGN_OUT_BOTTOM_LEFT, 0, Padding::narrow);
    edit_ctrls_[FieldId::cool_time] = edit_ctrl;
    box_above = edit_ctrl;

    // Temperature edit boxes

    static constexpr uint16_t min_temp = 500;
    static constexpr uint16_t max_temp = 2700;

    edit_ctrl = intEditFieldCreate( page_, 0, min_temp, max_temp, LV_DPI / 2, box_height, nullptr);
    lv_obj_align(edit_ctrl, temp_label, LV_ALIGN_OUT_BOTTOM_MID, 0, Padding::narrow);
    edit_ctrls_[FieldId::preheat_temp] = edit_ctrl;
    box_above = edit_ctrl;

    edit_ctrl = intEditFieldCreate( page_, 0, min_temp, max_temp, LV_DPI / 2, box_height, nullptr);
    lv_obj_align(edit_ctrl, box_above, LV_ALIGN_OUT_BOTTOM_LEFT, 0, Padding::narrow);
    edit_ctrls_[FieldId::soak_temp] = edit_ctrl;
    box_above = edit_ctrl;

    edit_ctrl = intEditFieldCreate( page_, 0, min_temp, max_temp, LV_DPI / 2, box_height, nullptr);
    lv_obj_align(edit_ctrl, box_above, LV_ALIGN_OUT_BOTTOM_LEFT, 0, Padding::narrow);
    edit_ctrls_[FieldId::reflow_ramp_temp] = edit_ctrl;
    box_above = edit_ctrl;

    // Stage labels

    uint16_t text_y_pos = Padding::outer + lv_obj_get_height(dur_label) + Padding::narrow + (box_height - lv_obj_get_height(dur_label)) / 2;
    dur_label = createDefaultStaticLabel(page_, "Preheat", Padding::outer, text_y_pos);
    text_y_pos += box_height + Padding::narrow;

    dur_label = createDefaultStaticLabel(page_, "Soak", Padding::outer, text_y_pos);
    text_y_pos += box_height + Padding::narrow;

    dur_label = createDefaultStaticLabel(page_, "Reflow", Padding::outer, text_y_pos);
    text_y_pos += box_height + Padding::narrow;

    dur_label = createDefaultStaticLabel(page_, "Hold", Padding::outer, text_y_pos);
    text_y_pos += box_height + Padding::narrow;

    dur_label = createDefaultStaticLabel(page_, "Cool", Padding::outer, text_y_pos);

    // Save button

    lv_obj_t* btn_save = createDefaultBtn(page_, "Save " LV_SYMBOL_RIGHT LV_SYMBOL_RIGHT);
    lv_obj_align(btn_save, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -Padding::outer, -Padding::outer);
    lv_obj_set_event_cb(btn_save, [] (struct _lv_obj_t * obj, lv_event_t event)
    {
        if (event == LV_EVENT_CLICKED) {
            pageRefloweditSave();
        }
    });
    // Populate values
    readWriteFields(getReflowProfiles().getActiveProfile(), DataDir::load);
}
