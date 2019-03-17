#include "ui/ui_common.h"
#include "app_settings.h"
#include "devices/speaker.h"
#include "reflow/reflow_profiles.h"
#include "oven/oven_operation.h"
#include "lvgl/lvgl.h"
#include <stdio.h>

static ReflowProfiles& profiles_ = getReflowProfiles();
static OvenOperation* oven_operation_;

static lv_obj_t* profile_dl_;
static lv_obj_t* profile_btns_;
static lv_obj_t* btn_start_;

static void updateProfileListAndBtns()
{
    const uint8_t num_profiles = profiles_.getNumProfiles();

    lv_btnm_set_btn_ctrl(profile_btns_, 0, LV_BTNM_CTRL_INACTIVE, num_profiles == ReflowProfiles::max_profiles_);
    lv_btnm_set_btn_ctrl(profile_btns_, 2, LV_BTNM_CTRL_INACTIVE, num_profiles == 1);
    lv_btnm_set_btn_ctrl(profile_btns_, 2, LV_BTNM_CTRL_CLICK_TRIG, num_profiles != 1);

    uint16_t len = 0;
    for (uint8_t i = 0; i < num_profiles; i++) {
        len += strlen(profiles_.getProfile(i).name) + 1;
    }

    // WARNING: below C99 VLA is not ISO C++ compliant.
    //          Confirm compiler support if not using GCC

    char ddlist_string[len]; // lv_ddlist_set_options allocates mem and copies text
    uint16_t idx = 0;
    for (uint8_t i = 0; i < num_profiles; i++) {
        char* name = profiles_.getProfile(i).name;
        strcpy(&ddlist_string[idx], name);
        idx += strlen(name);
        ddlist_string[idx++] = '\n';
    }
    ddlist_string[--idx] = '\0';
    uint16_t sel_idx = lv_ddlist_get_selected(profile_dl_);
    lv_ddlist_set_options(profile_dl_, ddlist_string);
    lv_ddlist_set_selected(profile_dl_, sel_idx);
}


static void onChangeProfile(uint8_t profile_idx)
{
    profiles_.setActiveProfile(profile_idx);
}


static void finishReflow()
{
    playSound(Sound::completed);
    createModalMbox("Reflow operation complete.", ModalMboxType::okay, []() {
        showPage(Pages::reflow);
    }, nullptr);
}


static void addEditDelClickAction(lv_obj_t * btnm, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED)
        return;
    uint16_t active_btn_id = lv_btnm_get_active_btn(btnm);
    if (lv_btnm_get_btn_ctrl(btnm, active_btn_id, LV_BTNM_CTRL_INACTIVE))
        return;

    switch (active_btn_id) {
    case 0: // new
        {
            uint8_t num_profiles = profiles_.addProfile();
            if (num_profiles > 0) {
                updateProfileListAndBtns();
                lv_ddlist_set_selected(profile_dl_, num_profiles - 1);
                profiles_.setActiveProfile(num_profiles - 1);
                showPage(Pages::reflow_edit_profile);
            }
            break;
        }
    case 1: // edit
        showPage(Pages::reflow_edit_profile);
        break;
    case 2: // delete
        static constexpr uint8_t max_len = sizeof("Delete ?") + ReflowProfiles::max_name_len;
        char title[max_len];
        snprintf(title, max_len, "Delete %s?", profiles_.getActiveProfile().name);
        createModalMbox(title, ModalMboxType::yes_no, []() {
            uint16_t sel_idx = lv_ddlist_get_selected(profile_dl_);
            profiles_.deleteProfile(sel_idx);
            updateProfileListAndBtns();
            sel_idx = lv_ddlist_get_selected(profile_dl_);
            profiles_.setActiveProfile(sel_idx);
        }, nullptr);
        break;
    }
    // TODO: copying button ID in btnm is not necessary since this is const in lv_event_get_data()
}


void pageReflowRefresh()
{
    updateProfileListAndBtns();
}

// TODO: break this function up

void pageReflowInit(OvenOperation* oven_operation)
{
    oven_operation_ = oven_operation;
    lv_obj_t* page = createPage(Pages::reflow);

    // Drop list

    lv_obj_t* text = createDefaultStaticLabel(page, "Select reflow profile");
    lv_obj_set_pos(text, Padding::outer, Padding::outer);
    profile_dl_ = lv_ddlist_create(page, NULL);

    lv_ddlist_set_style(profile_dl_, LV_DDLIST_STYLE_SEL, &style_main_btn);
    lv_ddlist_set_style(profile_dl_, LV_DDLIST_STYLE_BG, &style_droplist_body);
    lv_ddlist_set_draw_arrow(profile_dl_, true);

    lv_obj_align(profile_dl_, text, LV_ALIGN_OUT_BOTTOM_LEFT, 0, Padding::narrow);

    // Start button

    btn_start_ = createDefaultBtn(page, "Start " LV_SYMBOL_RIGHT LV_SYMBOL_RIGHT);
    lv_obj_set_event_cb(btn_start_,
            [] (struct _lv_obj_t * obj, lv_event_t event)
            {
                if (event == LV_EVENT_CLICKED) {
                    static constexpr uint8_t max_len = sizeof("Run reflow profile ''?") + ReflowProfiles::max_name_len;
                    char buf[max_len];
                    snprintf(buf, max_len, "Run reflow profile '%s'?", profiles_.getActiveProfile().name);
                    createModalMbox(buf, ModalMboxType::yes_no, []() {
                        bool started = oven_operation_->startReflow(profiles_.getActiveProfile(), finishReflow);
                        if (started) {
                            showPage(Pages::reflow_run);
                        }
                        else {
                            createModalMbox("Failed to start.\nOven too hot?", ModalMboxType::okay, nullptr, nullptr);
                        }
                    }, nullptr);
                }
            });

    lv_obj_align(btn_start_, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -Padding::outer, -Padding::outer);

    // Profile create/edit buttons

    lv_coord_t dl_width = LV_HOR_RES_MAX - lv_obj_get_width(btn_start_) - Padding::inner - Padding::outer - Padding::outer;


    static const char* profile_btn_lbls[] = { "New", "Edit", "Delete", "" };
    profile_btns_  = createDefaultBtnm(page, profile_btn_lbls);

    // Alignment and sizing

    lv_obj_set_size(profile_btns_, dl_width, lv_obj_get_height(btn_start_) + Padding::outer + Padding::outer);
    lv_ddlist_set_hor_fit(profile_dl_, LV_FIT_NONE);
    lv_obj_set_width(profile_dl_, dl_width);
    lv_obj_align(profile_btns_, profile_dl_, LV_ALIGN_OUT_BOTTOM_LEFT, -Padding::outer, 0);
    lv_obj_set_top(profile_dl_, true);

    lv_obj_set_event_cb(profile_btns_, addEditDelClickAction);

    // Only called for user change action (not called on repopulation) -  still true?
    lv_obj_set_event_cb(profile_dl_,
            [] (struct _lv_obj_t * ddlist, lv_event_t event)
            {
                if (event == LV_EVENT_VALUE_CHANGED) {
                    uint16_t sel_idx = lv_ddlist_get_selected(ddlist);
                    onChangeProfile(sel_idx);
                }
            });
    updateProfileListAndBtns();
}

void cancelReflow()
{
    oven_operation_->stop();
    showPage(Pages::reflow);
}

