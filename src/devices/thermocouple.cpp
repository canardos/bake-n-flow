#include "drivers/max31856/lp_max_31856.h"
#include "serial/lp_spi_bus.h"
#include "devices/peripherals.h"
#include "lp_libpekin_stm32_hal.h"
#include "serial/lp_spi_bus_stm32f1xx.h"
#include <cstdint>

using namespace libp;
using namespace libp_stm32;

// MAX31856 requires:
// - min CS active -> SCLK = 100ns
// - min CS inactive = 400ns
static struct {
    void set()
    {
        pins::max_spi_cs.set();
        delayUs(1);
    }
    void clear()
    {
        pins::max_spi_cs.clear();
        delayUs(1);
    }
} cs_pin;



static libp_stm32::spi::SpiBus bus = spi::SpiBus<SPI2_BASE>{};
static libp::SpiBus<bus, cs_pin> spi_bus{};
static max31856::Max31856<spi_bus> max_ic{};

static void initSpi()
{
    def_pin::spi2_sck.setAsOutput(OutputMode::alt_pushpull, OutputSpeed::high);
    def_pin::spi2_mosi.setAsOutput(OutputMode::alt_pushpull, OutputSpeed::high);
    def_pin::spi2_miso.setAsInput(InputMode::floating);

    clk::enable<clk::Apb1::spi2>();

    bus.start(
            spi::MasterSlave::master,
            spi::CpolCpha::cpha1cpol0,
            spi::BaudRate::pclk_div_32,
            spi::DataFrameFormat::bits_8,
            spi::BitEndianess::msb_first);
}

void initThermocouple()
{
    initSpi();
    max_ic.configure(
            max31856::Mode::continuous,
            max31856::TcType::k_type,
            max31856::ConversionMode::avg_8_samples,
            max31856::NoiseFilter::freq_50hz);

    // gradient between ref. junction and IC sensor
    max_ic.setCjOffset(-1.5 / 0.0625);
}

int16_t readTemp()
{
    uint32_t raw_temp = max_ic.readTemp();
    return max31856::decodeTemp(raw_temp);
}

