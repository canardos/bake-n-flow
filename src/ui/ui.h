#ifndef SRC_UI_UI_H_
#define SRC_UI_UI_H_

#include "oven/oven_operation.h"
#include "pid/pid_algo.h"
#include "ui/ui_common.h"

inline
void buildUi(OvenOperation* oven_operation, Libp::PidAlgo* pid_algo)
{
    statusHeaderInit();
    pageMainmenuInit();
    pageReflowInit(oven_operation);
    pageRefloweditInit();
    pageReflowrunInit();
    pageBakeInit(oven_operation);
    pageBakerunInit();
    pageSetupInit(pid_algo);
    pageAboutInit();
    pageManualOvenOp(oven_operation);
    showPage(Pages::main_menu);
}

#endif /* SRC_UI_UI_H_ */
