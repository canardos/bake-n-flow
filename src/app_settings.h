#ifndef APP_SETTINGS_H_
#define APP_SETTINGS_H_

#include <cstdint>
#include <cstring>
#include <flash/eeprom_stm32f1xx.h>
#include "reflow/reflow_profiles.h"
#include "touch/resistive_touch.h"
#include "misc_math.h"

/**
 * Persistent application settings container.
 */
class AppSettings {
public:
    struct PidParams {
        uint16_t kp;
        uint16_t ki;
        uint16_t kd;
    };
    struct Data {
        TempUnit units;
        /// 1->100
        uint8_t brightness;
        bool mute;
        Libp::ResistiveTouch::CalibrationMatrix touch_calib_mtx;
        ReflowProfiles::Profile profiles[ReflowProfiles::max_profiles_];
        PidParams pid_params;

        /**
         * Convert temperature to the currently configured units.
         *
         * @param temp in tenths of a degree
         * @param src_unit
         * @return temperature in tenths of a degree
         */
        int16_t unitsToCurrentUnits(int16_t temp, TempUnit src_unit)
        {
            return convertUnits(temp, src_unit, units);
        }

        /**
         * Convert temperature units
         *
         * @param temp in tenths of a degree
         * @param src_unit
         * @param dst_unit
         * @return temperature in tenths of a degree
         */
        static int16_t convertUnits(int16_t temp, TempUnit src_unit, TempUnit dst_unit)
        {
            if (src_unit == dst_unit)
                return temp;
            return src_unit == TempUnit::celsius
                    ? Libp::celcius_to_fahrenheit(temp)
                    : Libp::fahrenheit_to_celcius(temp);
        }
    };

    AppSettings() : reflow_profiles_(data_.profiles)
    {
        // Load settings/profiles from flash
        const Data* flash_dat = eeprom.get();
        if (flash_dat != nullptr) {
            memcpy(&data_, flash_dat, sizeof(Data));
            reflow_profiles_.updateCount();
        }
        // This is the first run, store defaults
        else {
            eeprom.write(&data_);
        }
    }

    /**
     * Return reference to application settings.
     */
    Data& settings()
    {
        return data_;
    }

    /**
     * Return reference to reflow profiles.
     */
    ReflowProfiles& profiles()
    {
        return reflow_profiles_;
    }

    bool writeToFlash() {
        return eeprom.write(&data_);
    }

    static AppSettings& get()
    {
        static AppSettings instance;
        return instance;
    }

private:
    static constexpr uint32_t flash_page_size = FLASH_PAGE_SIZE;
#ifndef STM32F103xE
    static_assert(false, "Update flash_size below if MCU changed")
#endif
    static constexpr uint32_t flash_size = 384*1024;
    static constexpr uint32_t flash_end_addr = FLASH_BASE + flash_size - 1;
    static constexpr uint32_t eeprom_base_addr = flash_end_addr - (flash_page_size * 2) + 1;
    static constexpr uint16_t magic_signature = 0x1245;

    LibpStm32::Eeprom<Data, eeprom_base_addr, magic_signature> eeprom;
    Data data_ = []() {

        // -- App defaults --

        AppSettings::Data data { };
        data.units = TempUnit::celsius;
        data.brightness = 75;
        data.mute = false;
        data.touch_calib_mtx = { 0 };
        data.profiles[0] = ReflowProfiles::sn63pb37;
        data.profiles[1] = ReflowProfiles::pbfree;
        data.pid_params = { 40, 10, 5 }; // 500, 50, 30000

        return data;
    }();
    ReflowProfiles reflow_profiles_;
};

inline
AppSettings::Data& getSettings()
{
    return AppSettings::get().settings();
}

inline
ReflowProfiles& getReflowProfiles()
{
    return AppSettings::get().profiles();
}

#endif /* APP_SETTINGS_H_ */
