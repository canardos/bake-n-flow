#ifndef REFLOW_REFLOW_OPERATION_H_
#define REFLOW_REFLOW_OPERATION_H_

#include <oven/oven_hardware.h>
#include <pid/pid_algo.h>
#include <cstdint>
#include "reflow/reflow_profiles.h"
#include "reflow/reflow_operation.h"
#include "libpekin.h"

/**
 * State machine to manage execution of reflow/baking operations.
 */
class OvenOperation {

public:
    using GetMillisFunc = uint64_t (*)();
    using OperationCompleteCb = void (*)();

    enum class State : uint8_t {
        stopped,         /**< Stopped */
        baking,          /**< Baking at fixed temp for X mins. */
        manual_pwr,      /**< Fixed power percentage forever */
        manual_temp,     /**< Fixed temp forever */
        reflow_warming,  /**< Running at 100% pwr until reflow start temp. */
        reflow_tracking, /**< Following reflow profile */
        reflow_cooling   /**< Cooling 0% pwr with door after reflow */
    };

    // 0.1°C units
    inline static constexpr uint16_t min_bake_temp = 500;
    inline static constexpr uint16_t max_bake_temp = 1300;
    inline static constexpr uint16_t max_bake_duration_s = 60 * 60 * 10;
    inline static constexpr uint16_t max_reflow_duration_s = 60 * 12;

    OvenOperation(OvenHardware& oven, Libp::PidAlgo& pid_algo, GetMillisFunc get_millis_func)
            : oven_(oven), pid_algo_(pid_algo), get_millis_func_(get_millis_func)
    { };

    /**
     * Begin reflow operation. Once started, \p process MUST be called regularly.
     *
     * @param profile
     * @param reflow_complete_cb function to call on successful completion (not
     *                           called if stopped early). May be null.
     *
     * @return @arg true if profile is started successfully.
     *         @arg false if reflow/bake already running or oven too hot.
     */
    bool startReflow(ReflowProfiles::Profile& profile, OperationCompleteCb reflow_complete_cb);

    /**
     * Begin bake operation. Once started, \p process MUST be called regularly.
     *
     * @param time_s number of seconds to bake for. Must not exceed @p
     *               OvenOperation::max_bake_duration_s.
     * @param temp temperature in tenths of a degrees Celcius (e.g. 1234 =
     *             123.4°C). Must be <= @p OvenOperation::max_bake_temp.
     * @param bake_complete_cb function to call on successful completion (not
     *                         called if stopped early). May be null.
     *
     * @return @arg true if bake operation is started successfully.
     *         @arg false if reflow/bake already running, oven is too hot, or
     *                    invalid parameters are provided.
     */
    bool startBake(uint16_t time_s, uint16_t temp, OperationCompleteCb bake_complete_cb);

    bool startManualPower(uint8_t power_level);
    bool startManualTemp(uint16_t temp);

    /**
     * Update running state and oven output for a reflow/bake operation. Must
     * be called regularly (< pid_sampling_period) when an operation is
     * running.
     *
     * @return @arg true if reflow/bake is still running.
     *         @arg false if not reflowing or baking.
     */
    bool process();

    /**
     * Stop reflow/bake operation. Oven power will be turned off and door will
     * be opened.
     */
    void stop();

    State getState() { return state_; }

    uint16_t getElapsedTime()
    {
        return state_ == State::stopped
                ? 0 : (get_millis_func_() - start_time_ms_) / 1000;
    }

private:
    inline static constexpr uint16_t pid_sampling_period_ms = 1000;
    inline static constexpr uint8_t bake_start_power = 50;
    /// Stop all operations if we exceed this temperature
    inline static constexpr uint16_t max_oven_temp = 2800; // units = 0.1C
    /// At less than this value we'll assume a thermocouple malfunction
    inline static constexpr uint16_t min_oven_temp = 50; // units = 0.1C

    OvenHardware& oven_;
    Libp::PidAlgo& pid_algo_;
    const GetMillisFunc get_millis_func_;

    State state_ = State::stopped;
    /// Start time for reflow/bake/manual
    uint32_t start_time_ms_ = 0;
    uint16_t bake_duration_s_ = 0;
    uint16_t bake_temp_ = 0;
    ReflowOperation reflow_op_;
    OperationCompleteCb operation_complete_cb_ = nullptr;

    bool runPidUpdate(uint16_t oven_temp);
    /**
     * @return true if an error is detected and the oven should be turned off.
     */
    bool isErrorCondition();
};

#endif /* REFLOW_REFLOW_OPERATION_H_ */
