#include "main.h"
#include "touch/resistive_touch.h"
#include "touch/stm32_resistive_ts.h"
#include "clock_stm32f1xx.h"

namespace {
    LibpStm32::ResistiveTs stm32_ts;
    Libp::ResistiveTouch::Screen touch_screen(stm32_ts, App::ui_width, App::ui_height);
}

Libp::ResistiveTouch::Screen& initTouchscreen()
{
    LibpStm32::Clk::enable<LibpStm32::Clk::Apb2::adc1>();
    stm32_ts.start(false);
    return touch_screen;
}
