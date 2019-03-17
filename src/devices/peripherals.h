#ifndef MYGPIO_H_
#define MYGPIO_H_

#include <cstdint>
#include "pins_stm32f1xx.h"
#include "clock_stm32f1xx.h"
#include "fsmc_stm32f1xx.h"

/*
 * STM32F103V[C/D/E]
 *
 * -----------------------
 * TFT LCD
 * -----------------------
 * TFT_Reset   : PC5
 *
 * TFT_RD      : PD4 (FSMC_NOE)
 * TFT_WR      : PD5 (FSMC_NWE)
 * TFT_CS      : PD7 (FSMC_NE1)
 * TFT_RS      : PE4 (FSMC_A20)
 *
 * TFT_D0      : PD14 (FSMC_D0)
 * TFT_D1      : PD15 (FSMC_D1)
 * TFT_D2      : PD0  (FSMC_D2)
 * TFT_D3      : PD1  (FSMC_D3)
 * TFT_D4      : PE7  (FSMC_D4)
 * TFT_D5      : PE8  (FSMC_D5)
 * TFT_D6      : PE9  (FSMC_D6)
 * TFT_D7      : PE10 (FSMC_D7)
 * TFT_D8      : PE11 (FSMC_D8)
 * TFT_D9      : PE12 (FSMC_D9)
 * TFT_D10     : PE13 (FSMC_D10)
 * TFT_D11     : PE14 (FSMC_D11)
 * TFT_D12     : PE15 (FSMC_D12)
 * TFT_D13     : PD8  (FSMC_D13)
 * TFT_D14     : PD9  (FSMC_D14)
 * TFT_D15     : PD10 (FSMC_D15)
 *
 * TFT_IM0     : PD11
 * TFT_IM1     : PD12
 * TFT_IM2     : PD13
 *
 * -----------------------
 * Resistive Touchscreen
 * -----------------------
 * TOUCH_X_NEG : PA3
 * TOUCH_X_POS : PA5 (ADC1/2 in5)
 * TOUCH_Y_NEG : PA6
 * TOUCH_Y_POS : PA7 (ADC1/2 in7)
 *
 * -----------------------
 * MAX31856 SPI Connection
 * -----------------------
 * MAX_CS      : PC6
 * MAX_SCK     : PB13 (SPI2)
 * MAX_MISO    : PB14 (SPI2)
 * MAX_MOSI    : PB15 (SPI2)
 * MAX_DDRY    : PC8
 * MAX_FAULT   : PC7
 *
 * -----------------------
 * Other
 * -----------------------
 * AUDIO_EN    : PA2
 * AUDIO_OUT   : PA4 (DAC)
 * SERVO_PWM   : PA0 (TIM5_CH1)
 * TFT_LED_PWM : PB1 (TIM3_CH4 / TIM1_CH3N)
 * SSR_PWM     : PB6 (TIM4_CH1)
 * PCB_LED     : PE5
 */

namespace PinNb {

inline constexpr uint8_t pcb_led = 5;
inline constexpr uint8_t audio_en = 2;
inline constexpr uint8_t audio_out = 4;

// SPI for MAX31856
inline constexpr uint8_t max_spi_cs = 6;

// TFT LCD (ex FSMC)
inline constexpr uint8_t im0 = 11;
inline constexpr uint8_t im1 = 12;
inline constexpr uint8_t im2 = 13;
inline constexpr uint8_t tft_reset = 5;
inline constexpr uint8_t tft_cs = 7;
inline constexpr uint8_t tft_rd = 4;
inline constexpr uint8_t tft_wr = 5;

// PWM
inline constexpr uint8_t door_servo_pwm = 0;
inline constexpr uint8_t ssr_pwm = 6;
inline constexpr uint8_t tft_led_pwm = 1;

}

namespace Pins {

static inline LibpStm32::PinE<PinNb::pcb_led> pcb_led;
static inline LibpStm32::PinA<PinNb::audio_en> audio_en;

static inline LibpStm32::PinC<PinNb::max_spi_cs> max_spi_cs;

// TFT LCD (ex FSMC)
static inline LibpStm32::PinD<PinNb::im0> tft_im0;
static inline LibpStm32::PinD<PinNb::im1> tft_im1;
static inline LibpStm32::PinD<PinNb::im2> tft_im2;
static inline LibpStm32::PinC<PinNb::tft_reset> tft_reset;

}

namespace Details {
inline __attribute__((always_inline))
void initGpio()
{
    using namespace LibpStm32;

    Clk::enable<
            Clk::Apb2::iopa,
            Clk::Apb2::iopb,
            Clk::Apb2::iopc,
            Clk::Apb2::iopd,
            Clk::Apb2::iope>();

    Pins::pcb_led.clear();
    Pins::audio_en.clear();
    Pins::max_spi_cs.clear();
    Pins::tft_im0.clear();
    Pins::tft_im1.clear();
    Pins::tft_im2.clear();
    Pins::tft_reset.clear();

    GpioA::setOutputs<OutputMode::pushpull, OutputSpeed::low,
            PinNb::audio_en >();

    GpioE::setOutputs<OutputMode::pushpull, OutputSpeed::low,
            PinNb::pcb_led >();

    GpioC::setOutputs<OutputMode::pushpull, OutputSpeed::low,
            PinNb::tft_reset,
            PinNb::max_spi_cs >();

    // SPI setup in thermocouple init

    GpioD::setOutputs<OutputMode::pushpull, OutputSpeed::low,
            PinNb::im0,
            PinNb::im1,
            PinNb::im2 >();

    // FSMC

    GpioD::setOutputs<OutputMode::alt_pushpull, OutputSpeed::high,
            8, 9, 10, 14, 15, 0, 1, PinNb::tft_rd, PinNb::tft_wr, PinNb::tft_cs >();

    GpioE::setOutputs<OutputMode::alt_pushpull, OutputSpeed::high,
            4, 7, 8, 9, 10, 11, 12, 13, 14, 15 >();

    // DAC

    GpioA::setInputs<InputMode::analog,
            PinNb::audio_out >();

    // USART

    DefPin::usart1_tx.setAsOutput(OutputMode::alt_pushpull, OutputSpeed::low);
    DefPin::usart1_rx.setAsInput(LibpStm32::InputMode::floating);
}

inline __attribute__((always_inline))
void initFsmcSram()
{
    constexpr LibpStm32::Fsmc::SramNorCfg cfg = []() {
        LibpStm32::Fsmc::Config config;

        config.timing.addr_setup = 5;
        config.timing.addr_hold = 2;
        config.timing.data_phase_dur = 3;

        config.addr_data_mux = false;
        config.mem_type = LibpStm32::Fsmc::Config::MemType::sram;
        config.flash_access_en = false;

        return config.makeConfig();
    }();
    LibpStm32::Clk::enable<LibpStm32::Clk::Ahb::fsmc>();
    LibpStm32::Fsmc::NorSram<1>::init(cfg);
    LibpStm32::Fsmc::nadvDisconnected(); // don't use NADV output
}
} // namespace Details

inline __attribute__((always_inline))
void initPeripherals()
{
    Details::initGpio();
    Details::initFsmcSram();
}

#endif /* MYGPIO_H_ */
