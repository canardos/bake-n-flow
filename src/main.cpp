#include "app_settings.h"
#include "devices/tft.h"
#include "devices/touch.h"
#include "devices/tft_led.h"
#include "devices/speaker.h"
#include "devices/door_servo.h"
#include "devices/oven_ssr.h"
#include "devices/thermocouple.h"
#include "devices/peripherals.h"

#include "lvgl_driver/lvgl_driver.h"
#include "lvgl/lvgl.h"
#include "libpekin.h"
#include "touch/touch_calibrate.h"
#include <graphics/idrawing_surface.h>


using namespace Libp;

/// Calibrate touch interactively and save calibration to flash.
template <typename T>
static void calibrateTouchscreen(ResistiveTouch::Screen& touch_screen, IDrawingSurface<T>& display)
{
    AppSettings app_settings = AppSettings::get();
    if (!ResistiveTouch::calibrateTouch(display, touch_screen)) {
        getErrHndlr().halt(ErrCode::general, "Touch calibration failed");
    }
    app_settings.settings().touch_calib_mtx = touch_screen.getCalibration();
    app_settings.writeToFlash();
}

static void initClk()
{
    using namespace LibpStm32;
    // 72 MHz
    Clk::setSysClk(Clk::SysClkSrc::ext_high_speed_osc, 2, Clk::PllSrc::hse, 2);
    Clk::setSysClk(Clk::SysClkSrc::pll, 2, Clk::PllSrc::hse, 9);
    Clk::setHClk(Clk::AhbPrescaler::div1);
    Clk::setPeripheralClk(Clk::ApbPrescaler::div2, Clk::ApbPrescaler::div1, Clk::AdcPrescaler::div6);
}

static void initDevices()
{
    initPeripherals();
    initErrHndlr();
    initSpeaker();
    initOvenSsr();
    initDoorServo();
    initThermocouple();
}

void runMainProgLoop();

int main()
{
    initClk();
    libpekinInitTimers();
    initDevices();

    Libp::IDrawingSurface<uint16_t>& tft_display = initTftDisplay();
    ResistiveTouch::Screen& touch_screen = initTouchscreen();

    // If screen held down on startup, set
    // brightness high and run touch calibration.

    static constexpr uint16_t touch_hold_time_ms = 2000;
    uint16_t i = 0;
    while (touch_screen.isTouched()) {
        if (i++ == touch_hold_time_ms / 10) {
            initTftLed(75);
            calibrateTouchscreen(touch_screen, tft_display);
            break;
        }
        delayMs(10);
    }

    AppSettings::Data& settings = AppSettings::get().settings();
    initTftLed(settings.brightness);

    enableSpeaker(!settings.mute);

    // Calibrate touch if we have no stored setting

    if (!settings.touch_calib_mtx.valid())
        calibrateTouchscreen(touch_screen, tft_display);
    else
        touch_screen.updateCalibration(settings.touch_calib_mtx);

    lv_init();
    initLvglHalDrivers(&touch_screen, &tft_display);

    runMainProgLoop();
}
