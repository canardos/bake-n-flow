/**
 * This file contains the guts of the menu/page architecture.
 *
 * The UI consists of a thin header with page title/status information and a
 * page "body".
 *
 * Each page is represented by a @p GuiPage object, which contains:
 * - container to show/hide when page is shown/hidden.
 * - title to show on the header when page is shown
 * - back function to call when back button is pressed from page.
 * - refresh function to call before showing the page.
 */
#ifndef UI_TEMPLATE_CPP_
#define UI_TEMPLATE_CPP_

#include <cstdio>
#include <ui/ui_common.h>
#include "lvgl/lvgl.h"
#include "app_settings.h"

static lv_obj_t* header_;
static lv_obj_t* status_label_;
static lv_obj_t* page_name_label_;
static lv_obj_t* back_btn_;

/// Container wrapper and label for each defined page.
/// Used to show/hide the various pages and update the page header.
static GuiPage gui_pages[] =
{
        // DO NOT REORDER - must be ordered per Pages enum

        nullptr, MenuLabel::main_menu, nullptr,  nullptr,

        // Reflow
        nullptr, MenuLabel::reflow, nullptr,  nullptr,

        // Edit reflow profile
        nullptr, MenuLabel::reflow, [] ()
        {
            createModalMbox(
                    "Cancel edit?",
                    ModalMboxType::yes_no,
                    [](){ showPage(Pages::reflow); },
                    nullptr );
        }, pageRefloweditRefresh,

        // Reflow run
        nullptr, MenuLabel::reflow, [] ()
        {
            createModalMbox(
                    "Cancel reflow?",
                    ModalMboxType::yes_no,
                    cancelReflow,
                    nullptr);
        }, pageReflowrunRefresh,

        // Bake
        nullptr, MenuLabel::bake, nullptr, pageBakeRefresh,// bake

        // Bake run
        nullptr, MenuLabel::bake, [] ()
        {
            createModalMbox(
                    "Cancel baking?",
                    ModalMboxType::yes_no,
                    cancelBake,
                    nullptr);
        }, nullptr,

        nullptr, MenuLabel::setup, pageSetupCancel, pageSetupRefresh,
        nullptr, MenuLabel::about, nullptr, nullptr,
        nullptr, MenuLabel::manual, cancelManualOvenOp, nullptr,
};

// Check that we have a page defined for each page defined in the Pages enum
static_assert(
        sizeof(gui_pages) == sizeof(GuiPage) * static_cast<uint8_t>(Pages::LEN),
        "Pages/pages enum/struct size mismatch" );

#ifdef SIMULATE_BRIGHTNESS

static lv_obj_t* scn_cover;
static lv_style_t scn_style;

#endif

void statusHeaderInit(void)
{
    header_ = lv_cont_create(lv_scr_act(), NULL);

    //lv_cont_set_fit(header, false, true); // vertical obj_align for text doesn't work if we use this
    static constexpr uint8_t status_text_vert_margin = Padding::narrow / 2;
    static const uint8_t hdr_height =
            (style_header_status.text.font->h_px * 2) +
            (status_text_vert_margin * 3);
    lv_obj_set_size(header_, LV_HOR_RES_MAX, hdr_height);
    lv_obj_set_pos(header_, 0, 0);
    lv_obj_set_style(header_, &style_header);

    // Status
    status_label_ = lv_label_create(header_, NULL);
    // Use max length for alignment
    statusHeaderUpdate(100, 9999);
    lv_label_set_style(status_label_, &style_header_status);

    // Page label and back buttons
    static constexpr uint8_t back_btn_width = LV_HOR_RES_MAX / 10;
    back_btn_ = createDefaultBtn(header_, LV_SYMBOL_LEFT);
    lv_obj_set_width(back_btn_, back_btn_width);
    lv_obj_align(back_btn_, NULL, LV_ALIGN_IN_LEFT_MID, 0, 0);
    lv_obj_set_hidden(back_btn_, true);

    page_name_label_ = lv_label_create(header_, NULL);
    lv_obj_align(status_label_, NULL, LV_ALIGN_IN_RIGHT_MID, -Padding::outer, 0);
    lv_obj_align(page_name_label_, NULL, LV_ALIGN_IN_LEFT_MID, Padding::outer, 0);

#ifdef SIMULATE_BRIGHTNESS

    lv_style_copy(&scn_style, &lv_style_transp);
    scn_cover = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(scn_cover, 480, 320);
    lv_obj_set_style(scn_cover, &scn_style);
    lv_obj_set_click(scn_cover, false);*/
#endif
}

#ifdef SIMULATE_BRIGHTNESS

void setTftLed(uint8_t brightness) {
    scn_style.body.main_color = LV_COLOR_BLACK;
    scn_style.body.grad_color = LV_COLOR_BLACK;
    scn_style.body.opa = 100 - brightness;
    lv_obj_set_parent(scn_cover, lv_scr_act());
    lv_obj_invalidate(scn_cover);
}

#endif

/**
 * @param power_level
 * @param temp 0.1degC units
 */
void statusHeaderUpdate(uint8_t power_level, uint16_t temp)
{
    static constexpr size_t max_len = sizeof("100%\n999.9°C");
    // TODO: can we just update the static text?
    //       How to trigger label update? seems no public LV function.
    static char buf[max_len];
    TempUnit current_units = getSettings().units;
    temp = getSettings().unitsToCurrentUnits(temp, TempUnit::celsius);
    if (power_level == 0)
        snprintf(buf, max_len, "Off\n%d.%d°%c",
                (int)temp/10, (int)temp%10,
                current_units == TempUnit::celsius ? 'C' : 'F');
    else
        // Ignore truncation warning - temp is always <1000
        snprintf(buf, max_len, "%d%%\n%d.%d°%c",
                (int)power_level, (int)temp/10, (int)temp%10,
                current_units == TempUnit::celsius ? 'C' : 'F');
    lv_label_set_static_text(status_label_, buf);
}


/// Set the container for a specific page (i.e. container
/// to be be shown/hidden when the page is en/disabled)
static void setPageContainer(Pages page, lv_obj_t* container)
{
    // remove any previous page
    uint8_t idx = Libp::enumBaseT(page);
    if (gui_pages[idx].container != NULL)
        lv_obj_del(gui_pages[idx].container);
    gui_pages[idx].container = container;
}

/// Create page container and assign to the specified page
lv_obj_t* createPage(Pages page)
{
    // TODO: we should probably use the top
    //       level page shell as the parent
    lv_obj_t* container = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_size(container, LV_HOR_RES_MAX, LV_VER_RES_MAX - lv_obj_get_height(header_));
    lv_cont_set_fit(container, LV_FIT_NONE);
    lv_obj_align(container, header_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style(container, &style_body);
    setPageContainer(page, container);
    return container;
}

/// Show the specified page, hide others, and update the header.
void showPage(Pages new_page)
{
    // static required for capture in lambda below
    static uint8_t new_page_idx;
    new_page_idx = Libp::enumBaseT(new_page);

    // Update any dynamic content on the new page prior to showing
    if (gui_pages[new_page_idx].refresh_func)
        gui_pages[new_page_idx].refresh_func();

    // If it's the main menu, hide the back button
    if (new_page_idx == 0) {
        lv_obj_set_hidden(back_btn_, true);
        lv_obj_align(page_name_label_, NULL, LV_ALIGN_IN_LEFT_MID, Padding::outer, 0);
    }
    // Otherwise show the back button and set the back press function
    else {
        lv_obj_set_hidden(back_btn_, false);
        lv_obj_align(page_name_label_, back_btn_, LV_ALIGN_OUT_RIGHT_MID, Padding::outer, 0);
        lv_obj_set_event_cb(back_btn_,
                [] (struct _lv_obj_t * obj, lv_event_t event)
                {
                    if (event == LV_EVENT_CLICKED) {
                        if (gui_pages[new_page_idx].back_func != NULL)
                            gui_pages[new_page_idx].back_func();
                        else
                            showPage(Pages::main_menu);
                    }
                });
    }
    lv_label_set_static_text(page_name_label_, gui_pages[new_page_idx].title);
    for (uint8_t i = 0; i < Libp::enumBaseT(Pages::LEN); i++) {
        if (gui_pages[i].container != NULL)
            lv_obj_set_hidden(gui_pages[i].container, i != new_page_idx);
    }
}

#endif /* UI_TEMPLATE_ */
