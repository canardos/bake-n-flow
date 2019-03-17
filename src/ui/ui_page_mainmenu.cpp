#include "ui/ui_common.h"
#include "ui/ui_defs.h"
#include "lvgl/lvgl.h"

/*static void shutdown(void) {
    //printf("Shutting down\n");
    //exit(0);
}*/
#include <cstdio>
// TODO: does this need C calling conv?
static void menuBtnClickHandler(lv_obj_t * btnm, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED)
        return;
    switch (lv_btnm_get_active_btn(btnm)) {
    case 0:
        showPage(Pages::reflow);
        break;
    case 1:
        showPage(Pages::bake);
        break;
    case 2:
        showPage(Pages::setup);
        break;
    case 3:
        showPage(Pages::about);
        break;
    case 4:
        showPage(Pages::advanced);
        break;
/*    case 4:
        createModalMbox("Shutdown?", ModalMboxType::yes_no, shutdown, nullptr);
        break;*/
    }
}

void pageMainmenuInit()
{
    lv_obj_t* page = createPage(Pages::main_menu);
    static const char * btnm_map[] = {
        MenuLabel::reflow,      // 0
        MenuLabel::bake, "\n",  // 1
        MenuLabel::setup,       // 2
        MenuLabel::about, "\n", // 3
        MenuLabel::manual,    // 4
        ""
    };
    lv_obj_t* btnm_menu = createDefaultBtnm(page, btnm_map);
    lv_obj_set_size(btnm_menu, LV_HOR_RES_MAX, lv_obj_get_height(page));
    lv_obj_set_event_cb(btnm_menu, menuBtnClickHandler);
}
