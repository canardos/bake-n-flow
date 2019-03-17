#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cinttypes>
#include "string_util.h"
#include "ui/ui_modal.h"
#include "lvgl/lvgl.h"

using namespace Libp;

// These are modal dialogs so we can store these pointers statically
// Nested dialogs will still function correctly
static void (*apply_yes_func_)();
static void (*apply_no_func_)();

// We need two as we display the message box over the keyboard input window
// on validation errors and the alternative of managing the z-order is painful.
// Nested message boxes work fine because the first is destroyed prior to the
// yes/no/okay callbacks executing
static lv_obj_t* screen_cover_ = nullptr;
static lv_obj_t* screen_cover_msgbox_ = nullptr;

static constexpr const char* input_masks_[] {
        // Use "" for no restriction
        // DO NOT REORDER - order must match KbInputMask
        "-0123456789",
        "0123456789",
        ""
};

static void createScreenCovers()
{
    if (screen_cover_ == nullptr) {
        static lv_style_t style_modal_bg;
        // Screen sized rectangle to hide and disable background for modal dialogs
        lv_style_copy(&style_modal_bg, &lv_style_plain_color);
        style_modal_bg.body.main_color = LV_COLOR_BLACK;
        style_modal_bg.body.grad_color = LV_COLOR_BLACK;
        style_modal_bg.body.opa = LV_OPA_50;

        screen_cover_ = lv_cont_create(lv_scr_act(), NULL);
        lv_obj_set_size(screen_cover_, LV_HOR_RES_MAX, LV_VER_RES_MAX);
        lv_cont_set_style(screen_cover_, &style_modal_bg);

        screen_cover_msgbox_ = lv_cont_create(lv_scr_act(), screen_cover_);

        lv_obj_set_hidden(screen_cover_, true);
        lv_obj_set_hidden(screen_cover_msgbox_, true);
    }
}


static lv_obj_t* keyboardCreate(lv_kb_mode_t mode)
{
    lv_obj_t* kb = lv_kb_create(lv_scr_act(), NULL);

    lv_kb_set_style(kb, LV_KB_STYLE_BG, &lv_style_transp_tight); // no btn spacing
    lv_kb_set_style(kb, LV_KB_STYLE_BTN_REL, &style_main_btn);
    lv_kb_set_style(kb, LV_KB_STYLE_BTN_PR, &style_main_btn_press);
    lv_kb_set_cursor_manage(kb, true);
    uint16_t height = (lv_obj_get_height(lv_scr_act()) * 7) / 10; // todo: constants
    lv_obj_set_size(kb, lv_obj_get_width(lv_scr_act()), height);
    lv_obj_set_pos(kb, 0, lv_obj_get_height(lv_scr_act()) - height);
    lv_kb_set_mode(kb, mode);
    return kb;
}

static lv_obj_t* kbTextareaCreate()
{
    lv_obj_t* ta = lv_ta_create(lv_scr_act(), NULL);
    lv_obj_align(ta, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);
    lv_ta_set_cursor_type(ta, LV_CURSOR_BLOCK);
    lv_ta_set_one_line(ta, true);
    lv_ta_set_style(ta, LV_TA_STYLE_BG, &style_chart);
    return ta;
}

void getModalInputOneLine(
        KbInputMask mask, uint8_t max_length, lv_obj_t* label_to_update,
        InputOneLineOkayFunc okay_func )
{
    // Event lambda can't capture so cache in static vars.
    // Not reentrant-safe, max 1 simultaneous instance
    static lv_obj_t* output_text_area;
    static lv_obj_t* label_to_update_cached;
    static InputOneLineOkayFunc okay_func_cached;

    label_to_update_cached = label_to_update;
    okay_func_cached = okay_func;

    createScreenCovers();
    // Move to top
    lv_obj_set_parent(screen_cover_, lv_scr_act());
    lv_obj_set_hidden(screen_cover_, false);

    lv_obj_t* kb = keyboardCreate(mask == KbInputMask::string ? LV_KB_MODE_TEXT : LV_KB_MODE_NUM);
    output_text_area = kbTextareaCreate();

    uint8_t idx = enumBaseT(mask);
    if ( input_masks_[idx][0] != '\0' )
        lv_ta_set_accepted_chars(output_text_area, input_masks_[idx]);

    lv_ta_set_text(output_text_area, lv_label_get_text(label_to_update));
    lv_ta_set_max_length(output_text_area, max_length);
    lv_kb_set_ta(kb, output_text_area);

    // Okay / cancel events

    lv_obj_set_event_cb(kb, [] (struct _lv_obj_t * keyboard, lv_event_t event)
    {
        if (event != LV_EVENT_APPLY && event != LV_EVENT_CANCEL) {
            // call default kb handler for regular key etc. events
            lv_kb_def_event_cb(keyboard, event);
            return;
        }

        if (event == LV_EVENT_APPLY) {
            const char* new_text = lv_ta_get_text(output_text_area);
            // Validate entered text
            if (okay_func_cached != NULL) {
                if (!okay_func_cached(label_to_update_cached, new_text)) {
                    return;
                }
            }
            lv_label_set_text(label_to_update_cached, new_text);
        }
        lv_obj_del(keyboard);
        lv_obj_del(output_text_area);
        lv_obj_set_hidden(screen_cover_, true);
    });
}


static void modalApplyAction(lv_obj_t* mbox, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED)
        return;

    // apply functions might create another modal so close first
    uint16_t active_btn = lv_mbox_get_active_btn(mbox);
    lv_obj_del(mbox);
    lv_obj_set_hidden(screen_cover_msgbox_, true);

    switch (active_btn) {
    case 0:
        if (apply_yes_func_ != nullptr)
            apply_yes_func_();
        break;
    case 1:
        if (apply_no_func_ != nullptr)
            apply_no_func_();
        break;
    }
}

#include "devices/speaker.h"

void createModalMbox(const char* label, ModalMboxType type, void (*yes_func)(), void (*no_func)())
{
    playSound(Sound::error);

    apply_yes_func_ = yes_func;
    apply_no_func_ = no_func;

    createScreenCovers();
    // Move to top
    lv_obj_set_parent(screen_cover_msgbox_, lv_scr_act());
    lv_obj_set_hidden(screen_cover_msgbox_, false);

    lv_obj_t* mbox1 = lv_mbox_create(screen_cover_msgbox_, NULL);
    lv_mbox_set_text(mbox1, label);

    static const char * yes_no_btns[] = {"Yes", "No", ""};
    static const char * okay_btns[] = {"Okay", ""};
    lv_mbox_add_btns(mbox1, type == ModalMboxType::yes_no ? yes_no_btns : okay_btns);
    lv_obj_set_event_cb(mbox1, modalApplyAction);

    lv_obj_set_width(mbox1, 250); // TODO: why 250??
    lv_obj_align(mbox1, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_mbox_set_style(mbox1, LV_MBOX_STYLE_BG, &style_body);
    lv_mbox_set_style(mbox1, LV_MBOX_STYLE_BTN_REL, &style_main_btn);
    lv_mbox_set_style(mbox1, LV_MBOX_STYLE_BTN_PR, &style_main_btn_press);
}


struct IntEditFieldParams {
    int16_t max_value;
    int16_t min_value;
    lv_signal_cb_t lbl_signal_cb;
    IntEditFieldOkayFunc ok_func;
};

static inline IntEditFieldParams* intEditFieldGetParams(lv_obj_t* label)
{
    return static_cast<IntEditFieldParams*>(*lv_obj_get_user_data(label));
}

void intEditFieldSetValue(lv_obj_t* btn, long value)
{
    labelSetInt(lv_obj_get_child(btn, NULL), value);
}

// ARM newlib-nano doesn't support C99 "%lld" so using long
long intEditFieldGetValue(lv_obj_t* btn)
{
    return strtol(
            lv_label_get_text(lv_obj_get_child(btn, NULL)),
            nullptr, 10);
}

lv_obj_t* intEditFieldCreate(lv_obj_t* parent,
        int16_t init_value, int16_t min_value, int16_t max_value,
        uint8_t width, uint8_t height, IntEditFieldOkayFunc ok_func)
{
    lv_obj_t* btn = lv_btn_create(parent, NULL);
    lv_btn_set_style(btn, LV_BTN_STYLE_REL, &style_droplist_body);
    lv_btn_set_style(btn, LV_BTN_STYLE_PR, &style_droplist_body);
    lv_obj_set_size(btn, width, height);
    lv_obj_t* label = lv_label_create(btn, NULL);
    labelSetInt(label, init_value);

    // Use labels user_data to store numeric limits

    void* params_mem = lv_mem_alloc(sizeof(IntEditFieldParams));
    if (!params_mem)
        getErrHndlr().halt(ErrCode::mem_alloc_fail);

    IntEditFieldParams* my_params = static_cast<IntEditFieldParams*>(params_mem);
    my_params->min_value = min_value;
    my_params->max_value = max_value;
    my_params->lbl_signal_cb = lv_obj_get_signal_cb(label);
    my_params->ok_func = ok_func;
    lv_obj_set_user_data(label, params_mem);

    // Intercept cleanup signal to free parameter memory (TODO: use delete signal instead)

    lv_obj_set_signal_cb(label,
            [](lv_obj_t* label, lv_signal_t sign, void* param) -> lv_res_t
            {
                IntEditFieldParams* my_params = intEditFieldGetParams(label);
                if(sign == LV_SIGNAL_CLEANUP) {
                    lv_mem_free(static_cast<void*>(my_params));
                    lv_obj_set_user_data(label, NULL);
                }
                return my_params->lbl_signal_cb(label, sign, param);
            });

    // Display modal input window on click

    auto click_value_event = [] (lv_obj_t * btn, lv_event_t event)
    {
        if (event != LV_EVENT_CLICKED)
            return;
        getModalInputOneLine(
                // TODO: shouldn't be only positive
                KbInputMask::positive_integers, maxStrLen<int16_t>(), lv_obj_get_child(btn, NULL),
                // Validate entered value in okay callback
                [](lv_obj_t* label, const char* edited_text)-> bool
                {
                    IntEditFieldParams* params = intEditFieldGetParams(label);
                    int16_t entered_value = strtol(edited_text, nullptr, 10);
                    // Reopen modal entry window if input is invalid
                    if (entered_value < params->min_value || entered_value > params->max_value) {
                        static constexpr uint8_t max_len =
                                sizeof("Value must be between ") + maxStrLen<int16_t>() + sizeof(" and ") + maxStrLen<int16_t>();
                        char msg[max_len];
                        snprintf(msg, max_len, "Value must be between %d and %d", (int)params->min_value, (int)params->max_value);
                        createModalMbox(msg, ModalMboxType::okay, nullptr, nullptr);
                        return false;
                    }
                    // strip any leading zeros etc.
                    //lblSetIntText(label, entered_value);
                    if (params->ok_func != nullptr)
                        params->ok_func(entered_value);
                    return true;
                });
    };
    lv_obj_set_event_cb(btn, click_value_event);
    lv_obj_set_user_data(btn, static_cast<void*>(label));
    return btn;
}
