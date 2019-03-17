/**
 * This file contains the functions that define the external interface of the
 * UI.
 */
#ifndef UI_PAGES_H_
#define UI_PAGES_H_

#include <pid/pid_algo.h>
#include <cstdint>
#include "oven/oven_operation.h"
//#include "pid/pid_ctrl.h"
#include "lvgl/lvgl.h"
#include "ui/ui_defs.h"

// Common

/**
 * Create the top level page structure with status header, back button etc.
 *
 * Individual pages are created as containers and sized to sit just below the
 * header. They are created once at startup and shown/hidden as required.
 */
void statusHeaderInit();

/**
 * Update temp/power level status in the header bar.
 *
 * @param power_level
 * @param temp tenths of a degree Celcius
 */
void statusHeaderUpdate(uint8_t power_level, uint16_t temp);

/// Create page container and assign to the specified page
lv_obj_t* createPage(Pages page);

/// Show the specified page, hide others, and update the header.
void showPage(Pages new_page);


/* ========================
 * Individual Page Inits
 * ======================== */

void pageMainmenuInit();

void pageAboutInit();

//void pageSetupInit(PidCtrl<float>* pid);
void pageSetupInit(Libp::PidAlgo* pid);

void pageBakeInit(OvenOperation* oven_operation);

void pageBakerunInit();

void pageReflowInit(OvenOperation* oven_operation);

void pageRefloweditInit();

void pageReflowrunInit();

void pageManualOvenOp(OvenOperation* oven_operation);

/* ========================
 * Individual Page Updates
 * ======================== */

void pageReflowRefresh();

/// Refresh data using currently selected profile
void pageRefloweditRefresh();

// Refresh pages on settings change

/// Refresh data using currently selected profile
void pageReflowrunRefresh();

/// ????Update time and progress????
void pageBakeRefresh();

// Refresh UI for running operations

void pageBakerunRefreshUi(uint16_t elapsed_time_sec);

void pageReflowrunRefreshUi(uint16_t elapsed_time_s, uint16_t oven_temp, bool cooling);

/**
 *
 * @param time_mins
 * @param temp in currently set units
 */
void pageBakerunSetBakeParams(uint16_t time_mins, uint16_t temp);

void pageSetupRefresh();

void pageSetupCancel();

/* ========================
 * Callbacks used by the UI
 * to cancel running
 * operations
 * ======================== */


/// Stop oven and return to reflow page
void cancelReflow();

/// Stop oven and return to bake page
void cancelBake();

/// Stop manual oven and return to main menu
void cancelManualOvenOp();

#endif /* UI_PAGES_H_ */
