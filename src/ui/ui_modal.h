#ifndef UI_MODAL_BOX_H_
#define UI_MODAL_BOX_H_

#include <cstdint>
#include "ui/ui_common.h"
#include "lvgl/lvgl.h"

enum class KbInputMask : uint8_t {
    // DO NOT REORDER
    integers,
    positive_integers,
    string
};

enum class ModalMboxType : uint8_t {
    okay, yes_no
};

/// TODO
using InputOneLineOkayFunc = bool (*)(lv_obj_t* label, const char* edited_text);

/**
 * Opens a modal keyboard to edit an existing label. Initial text will be taken
 * from the label.
 *
 * @param mask input type
 * @param max_length maximum allowable text length
 * @param label_to_update pointer to lv_label object to update if okay is
 *                        pressed.
 * @param okay_func function to call after okay is pressed but prior to
 *                  updating label. Can do validation here. Return false for
 *                  validation failure.
 *                  Use NULL for no callback.
 */
void getModalInputOneLine(
        KbInputMask mask, uint8_t max_length, lv_obj_t* label_to_update,
        InputOneLineOkayFunc okay_func);


/**
 * Create a modal okay or yes/no message box.
 * @p label will copied and may be freed after this function returns.
 *
 * @param label text to display on dialog box above yes/no buttons. Will be
 *              copied.
 * @param type     message box type
 * @param yes_func function to call if yes or okay is pressed. Use NULL to do
 *                 nothing.
 * @param no_func  function to call if no is pressed. Ignored in okay message
 *                 boxes. Use NULL to do nothing.
 */
void createModalMbox(const char* label, ModalMboxType type, void (*yes_func)(), void (*no_func)());

/**
 * Create a modal message dialog with okay button.
 * @p label will copied and may be freed after this function returns.
 *
 * @param label text to display on dialog. Will be copied.
 * @param okay_func function to call after okay is pressed. Use NULL to do
 *                  nothing.
 */
//void createModalOkay(const char* label, void (*okay_func)());


/// function to call after editing intEditField
using IntEditFieldOkayFunc = void (*)(int16_t new_value);

/**
 * Creates an edit box consisting of a button and single label child.
 *
 * Note that @p value is not checked to comply with @p min_value or
 * @p max_value.
 *
 * @param parent
 * @param init_value
 * @param min_value minimum allowable value. Validated on okay press
 * @param max_value maximum allowable value. Validated on okay press
 * @param width
 * @param height
 * @param ok_func called after value is edited and ok is pressed. Function will
 *                be called even if value was not changed. May be NULL
 * @return button object. The button user data is a pointer to the label.
 */
lv_obj_t* intEditFieldCreate(lv_obj_t* parent,
        int16_t init_value, int16_t min_value, int16_t max_value,
        uint8_t width, uint8_t height, IntEditFieldOkayFunc ok_func);

long intEditFieldGetValue(lv_obj_t* btn);
void intEditFieldSetValue(lv_obj_t* btn, long value);



#endif /* UI_MODAL_BOX_H_ */
