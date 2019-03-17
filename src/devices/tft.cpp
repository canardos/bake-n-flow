#include "display/ili9481_cmd_defs.h"
#include "display/mipi_dcs_tft_driver.h"
#include "display/lcd_bus_fsmc.h"

#include "graphics/idrawing_surface.h"
#include "clock_stm32f1xx.h"
#include "dma_stm32f1xx.h"
#include "libpekin.h"

#include "lvgl_driver/lvgl_tft_driver.h"
#include "devices/peripherals.h"
#include "error_handler.h"

using namespace Libp;
using namespace LibpStm32;

static inline constexpr uint32_t fsmc_dat_addr = 0x60200000;
static inline constexpr uint32_t fsmc_cmd_addr = 0x60000000;

static LibpStm32::Dma::DmaDevice<1> dma;
static LcdBusFsmc<fsmc_dat_addr, fsmc_cmd_addr> fsmcBus(&dma);
static MipiDcsTftDriver tft(fsmcBus, Ili9481::device_width, Ili9481::device_height);

static uint16_t readDeviceId(IBasicBus16& lcd_bus)
{
    uint8_t result[5]; // TODO: should be 6?
    lcd_bus.write8Cmd(Ili9481::Cmd::device_read_code);
    for (uint8_t i = 0; i < 5; i++) {
        result[i] = lcd_bus.read8();
    }
    return result[3] << 8 | result[4];
}

IDrawingSurface<uint16_t>& initTftDisplay()
{
    Clk::enable<Clk::Ahb::fsmc>();

    // Set DBI type B 16-bit mode
    // -110 (8-bit 8080 mode)
    // -010 (16-bit 8080 mode)
    Pins::tft_im0.clear();
    Pins::tft_im1.set();
    Pins::tft_im2.clear();
    delayMs(100);

    // Reset display

    Pins::tft_reset.set();
    delayMs(1);
    Pins::tft_reset.clear();
    delayMs(1);
    Pins::tft_reset.set();
    delayMs(1);

    // Setup DMA

    LibpStm32::Clk::enable<LibpStm32::Clk::Ahb::dma1>();

    using namespace LibpStm32::Dma;
    constexpr uint32_t cfg =
            CfgBuilder::create(CfgBuilder::Mode::mem_to_mem, CfgBuilder::IncMode::periph_only)
                    .bitWidth(CfgBuilder::MemSize::bits16, CfgBuilder::PeriphSize::bits16)
                    .enableInts(true, false, true)
                    .build();

    dma.configure(cfg);

    NVIC_SetPriority(DMA1_Channel1_IRQn, 0);
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);

    // Initialize display

    uint16_t id = readDeviceId(fsmcBus);
    if (id != Ili9481::device_id) {
        getErrHndlr().halt(ErrCode::display, "TFT ID: 0x%04x", (int) id);
    }
    tft.init(
            Ili9481::init_cmd_seq,
            sizeof(Ili9481::init_cmd_seq),
            Orientation::landscape_rev, true);

    return tft;
}

IDrawingSurface<uint16_t>& getTftDisplay()
{
    return tft;
}

extern "C"
void DMA1_Channel1_IRQHandler(void)
{
    // TODO: check if int is enabled?
    if (dma.intErr()) {
        dma.clearIntErr();
        getErrHndlr().halt(ErrCode::dma);
    }
    else if (dma.intComplete()) {
        dma.clearIntComplete();
        lvgl_tftdriver_set_dma_complete();
    }
    else if (dma.intHalfComplete()) {
        dma.clearIntHalfComplete();
    }
}
