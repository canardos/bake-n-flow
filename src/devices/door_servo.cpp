#include <cstdint>
#include <tuple>
#include "clock_stm32f1xx.h"
#include "timer_stm32f1xx.h"
#include "pins_stm32f1xx.h"
#include "devices/peripherals.h"

using namespace LibpStm32;

constexpr uint32_t mcu_freq = 72'000'000;

// 1->2ms pulse every 20ms = 0->180deg for standard servo
// Servo in use uses 600 -> 2050us pulse
constexpr uint32_t us_period = 20'000;
constexpr uint32_t us_for_0_deg = 1'000;
constexpr uint32_t us_for_180_deg = 2'000;
constexpr uint32_t us_for_full_arc = us_for_180_deg - us_for_0_deg;
constexpr uint32_t n_degrees = 180;

constexpr uint32_t cycles_per_deg = us_for_full_arc / (double)n_degrees * mcu_freq / 1'000'000.0;
constexpr uint32_t cycles_for_0_deg = us_for_0_deg / 1'000'000.0 * mcu_freq;
constexpr uint32_t cycles_per_period = us_period / 1'000'000.0 * mcu_freq;

static const Tim::Channel tim_channel = Tim::Channel::ch1;
static Tim::BasicTimer<TIM5_BASE> timer_;

void initDoorServo()
{
    // Timer 5, channel 1 = PA0 (no remap)
    Clk::enable<Clk::Apb2::iopa>();
    GpioA::setOutputs<
            OutputMode::alt_pushpull,
            OutputSpeed::low,
            PinNb::door_servo_pwm>();
    Clk::enable<Clk::Apb1::tim5>();

    timer_.initPwm(
            tim_channel,
            cycles_per_period,
            cycles_for_0_deg, // Or will 0 mean no movement? Preferred
            true);
}

void setDoorServo(uint16_t angle)
{
    timer_.setCompare(tim_channel, cycles_for_0_deg + angle * cycles_per_deg);
}
