#include <misc_math.h>
#include <oven/oven_operation.h>

bool OvenOperation::startReflow(ReflowProfiles::Profile& profile, OperationCompleteCb reflow_complete_cb)
{
    if (state_ != State::stopped)
        return false;
    if (oven_.getTemp() >= ReflowProfiles::start_temp)
        return false;

    operation_complete_cb_ = reflow_complete_cb;
    reflow_op_.init(profile);

    // Run at 100% power until profile start temperature
    state_ = State::reflow_warming;
    oven_.setDoorOpening(false);
    oven_.setPowerLevel(100);
    return true;
}


bool OvenOperation::startBake(uint16_t time_s, uint16_t temp, OperationCompleteCb bake_complete_cb)
{
    if (state_ != State::stopped)
        return false;

    if (oven_.getTemp() >= temp)
        return false;

    if (time_s > max_bake_duration_s || temp > max_bake_temp)
        return false;

    operation_complete_cb_ = bake_complete_cb;
    bake_temp_ = temp;
    bake_duration_s_ = time_s;
    start_time_ms_ = get_millis_func_();
    state_ = State::baking;

    oven_.setDoorOpening(false);
    oven_.setPowerLevel(bake_start_power);
    pid_algo_.init(oven_.getTemp(), pid_sampling_period_ms / 1000, Libp::PidAlgo::Mode::normal);

    return true;
}


bool OvenOperation::startManualPower(uint8_t power_level)
{
    if (state_ != State::stopped && state_ != State::manual_pwr)
        return false;

    if (power_level == 0) {
        stop();
    }
    else {
        start_time_ms_ = get_millis_func_();
        state_ = State::manual_pwr;
        oven_.setPowerLevel(power_level);
    }
    return true;
}

bool OvenOperation::startManualTemp(uint16_t temp)
{
    if (state_ != State::stopped && state_ != State::manual_temp)
        return false;

    bake_temp_ = temp;
    bake_duration_s_ = 3600 * 10; // TODO:
    start_time_ms_ = get_millis_func_();
    state_ = State::manual_temp;

    oven_.setPowerLevel(bake_start_power);
    pid_algo_.init(oven_.getTemp(), pid_sampling_period_ms / 1000, Libp::PidAlgo::Mode::normal);

    return true;
}


bool OvenOperation::isErrorCondition()
{
    uint16_t oven_temp = oven_.getTemp();
    uint32_t now_ms = get_millis_func_();
    uint16_t elapsed_time_s = (now_ms - start_time_ms_) / 1000;

    // oven too hot/cold
    if (oven_temp < min_oven_temp || oven_temp > max_oven_temp)
        return true;

    switch (state_) {

    // baking too long
    case State::baking:
        if (elapsed_time_s > (max_bake_duration_s + 30) )
            return true;
        break;

    // reflowing too long
    case State::reflow_tracking:
    case State::reflow_warming:
        if (elapsed_time_s > (max_reflow_duration_s + 30) )
            return true;
        break;
    case State::stopped:
        // TODO: set max manual timer
    case State::manual_pwr:
    case State::manual_temp:
    case State::reflow_cooling:
        break;
    }

    // timer overflow
    if (now_ms < start_time_ms_)
        return true;

    return false;
}

static constexpr uint16_t slope_look_ahead_reflow_s = 10;
static constexpr uint16_t slope_look_ahead_bake_s = 45;

bool OvenOperation::runPidUpdate(uint16_t oven_temp)
{
    if (state_ != State::baking && state_ != State::manual_temp && state_ != State::reflow_tracking) {
        // ERROR
        // TODO; shutdown
        return false;
    }

    uint16_t elapsed_time_s = getElapsedTime();
    int16_t target_slope;

    target_slope = (state_ == State::reflow_tracking)
    		? reflow_op_.getTargetSlope(oven_temp, elapsed_time_s, slope_look_ahead_reflow_s)
			: (bake_temp_ - oven_temp) / slope_look_ahead_bake_s;

    float new_power_lvl;
    bool pid_has_update = pid_algo_.compute(target_slope, oven_temp, &new_power_lvl);

    if (pid_has_update) {
        oven_.setPowerLevel(new_power_lvl);
    }
    return true;
}


bool OvenOperation::process()
{
    if (state_ == State::stopped)
        return true;

    if (isErrorCondition()) {
        stop();
        // TODO: we should alert here
        return false;
    }
    uint16_t oven_temp = oven_.getTemp();
    uint16_t elapsed_time_s = getElapsedTime();

    switch (state_) {
    case State::reflow_warming:
        if (oven_temp < ReflowProfiles::start_temp)
            // Wait until we hit profile start temperature
            return true;
        start_time_ms_ = get_millis_func_();
        pid_algo_.init(oven_temp, pid_sampling_period_ms / 1000, Libp::PidAlgo::Mode::gradient);
        state_ = State::reflow_tracking;
        break;
    case State::reflow_tracking:
        if (reflow_op_.isFinished(elapsed_time_s)) {
            oven_.setPowerLevel(0);
            state_ = State::reflow_cooling;
        }
        else {
            runPidUpdate(oven_temp);
        }
        break;
    case State::reflow_cooling:
        if (oven_temp < ReflowProfiles::end_temp) {
            operation_complete_cb_();
            stop();
            return false;
        }
        // TODO: open/close door to control cooling rate
        break;
    case State::baking:
    case State::manual_temp:
        if (elapsed_time_s >= bake_duration_s_) {
            operation_complete_cb_();
            stop();
            return false;
        }
        else return runPidUpdate(oven_temp);
    // suppress unhandled case warning to ensure we catch future ones
    case State::stopped:
    case State::manual_pwr:
        break;
    }
    return true;
}


void OvenOperation::stop()
{
    oven_.setPowerLevel(0);
    oven_.setDoorOpening(true);
    state_ = State::stopped;
}
