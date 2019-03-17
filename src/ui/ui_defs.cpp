#include "lvgl/lvgl.h"
#include "ui_common.h"

lv_obj_t* createDefaultCb(lv_obj_t* parent, const char* text, bool checked)
{
    lv_obj_t* cb =  lv_cb_create(parent, NULL);
    lv_cb_set_style(cb, LV_CB_STYLE_BOX_REL, &style_droplist_body);
    lv_cb_set_style(cb, LV_CB_STYLE_BOX_PR, &style_main_btn_press);     // ideally should have
    lv_cb_set_style(cb, LV_CB_STYLE_BOX_TGL_PR, &style_main_btn_press); // dark pressed style
    // Checked style
    lv_cb_set_style(cb, LV_CB_STYLE_BOX_TGL_REL, &style_main_btn);
    lv_cb_set_static_text(cb, text);
    lv_cb_set_checked(cb, checked);
    return cb;
}

lv_obj_t* createDefaultRadioBtn(lv_obj_t* parent, const char* text, bool checked)
{
    lv_obj_t* cb =  lv_cb_create(parent, NULL);
    lv_cb_set_static_text(cb, text);
    lv_cb_set_checked(cb, checked);
    lv_cb_set_style(cb, LV_CB_STYLE_BOX_REL, &style_chart);
    lv_cb_set_style(cb, LV_CB_STYLE_BOX_PR, &style_radio_btn);     // ideally should have
    lv_cb_set_style(cb, LV_CB_STYLE_BOX_TGL_PR, &style_radio_btn); // dark pressed style
    // Checked style
    lv_cb_set_style(cb, LV_CB_STYLE_BOX_TGL_REL, &style_radio_btn);
    return cb;
}

lv_obj_t* createDefaultBtn(lv_obj_t* parent)
{
    lv_obj_t* btn = lv_btn_create(parent, NULL);
    lv_btn_set_style(btn, LV_BTN_STYLE_REL, &style_main_btn);
    lv_btn_set_style(btn, LV_BTN_STYLE_PR, &style_main_btn_press);
    lv_btn_set_style(btn, LV_BTN_STYLE_INA, &style_main_btn_disabled);
    lv_btn_set_fit(btn, LV_FIT_TIGHT);
    return btn;
}

lv_obj_t* createDefaultBtnm(lv_obj_t* parent, const char * map[])
{
    lv_obj_t* btnm = lv_btnm_create(parent, NULL);
    lv_btnm_set_map(btnm, map);
    lv_btnm_set_style(btnm, LV_BTNM_STYLE_BG, &style_body);
    lv_btnm_set_style(btnm, LV_BTNM_STYLE_BTN_REL, &style_main_btn);
    lv_btnm_set_style(btnm, LV_BTNM_STYLE_BTN_PR, &style_main_btn_press);
    lv_btnm_set_style(btnm, LV_BTNM_STYLE_BTN_INA, &style_main_btn_disabled);
    return btnm;
}


lv_obj_t* createDefaultBtn(lv_obj_t* parent, const char* text)
{
    lv_obj_t* btn = createDefaultBtn(parent);
    lv_obj_t* label = lv_label_create(btn, NULL);
    lv_label_set_static_text(label, text);
    return btn;
}


lv_obj_t* createDefaultStaticLabel(lv_obj_t* parent, const char* text)
{
    lv_obj_t* label = lv_label_create(parent, NULL);
    lv_label_set_static_text(label, text);
    return label;
}

lv_obj_t* createDefaultStaticLabel(lv_obj_t* parent, const char* text, lv_coord_t x, lv_coord_t y)
{
    lv_obj_t* label = createDefaultStaticLabel(parent, text);
    lv_obj_set_pos(label, x, y);
    return label;
}
