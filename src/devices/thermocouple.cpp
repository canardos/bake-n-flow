#include <cstdint>
#include "serial/spi_bus_stm32f1xx.h"
#include "drivers/max31856/max_31856.h"
#include "serial/spi.h"
#include "libpekin_stm32_hal.h"
#include "devices/peripherals.h"

using namespace Libp;
using namespace LibpStm32;

void setCs(bool active)
{
    Pins::max_spi_cs.set(!active);
    // MAX31856:
    // - min CS active -> SCLK = 100ns
    // - min CS inactive = 400ns
    delayUs(1);
}

Spi::SpiBus bus = Spi::SpiBus<SPI2_BASE>();
SpiRegisterOps busops(bus, setCs);

Max31856::Max31856 max_ic(busops);

static void initSpi()
{
    DefPin::spi2_sck.setAsOutput(OutputMode::alt_pushpull, OutputSpeed::high);
    DefPin::spi2_mosi.setAsOutput(OutputMode::alt_pushpull, OutputSpeed::high);
    DefPin::spi2_miso.setAsInput(InputMode::floating);

    Clk::enable<Clk::Apb1::spi2>();

    bus.start(
            Spi::MasterSlave::master,
            Spi::CpolCpha::cpha1cpol0,
            Spi::BaudRate::pclk_div_32,
            Spi::DataFrameFormat::bits_8,
            Spi::BitEndianess::msb_first);
}

void initThermocouple()
{
    initSpi();
    max_ic.configure(
            Max31856::Mode::continuous,
            Max31856::TcType::k_type,
            Max31856::ConversionMode::avg_8_samples,
            Max31856::NoiseFilter::freq_50hz);

    // gradient between ref. junction and IC sensor
    max_ic.setCjOffset(-1.5 / 0.0625);
}

int16_t readTemp()
{
    uint32_t raw_temp = max_ic.readTemp();
    return Max31856::decodeTemp(raw_temp);
}

