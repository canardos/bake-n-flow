#include "speaker.h"
#include "audio/lp_dac_audio.h"

#include "devices/peripherals.h"
#include "error_handler.h"

#include "audio_samples/audio_click.h"
#include "audio_samples/audio_complete.h"
#include "audio_samples/audio_alert.h"

#include "lp_clock_stm32f1xx.h"
#include "lp_dac_stm32f1xx.h"
#include "lp_dma_stm32f1xx.h"
#include "lp_pins_stm32f1xx.h"
#include "lp_timer_stm32f1xx.h"


static libp_stm32::tmr::BasicTimer<TIM6_BASE> timer_;
static libp_stm32::dac::DacDevice dac_;

void initSpeaker()
{
    using namespace libp_stm32;

    // Setup timer

    clk::enable<clk::Apb1::tim6>();
    timer_.initBasic(false, false);

    NVIC_SetPriority(TIM6_IRQn, 0);
    NVIC_EnableIRQ(TIM6_IRQn);

    // Setup DAC / DMA

    clk::enable<libp_stm32::clk::Ahb::dma2>();
    clk::enable<libp_stm32::clk::Apb1::dac>();

    NVIC_SetPriority(DMA2_Channel3_IRQn, 0);
    NVIC_EnableIRQ(DMA2_Channel3_IRQn);

    constexpr uint32_t cfg =
            dma::CfgBuilder::create(dma::CfgBuilder::Mode::mem_to_periph, dma::CfgBuilder::IncMode::mem_only)
                    .bitWidth(dma::CfgBuilder::MemSize::bits8, dma::CfgBuilder::PeriphSize::bits8)
                    .enableInts(true, false, true)
                    .build();
    dma::DmaDevice<3, DMA2_BASE>::configure(cfg);

    constexpr uint32_t dac_cr_reg = []() {
        dac::Cfg cfg;
        cfg.ch1_trigger = dac::Cfg::Trigger::tim6_trgo;
        uint32_t reg = cfg.toCrRegVal();
        return reg;
    }();
    dac::DacDevice dac;
    dac.configure(dac_cr_reg);
}

void enableSpeaker(bool enable)
{
    // NCV2211 low enable
    pins::audio_en.set(!enable);
}

/// 8-bit only
static void playSample(const libp::AudioSample& sample)
{
    using namespace libp_stm32;

    timer_.enable();

    // Setup timer for sample rate
    uint32_t apb1_tim_clock = clk::getPClk1() * 2;
    timer_.setPeriod(apb1_tim_clock / sample.rate);

    // TODO: do we need to stop first?
    dac_.startDma<3, DMA2_BASE>(
            dac::Channel::ch1,
            const_cast<uint32_t*>(reinterpret_cast<const uint32_t*>(sample.data)),
            sample.len,
            dac::Format::r_align_8bit);
}

/// Last played sample (for use in repeats)
static const libp::AudioSample* volatile current_sample_ = nullptr;
static volatile uint8_t repeat_count_ = 0;

void playSound(Sound sound)
{
    switch (sound) {
    case Sound::click:
        repeat_count_ = 0;
        current_sample_ = &audiosample_keypress;
        break;
    case Sound::error:
        repeat_count_ = 0;
        current_sample_ = &audiosample_alert;
        break;
    case Sound::completed:
        repeat_count_ = 3;
        current_sample_ = &audiosample_complete;
        break;
    }
    playSample(*current_sample_);
}


// Called at end of DMA transfer started in playSound above.
//
extern "C"
void DMA2_Channel3_IRQHandler(void)
{
    static libp_stm32::dma::DmaDevice<3, DMA2_BASE> dma;

    // TODO: check if int is enabled?
    if (dma.intErr()) {
        dma.clearIntErr();
        getErrHndlr().halt(libp::ErrCode::dma, "DAC DMA error");
    }
    else if (dma.intComplete()) {
        dma.clearIntComplete();
        if (repeat_count_ > 0) {
            repeat_count_ = repeat_count_ - 1;
            playSample(*current_sample_);
        }
    }
    else if (dma.intHalfComplete()) {
        dma.clearIntHalfComplete();
    }
}
