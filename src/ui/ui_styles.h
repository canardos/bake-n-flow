#ifndef SRC_UI_UI_STYLES_H_
#define SRC_UI_UI_STYLES_H_

#include "lvgl/lvgl.h"

namespace Padding {
    inline constexpr uint8_t narrow = 10;
    inline constexpr uint8_t outer = LV_DPI / 9;
    inline constexpr uint8_t inner = LV_DPI / 7;
}

/// constexpr replacement for LVGL macro/inline functions
inline constexpr lv_color_t makeColor(uint8_t r8, uint8_t g8, uint8_t b8) {
    lv_color_t color = {0};
#if LV_COLOR_16_SWAP == 0
    color.ch.blue  = b8 >> 3;
    color.ch.green = g8 >> 2;
    color.ch.red   = r8 >> 3;
#else
    color.ch.green_h = g8 >> 5;
    color.ch.red = r8 >> 3;
    color.ch.blue = b8 >> 3;
    color.ch.green_l = (g8 >> 2) & 0x7;
#endif
    return color;
}

inline constexpr lv_color_t color_primary = makeColor(93,196,231);
inline constexpr lv_color_t color_primary_dk = makeColor(73,176,211);
inline constexpr lv_color_t color_bg = LV_COLOR_WHITE;
inline constexpr lv_color_t color_text = LV_COLOR_BLACK;
inline constexpr lv_color_t color_bg_highlight = makeColor(230,230,230);
inline constexpr lv_color_t color_dark = makeColor(80,97,115);


/*
 * The below functions exist to get the styles out of RAM and into
 * flash (i.e. static const) while keeping the ability to set them up using
 * field names since designated initializers aren't legal in C++ and we don't
 * wan't to have to specify every field.
 */


/*
 * Text:
 * - large light : style_header
 * - large dark  : style_chart
 * - small light : style_header_status
 * - small dark  : ???
 */

/// Equivalent to lv_style_scr (v5.3)
inline static constexpr lv_style_t getStyleDefault()
{
    lv_style_t s { };
    /*Screen style*/
    s.glass               = 0;
    s.body.opa            = LV_OPA_COVER;
    s.body.main_color     = LV_COLOR_WHITE;
    s.body.grad_color     = LV_COLOR_WHITE;
    s.body.radius         = 0;
    s.body.padding.left   = 0;
    s.body.padding.right  = 0;
    s.body.padding.top    = 0;
    s.body.padding.bottom = 0;
    s.body.padding.inner  = LV_DPI / 12;

    s.body.border.color = LV_COLOR_BLACK;
    s.body.border.opa   = LV_OPA_COVER;
    s.body.border.width = 0;
    s.body.border.part  = LV_BORDER_FULL;

    s.body.shadow.color = LV_COLOR_GRAY;
    s.body.shadow.type  = LV_SHADOW_FULL;
    s.body.shadow.width = 0;

    s.text.opa          = LV_OPA_COVER;
    s.text.color        = makeColor(0x30, 0x30, 0x30);
    s.text.sel_color    = makeColor(0x55, 0x96, 0xd8);
    s.text.font         = LV_FONT_DEFAULT;
    s.text.letter_space = 2;
    s.text.line_space   = 2;

    s.image.opa     = LV_OPA_COVER;
    s.image.color   = makeColor(0x20, 0x20, 0x20);
    s.image.intense = LV_OPA_TRANSP;

    s.line.opa     = LV_OPA_COVER;
    s.line.color   = makeColor(0x20, 0x20, 0x20);
    s.line.width   = 2;
    s.line.rounded = 0;
    return s;
}

///
inline constexpr lv_style_t style_header = []() {
    lv_style_t s = getStyleDefault();

    s.image.color = makeColor(0xf0, 0xf0, 0xf0); // TODO: is this needed?
    s.body.main_color = color_dark;
    s.body.grad_color = color_dark;
    s.text.color = color_bg;
    s.text.font = &lv_font_dejavu_30;
    // Used for chart lines - dark
    s.line.color = color_dark;
    s.line.width = 3;
    return s;
}();

inline constexpr lv_style_t style_header_status = []() {
    lv_style_t s = style_header;
    s.text.font = &lv_font_dejavu_20;
    // Used for chart lines - light
    s.line.color = color_primary;
    s.line.width = 4;
    return s;
}();

inline constexpr lv_style_t style_body = []() {
    lv_style_t s = getStyleDefault();
    s.body.main_color = color_bg;
    s.body.grad_color = color_bg;
    s.text.color = color_text;
    s.text.font = &lv_font_dejavu_20;
    // Padding used between buttons in button matrices
    s.body.padding.top = Padding::outer;
    s.body.padding.bottom = Padding::outer;
    s.body.padding.left = Padding::outer;
    s.body.padding.right = Padding::outer;
    s.body.padding.inner = Padding::inner;
    return s;
}();

// TODO: work out how to avoid this style
inline constexpr lv_style_t style_body_nopad = []() {
    lv_style_t s = style_body;
    s.body.padding.top = 0;
    s.body.padding.bottom = 0;
    s.body.padding.left = 0;
    s.body.padding.right = 0;
    return s;
}();


inline constexpr lv_style_t style_main_btn = []() {
    lv_style_t s = getStyleDefault();

    // TODO: these are the old inherited syles - which are important?
    s.body.padding.inner = LV_DPI / 10;
    s.body.border.opa = LV_OPA_70;
    s.body.shadow.color = LV_COLOR_GRAY;
    s.body.shadow.width = 0;
    s.text.color = makeColor(0xff, 0xff, 0xff);
    s.image.color = makeColor(0xff, 0xff, 0xff);
    // end
    s.body.main_color = color_primary;
    s.body.grad_color = color_primary;
    s.body.radius = 1;
    s.body.border.color = color_primary_dk;
    s.body.border.width = 2;
    s.body.padding.top    = Padding::inner;
    s.body.padding.bottom = Padding::inner;
    s.body.padding.left   = Padding::inner;
    s.body.padding.right  = Padding::inner;
    return s;
}();

inline constexpr lv_style_t style_main_btn_press = []() {
    lv_style_t s = style_main_btn;
    s.body.main_color = color_primary_dk;
    s.body.grad_color = color_primary_dk;
    return s;
}();

///
inline constexpr  lv_style_t style_main_btn_disabled = []() {
    lv_style_t s = style_main_btn;
    s.body.main_color = color_bg_highlight;
    s.body.grad_color = color_bg_highlight;
    s.body.border.color = color_dark;
    s.text.color = color_dark;
    return s;
}();

///
inline constexpr  lv_style_t style_chart = []() {
    lv_style_t s = style_body;
    s.body.main_color = color_bg_highlight;
    s.body.grad_color = color_bg_highlight;
    s.line.color = color_bg;
    s.text.font = &lv_font_dejavu_30;
    // for radio buttons (so chart has rounded cnrs)
    s.body.radius = 10;
    return s;
}();

// Used in various places - checkboxes etc.
inline constexpr lv_style_t style_droplist_body = []() {
    lv_style_t s = style_body;
    s.body.main_color = color_bg_highlight;
    s.body.grad_color = color_bg_highlight;
    s.body.padding.top = Padding::inner;
    s.body.padding.bottom = Padding::inner;
    s.body.padding.left = Padding::inner;
    s.body.padding.right = Padding::inner;
    s.text.line_space= Padding::inner;
    return s;
}();

inline constexpr lv_style_t style_radio_btn = []() {
    lv_style_t s = style_main_btn;
    s.body.radius = 10;
    return s;
}();


#endif /* SRC_UI_UI_STYLES_H_ */
