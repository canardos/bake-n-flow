#ifndef OVEN_SIM_H_
#define OVEN_SIM_H_

#include <cstdint>
#include <algorithm>
#include <cstring>
#include "libpekin.h"

// Cooling (0.1deg C)
// dT/ds = m*temp + b

static inline constexpr float cool_m = -.002115;
static inline constexpr float cool_b =  .6675;

// Heating (0.1deg C)
// dT/ds = m*temp + b

static inline constexpr float heat_m = -.0015;
static inline constexpr float heat_b = 25;
static inline constexpr float max_heat_rate = 25;

// Power adj. for heating
// dT/ds = dT/ds(100%) * pwr/100 * pwr_adj
// factor = m*pwr + b

static inline constexpr float pwr_m =  .002;
static inline constexpr float pwr_b =  .80;

inline
float calc_dt_ds(float temp_diff, float power, uint16_t millis)
{
    float cool_dtds = cool_m * temp_diff + cool_b;
    float heat_dtds = heat_m * temp_diff + heat_b;
    float pwr_adj = power/100 * (power*pwr_m + pwr_b);
    heat_dtds *= pwr_adj;
    heat_dtds = std::min(heat_dtds, max_heat_rate);
    return (cool_dtds + heat_dtds) * millis / 1000;
}

static inline constexpr uint16_t max_avg_len = 200;
static float avg_dat[max_avg_len] = { 0 };
static float movingAvg(double new_data, uint16_t n)
{
    memmove(&avg_dat[1], &avg_dat[0], (max_avg_len - 1) * sizeof(uint16_t));
    avg_dat[0] = new_data;

    float avg_sum = 0;
    int16_t i = n - 1;
    while (i >= 0)
        avg_sum += avg_dat[i--];

    return avg_sum / n;
}

static float ambient_temp = 270;
static float mock_temp = ambient_temp;
static uint64_t last_instant = 0;
static void updateMockTemp(uint8_t power_lvl, uint16_t elapsed_ms)
{
    uint16_t avg_len = std::max(15., (double)(1700 - mock_temp) / 20);
    float lagged_power = movingAvg(power_lvl, avg_len); // 150
    float dt_ds = calc_dt_ds(mock_temp - ambient_temp, lagged_power, elapsed_ms);
    mock_temp += dt_ds;
}

static float getMockTemp(uint8_t power_lvl)
{
    if (last_instant == 0)
        last_instant = Libp::getMillis();
    uint64_t now = Libp::getMillis();
    uint16_t elapsed_ms = now - last_instant;
    if (elapsed_ms >= 1000) {
        updateMockTemp(power_lvl, elapsed_ms);
        last_instant = now;
    }
    return mock_temp;
}

#endif /* OVEN_SIM_H_ */
