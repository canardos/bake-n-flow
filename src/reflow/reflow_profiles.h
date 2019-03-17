#ifndef SRC_REFLOW_REFLOW_PROFILES_H_
#define SRC_REFLOW_REFLOW_PROFILES_H_

#include <main.h>
#include <cstdint>

/**
 * Wrapper around array of reflow profiles to manage
 * addition/editing/deleting etc.
 *
 * All temperatures units are 0.1 degC
 */
class ReflowProfiles {
public:
    inline static constexpr uint8_t max_name_len = 15;
    /// Assumed starting temperature.
    inline static constexpr uint16_t start_temp = 500;
    /// End of reflow temperature (natural cooling thereafter).
    inline static constexpr uint16_t end_temp = 1000;
    /// Maximum number of profiles (fixed array size)
    inline static constexpr uint8_t max_profiles_ = 10;

    struct Profile {
        struct Stage {
            /// Duration in seconds
            uint16_t duration;
            /// Ending temperature in tenths of a degree celsius (e.g. 1234 = 123.4°C)
            uint16_t final_temp;
        };

        char name[max_name_len + 1];
        TempUnit units;
        /// First stage
        Stage preheat;
        /// Soak immediately after preheating
        Stage soak;
        /// After soak, ramp to max profile temp
        Stage reflow_ramp;
        /// Dwell at max temp after reflow ramp (duration in seconds)
        uint16_t reflow_dwell_duration;
        /// Duration in seconds to fall to `END_TEMP`
        uint16_t cool_duration;

        /// Return the total reflow profile duration in seconds
        uint16_t getTotalDuration() const
        {
            return preheat.duration + soak.duration + reflow_ramp.duration
                    + reflow_dwell_duration + cool_duration;
        }
        /// Return the final ramp temperature.
        uint16_t maxTemp() const
        {
            return reflow_ramp.final_temp;
        }

        /// Number of @p TempPoint objects to represent the full profile
        inline static constexpr uint8_t num_profile_points = 6;
        /// A specific temperature at a point in time
        struct TempPoint {
            uint16_t time_s;
            /// Temperature in 0.1°C (e.g. 1234 = 123.4°C)
            uint16_t temp;
        };

        /**
         * Convert the profile to a set of @p TempPoint 's where the time
         * is the elapsed time from the start of the profile.
         *
         * @param points array of points to receive the data
         */

        void covertToPoints(TempPoint (&points)[num_profile_points]) const {
            points[0] = { 0,
                          start_temp};
            points[1] = { preheat.duration,
                          preheat.final_temp};
            points[2] = { static_cast<uint16_t>(points[1].time_s + soak.duration),
                          soak.final_temp};
            points[3] = { static_cast<uint16_t>(points[2].time_s + reflow_ramp.duration),
                          reflow_ramp.final_temp};
            points[4] = { static_cast<uint16_t>(points[3].time_s + reflow_dwell_duration),
                          points[3].temp};
            points[5] = { static_cast<uint16_t>(points[4].time_s + cool_duration),
                          end_temp};
        }
    };

    inline static constexpr Profile sn63pb37 {
        "Sn63_Pb37",
        TempUnit::celsius,
        { 60, 1500},
        { 90, 1700},
        { 45, 2200},
        15,
        40 // -3deg/sec
    };
    inline static constexpr Profile pbfree {
        "Pb_Free",
        TempUnit::celsius,
        { 60, 1500},
        {120, 1800},
        { 45, 2500},
        15,
        42
    };

    /**
     *
     * @param profiles must be in sequence starting at index 0.
     *                 name of profile after last valid profile must be "".
     *                 Array reference will be kept and used.
     */
    ReflowProfiles(Profile (&profiles)[max_profiles_])
            : profiles_(profiles), active_profile_(0)
    {
        updateCount();
    }

    /// Set the number of profiles by counting the array.
    /// Call this function to update the count if the array is modified
    /// externally.
    void updateCount();

    uint8_t getNumProfiles() const
    {
        return num_profiles_;
    }

    /// If @p idx is invalid, the first profile will be returned
    Profile& getProfile(uint8_t idx) const
    {
        return idx >= max_profiles_
                ? profiles_[0]
                : profiles_[idx];
    }

    Profile& getActiveProfile() const
    {
        // There is always at least one profile
        return profiles_[active_profile_];
    }

    void setActiveProfile(uint8_t idx)
    {
        if (idx < num_profiles_)
            active_profile_ = idx;
    }

    /**
     * Noop if maximum number of profiles already present
     *
     * @return total number of profiles after the addition
     */
    uint8_t addProfile();

    /// Noop if @p idx is invalid or only 1 profile remains
    void deleteProfile(uint8_t idx);

private:
    Profile (&profiles_)[max_profiles_];
    uint8_t num_profiles_;
    uint8_t active_profile_;
};

#endif /* SRC_REFLOW_REFLOW_PROFILES_H_ */
