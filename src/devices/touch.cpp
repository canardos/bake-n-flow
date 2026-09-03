#include "main.h"

#include "lp_clock_stm32f1xx.h"
#include "touch/lp_resistive_ts_stm32f1xx.h"
#include "touch/lp_resistive_touch.h"

namespace {
    libp_stm32::ResistiveTs stm32_ts;
    libp::resist_touch::Screen touch_screen(stm32_ts, App::ui_width, App::ui_height);
}

libp::resist_touch::Screen& initTouchscreen()
{
    libp_stm32::clk::enable<libp_stm32::clk::Apb2::adc1>();
    stm32_ts.start(false);
    return touch_screen;
}
