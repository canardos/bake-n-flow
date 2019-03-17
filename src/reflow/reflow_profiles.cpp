#include <reflow/reflow_profiles.h>
#include <cstring>
#include <cstdio>

void ReflowProfiles::updateCount()
{
    for (uint8_t i = 0; i < max_profiles_; i++) {
        if (profiles_[i].name[0] == '\0') {
            num_profiles_ = i;
            break;
        }
    }
}


uint8_t ReflowProfiles::addProfile()
{
    if (num_profiles_ == max_profiles_)
        return 0;
    // Use built in profile as template
    memcpy(&profiles_[num_profiles_], &sn63pb37, sizeof(Profile));
    snprintf(profiles_[num_profiles_].name, max_name_len, "Profile %d", num_profiles_ + 1);
    return ++num_profiles_;
}


void ReflowProfiles::deleteProfile(uint8_t idx)
{
    // At least one profile is required
    if (idx >= num_profiles_ || num_profiles_ == 1)
        return;
    if (idx < (num_profiles_ - 1)) {
        size_t len = (num_profiles_ - idx - 1) * sizeof(Profile);
        memcpy( static_cast<void*>(&profiles_[idx]),
                static_cast<void*>(&profiles_[idx + 1]), len);
    }
    num_profiles_--;
}
