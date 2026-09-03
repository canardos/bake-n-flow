#include "display/lp_ili9481_cmd_defs.h"
#include "display/lp_mipi_dcs_tft_driver.h"
#include "graphics/lp_idrawing_surface.h"
#include "libpekin.h"

#include "lvgl_driver/lvgl_tft_driver.h"
#include "devices/peripherals.h"
#include "error_handler.h"

#include "display/lp_lcd_bus_fsmc_stm32f1xx.h"
#include "lp_clock_stm32f1xx.h"
#include "lp_dma_stm32f1xx.h"

using namespace libp;
using namespace libp_stm32;

static inline constexpr uint32_t fsmc_dat_addr = 0x60200000;
static inline constexpr uint32_t fsmc_cmd_addr = 0x60000000;

static libp_stm32::dma::DmaDevice<1> dmadev;
static LcdBusFsmc<fsmc_dat_addr, fsmc_cmd_addr> fsmcBus(&dmadev);
static MipiDcsTftDriver tft(fsmcBus, ili9481::device_width, ili9481::device_height);

static uint16_t readDeviceId(IBasicBus16& lcd_bus)
{
    uint8_t result[5]; // TODO: should be 6?
    lcd_bus.write8Cmd(ili9481::cmd::device_read_code);
    for (uint8_t i = 0; i < 5; i++) {
        result[i] = lcd_bus.read8();
    }
    return result[3] << 8 | result[4];
}

IDrawingSurface<uint16_t>& initTftDisplay()
{
    clk::enable<clk::Ahb::fsmc>();

    // Set DBI type B 16-bit mode
    // -110 (8-bit 8080 mode)
    // -010 (16-bit 8080 mode)
    pins::tft_im0.clear();
    pins::tft_im1.set();
    pins::tft_im2.clear();
    delayMs(100);

    // Reset display

    pins::tft_reset.set();
    delayMs(1);
    pins::tft_reset.clear();
    delayMs(1);
    pins::tft_reset.set();
    delayMs(1);

    // Setup DMA

    libp_stm32::clk::enable<libp_stm32::clk::Ahb::dma1>();

    using namespace libp_stm32::dma;
    constexpr uint32_t cfg =
            CfgBuilder::create(CfgBuilder::Mode::mem_to_mem, CfgBuilder::IncMode::periph_only)
                    .bitWidth(CfgBuilder::MemSize::bits16, CfgBuilder::PeriphSize::bits16)
                    .enableInts(true, false, true)
                    .build();

    dmadev.configure(cfg);

    NVIC_SetPriority(DMA1_Channel1_IRQn, 0);
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);

    // Initialize display

    uint16_t id = readDeviceId(fsmcBus);
    if (id != ili9481::device_id) {
        getErrHndlr().halt(ErrCode::dma, "TFT ID: 0x%04x", (int) id);
    }
    tft.init(
            ili9481::init_cmd_seq,
            sizeof(ili9481::init_cmd_seq),
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
    if (dmadev.intErr()) {
        dmadev.clearIntErr();
        getErrHndlr().halt(ErrCode::dma);
    }
    else if (dmadev.intComplete()) {
        dmadev.clearIntComplete();
        lvgl_tftdriver_set_dma_complete();
    }
    else if (dmadev.intHalfComplete()) {
        dmadev.clearIntHalfComplete();
    }
}
