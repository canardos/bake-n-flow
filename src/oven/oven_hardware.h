#ifndef HARDWARE_OVEN_HARDWARE_H_
#define HARDWARE_OVEN_HARDWARE_H_

#include <cstdint>
#include "devices/oven_ssr.h"
#include "devices/thermocouple.h"

#define MOCK_OVEN 0
#if MOCK_OVEN
#include "oven_sim.h"
#endif

/**
 * Interface to the external oven hardware.
 */
class OvenHardware {
public:

    OvenHardware() { };

    ~OvenHardware() { }

    /**
     * Set the oven power level percentage.
     *
     * Set to 0 to turn oven off.
     *
     * @param percentage valid range is 0 -> 100.
     */
    void setPowerLevel(uint8_t percentage)
    {
    #if MOCK_OVEN
        power_lvl_ = percentage;
        power_on_ = percentage > 0;
    #else
        setOvenSsr(percentage);
    #endif
    }

    /**
     * Open/close the oven door.
     *
     * @param opening amount 0->100 where 0=closed and 100=open.
     */
    void setDoorOpening(uint8_t opening)
    {
        door_opening_ = opening;
    }

    /**
     * Read the current oven temperature.
     *
     * @return temperature in tenths of a degrees Celcius (e.g. 1234 = 123.4°C)
     */
    uint16_t getTemp()
    {
    #if MOCK_OVEN
        return getMockTemp(power_lvl_);
    #else
        return readTemp();
    #endif
    }

#if MOCK_OVEN
    uint8_t power_lvl_ = 0;
    bool power_on_ = false;
    bool getPowerOn()       { return power_on_;  }
    uint8_t getPowerLevel() { return power_lvl_; }
#else
    bool getPowerOn()       { return getOvenSsr() > 0; }
    uint8_t getPowerLevel() { return getOvenSsr();     }
#endif
    bool getDoorOpening()   { return door_opening_;    }

private:
    uint8_t door_opening_;
};

#endif /* HARDWARE_OVEN_HARDWARE_H_ */
