#ifndef UI_DEFS_H_
#define UI_DEFS_H_

#include <cstdint>
#include "lvgl/lvgl.h"

/**
 * Create a checkbox with a static text label and apply the default checkbox
 * styles.
 *
 * @param parent parent of the new checkbox. May be NULL.
 * @param text null terminated text for the label. Must exist as long as this
 *             checkbox exists.
 * @param checked
 * @return
 */
lv_obj_t* createDefaultCb(lv_obj_t* parent, const char* text, bool checked);


/**
 * Create a checkbox styled as a radio button with a static text label and
 * apply the default checkbox styles.
 *
 * @param parent parent of the new checkbox. May be NULL.
 * @param text null terminated text for the label. Must exist as long as this
 *             checkbox exists.
 * @param checked
 * @return
 */
lv_obj_t* createDefaultRadioBtn(lv_obj_t* parent, const char* text, bool checked);


/**
 * Create a button with a static text label and apply the default button
 * styles.
 *
 * @param parent parent of the new button. May be NULL.
 * @param text null terminated text for the label. Must exist as long as this
 *             button exists.
 *
 * @return newly created button object pointer
 */
lv_obj_t* createDefaultBtn(lv_obj_t* parent, const char* text);

/**
 * Create a button matrix and applies the default button styles.
 *
 * @param parent parent of the new button. May be NULL.
 * @param map pointer a string array. The last string has to be: "". Use "\n"
 *            to make a line break.
 *
 * @return newly created button matrix object pointer
 */
lv_obj_t* createDefaultBtnm(lv_obj_t* parent, const char * map[]);

/**
 * Create a button and apply the default button styles.
 *
 * @param parent parent of the new button. May be NULL.
 *
 * @return newly created button object pointer
 */
lv_obj_t* createDefaultBtn(lv_obj_t* parent);

/**
 * Create a label with a static text string.
 *
 * @param parent
 * @param text null terminated text for the label. Must exist as long as this
 *             label exists.
 * @return the newly created label object pointer
 */
lv_obj_t* createDefaultStaticLabel(lv_obj_t* parent, const char* text);

/**
 * Create a label with a static text string and set position.
 *
 * @param parent
 * @param text null terminated text for the label. Must exist as long as this
 *             label exists.
 * @param x new distance from the left side of the parent
 * @param y new distance from the top of the parent
 *
 * @return the newly created label object pointer
 */
lv_obj_t* createDefaultStaticLabel(lv_obj_t* parent, const char* text, lv_coord_t x, lv_coord_t y);


namespace MenuLabel {
    inline constexpr const char* main_menu = "Main Menu";
    inline constexpr const char* reflow    = "Reflow";
    inline constexpr const char* bake      = "Bake";
    inline constexpr const char* setup     = "Setup";
    inline constexpr const char* about     = "About";
    inline constexpr const char* manual    = "Advanced";
}

enum class Pages : uint8_t {
    // Main menu page must be value 0
    // DO NOT REORDER - must match order of gui_pages GuiPage array in ui_template.cpp
    main_menu = 0,
    reflow,
    reflow_edit_profile,
    reflow_run,
    bake,
    bake_run,
    setup,
    about,
    advanced,
    LEN
};

/// Pointer to function to execute when back button is pressed
using BackBtnFunc = void (*)(void);

/// Pointer to function to call prior to showing page
using PageRefreshFunc = void (*)(void);

struct GuiPage {
    /// Container to show/hide when page is selected/deselected
    lv_obj_t* container;
    /// Title for header bar
    const char* title;
    /// Called when the back button is pressed.
    /// Use NULL to return to main menu
    BackBtnFunc back_func;
    /// Called prior to showing page. Update any dynamic content here.
    /// Use null for no refresh
    PageRefreshFunc refresh_func;
};


#endif /* UI_DEFS_H_ */
