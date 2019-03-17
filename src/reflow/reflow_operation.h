#ifndef SRC_OVEN_REFLOW_OPERATION_H_
#define SRC_OVEN_REFLOW_OPERATION_H_

#include <cstdint>
#include "misc_math.h"
#include "reflow/reflow_profiles.h"

/**
 * State machine to manage execution of a reflow operation.
 */
class ReflowOperation {
public:
    void init(ReflowProfiles::Profile& profile)
    {
        profile.covertToPoints(profile_temps_);
        profile_ = &profile;
        dwelling_ = false;
    }


    bool isFinished(uint16_t time_s) {
        return dwelling_ && ((time_s - dwell_start_time_s) >= profile_->reflow_dwell_duration);
    }

    uint16_t getTargetSlope(uint16_t oven_temp, uint16_t time_s, uint16_t look_ahead_s)
    {
        static constexpr uint16_t dwell_temp_err_mgn = 30;

        if (!dwelling_ && oven_temp >= (profile_->maxTemp() - dwell_temp_err_mgn)) {
            dwell_start_time_s = time_s;
            dwelling_ = true;
        }

        if (dwelling_) {
            return (profile_->maxTemp() - oven_temp) / look_ahead_s;
        }
        else {
            uint16_t target_temp_t_plus_look_ahead = getReflowTargetTemp(time_s + look_ahead_s);
            return target_temp_t_plus_look_ahead == 0
                    ? 0
                    : (target_temp_t_plus_look_ahead - oven_temp) / look_ahead_s;
        }
    }


private:
    uint16_t dwell_start_time_s = 0;
    bool dwelling_ = false;
    ReflowProfiles::Profile::TempPoint profile_temps_[ReflowProfiles::Profile::num_profile_points];
    ReflowProfiles::Profile* profile_ = nullptr;

    /**
     * Return the target reflow profile temperature at a specific point in
     * profile time.
     *
     * @param time_s elapsed profile time in seconds
     *
     * @return target temperature at time \p time_s in 0.1°C.
     */
    uint16_t getReflowTargetTemp(uint16_t time_s)
    {
        if (time_s == 0)
            return ReflowProfiles::start_temp;

        for (uint8_t i = 1; i < ReflowProfiles::Profile::num_profile_points; i++) {
            if (profile_temps_[i].time_s >= time_s) {
                return Libp::linearInterp(
                        profile_temps_[i - 1].time_s,
                        profile_temps_[i - 1].temp,
                        profile_temps_[i].time_s,
                        profile_temps_[i].temp,
                        time_s);
            }
        }
        // time_s is > end of reflow profile
        return 0;
    }

};

#endif /* SRC_OVEN_REFLOW_OPERATION_H_ */
