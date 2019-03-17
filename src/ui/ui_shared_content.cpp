#include <cstdint>
#include <cstdio>
#include <cstring>

static char elapsed_time_text_[] = { "Elapsed:\n00:00:00" };
static char remaining_time_text_[] = { "Remaining:\n00:00:00" };

/// Convert number of seconds to [HH:]MM:SS string
/// len(buf)-offs must be >= include_hours ? sizeof("00:00:00") : sizeof("00:00")
static void secsToTimeString(uint16_t time_s, char* buf, uint8_t offs, bool include_hours)
{
    if (include_hours) {
        snprintf(&buf[offs], sizeof("00:00:00"), "%02d:%02d:%02d",
                (int) time_s / 3600,
                (int) time_s % 3600 / 60,
                (int) time_s % 60);
    }
    else {
        snprintf(&buf[offs], sizeof("00:00"), "%02d:%02d",
                (int) time_s % 3600 / 60,
                (int) time_s % 60);
    }
}


void updateTimeStrings(uint16_t elapsed_time_s, uint16_t total_time_s, bool include_hours)
{
    static constexpr uint8_t elapsed_offs = strlen("Elapsed:\n");
    secsToTimeString(elapsed_time_s, elapsed_time_text_, elapsed_offs, include_hours);
    static constexpr uint8_t remain_offs = strlen("Remaining:\n");
    secsToTimeString(total_time_s - elapsed_time_s, remaining_time_text_, remain_offs, include_hours);
}


char* getElapsedTimeString()
{
    return elapsed_time_text_;
}

char* getRemainingTimeString()
{
    return remaining_time_text_;
}
