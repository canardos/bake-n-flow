#include <cstdint>
#include <tuple>
#include "tft_led.h"
#include "clock_stm32f1xx.h"
#include "timer_stm32f1xx.h"
#include "pins_stm32f1xx.h"
#include "devices/peripherals.h"

using namespace LibpStm32;

static constexpr uint32_t mcu_freq = 72'000'000;
static constexpr uint32_t led_freq = 50'000; // high enough to avoid audio hum on amp
static constexpr uint32_t min_brightness = 10;
static constexpr uint32_t max_brightness = 90;
static constexpr uint32_t range = led_resolution * 2 - max_brightness + min_brightness;
static constexpr uint32_t cycles_per_period = mcu_freq / led_freq;
static constexpr uint32_t cycles_per_step = cycles_per_period / range;

static constexpr Tim::Channel tim_channel = Tim::Channel::ch4;
static Tim::BasicTimer<TIM3_BASE> timer_;

void initTftLed(uint8_t brightness)
{
    Clk::enable<Clk::Apb2::iopb>();
    GpioB::setOutputs<
            OutputMode::alt_pushpull,
            OutputSpeed::low,
            PinNb::tft_led_pwm>();

    Clk::enable<Clk::Apb1::tim3>();

    timer_.initPwm(
            tim_channel,
            cycles_per_period,
            (brightness + min_brightness) * cycles_per_step,
            true);

    timer_.enable();
    setTftLed(brightness);
}


void setTftLed(uint8_t brightness)
{
    timer_.setCompare(tim_channel, (brightness + min_brightness) * cycles_per_step);
}
