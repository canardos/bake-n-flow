#include <cstdint>
#include "clock_stm32f1xx.h"
#include "timer_stm32f1xx.h"
#include "pins_stm32f1xx.h"
#include "devices/peripherals.h"

using namespace LibpStm32;

static constexpr uint32_t mcu_freq = 72'000'000; // TODO: get dynamically
static constexpr uint8_t mains_freq = 60;

// SSR is zero-crossing type so the shortest
// duration it can be on/off is one half mains cycle
static constexpr uint8_t crossing_freq = mains_freq * 2;
static constexpr uint8_t power_steps = 100;
static constexpr uint32_t tim_resolution = mcu_freq / crossing_freq;
static constexpr uint32_t tim_period  = tim_resolution * power_steps;


static const Tim::Channel tim_channel = Tim::Channel::ch1;
static Tim::BasicTimer<TIM4_BASE> timer_;

void initOvenSsr()
{
    // Timer 4, channel 1 = PB6 (no remap)
    Clk::enable<Clk::Apb2::iopb>();
    GpioB::setOutputs<
            OutputMode::alt_pushpull,
            OutputSpeed::low,
            PinNb::ssr_pwm >();

    Clk::enable<Clk::Apb1::tim4>();

    timer_.initPwm(
            tim_channel,
            tim_period,
            0, // zero power out (off)
            true);
    timer_.enable();
}

void setOvenSsr(uint8_t power)
{
    // TODO: if power == 0, turn off timer/output
    timer_.setCompare(tim_channel, power * tim_resolution);
}

uint8_t getOvenSsr()
{
    // TODO: not working
    return timer_.getCompare(tim_channel) / tim_resolution;
}
