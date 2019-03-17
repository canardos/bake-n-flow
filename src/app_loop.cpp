#include <cstdint>
#include <pid/pid_algo.h>
#include "oven/oven_hardware.h"
#include "oven/oven_operation.h"
#include "ui/ui.h"
#include "libpekin.h"

static OvenHardware oven_;

static uint64_t getMillis() { return Libp::getMillis(); }

// 40 10 5

static Libp::PidAlgo pid_algo_(
        40, 10, 5,
        0, 100,
        // getMillis overflow is okay as oven won't be on for months.
        getMillis);

static OvenOperation oven_operation_(oven_, pid_algo_, getMillis);

static void processOvenEvents()
{
    oven_operation_.process();
}

static void updateUi()
{
    statusHeaderUpdate(oven_.getPowerLevel(), oven_.getTemp());
    switch (oven_operation_.getState()) {
    case OvenOperation::State::baking:
        pageBakerunRefreshUi(oven_operation_.getElapsedTime());
        break;
    case OvenOperation::State::reflow_tracking:
        pageReflowrunRefreshUi(oven_operation_.getElapsedTime(), oven_.getTemp(), false);
        break;
    case OvenOperation::State::reflow_cooling:
        pageReflowrunRefreshUi(oven_operation_.getElapsedTime(), oven_.getTemp(), true);
        break;
    case OvenOperation::State::reflow_warming:
    case OvenOperation::State::manual_pwr:
    case OvenOperation::State::manual_temp:
    case OvenOperation::State::stopped:
        // nothing to do
        break;
    }
}

#define PRINT_DEBUG_INFO 0

#if PRINT_DEBUG_INFO
#include "lvgl/lvgl.h"
static void outputDebugInfo()
{
    static uint32_t last = 0;
    uint32_t now = Libp::getMillis();
    if ((now - last) > 1000) {
        lv_mem_monitor_t mon;
        lv_mem_monitor(&mon);
        getErrHndlr().report("%d,%d,%d,%d,%d,%d\r\n",
                now,
                oven_.getPowerLevel(),
                oven_.getTemp(),
                (int)mon.total_size - mon.free_size,
                (int)mon.free_size,
                (int)mon.free_biggest_size);
        last = now;
    }
}
#endif

#include "app_settings.h"

void runMainProgLoop()
{
    AppSettings::Data& settings = getSettings();

    pid_algo_.setPidParams(
            settings.pid_params.kp,
            settings.pid_params.ki,
            settings.pid_params.kd);
    buildUi(&oven_operation_, &pid_algo_);
    uint32_t timestamp_ms = Libp::getMillis();

    while (true) {
        lv_task_handler();
        uint32_t now = Libp::getMillis();
        lv_tick_inc(now - timestamp_ms);
        timestamp_ms = now;
        processOvenEvents();
        updateUi();
#if PRINT_DEBUG_INFO
        outputDebugInfo();
#endif
        Libp::delayMs(5);
    }
}
