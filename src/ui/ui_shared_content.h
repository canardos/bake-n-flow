/**
 * Functions to get/set strings that are shared across several UI pages.
 */
#ifndef UI_UI_SHARED_CONTENT_H_
#define UI_UI_SHARED_CONTENT_H_

#include <cstdint>

/**
 * Update the shared elapsed/remaining time strings used on bake and reflow
 * pages.
 */
void updateTimeStrings(uint16_t elapsed_time_s, uint16_t total_time_s, bool include_hours);

/**
 * Retrieve the elapsed time string used on bake and reflow pages.
 *
 * Use @p updateTimeStrings to update.
 *
 * @return pointer to null terminated static char array
 */
char* getElapsedTimeString();

/**
 * Retrieve the remaining time string used on bake and reflow pages.
 *
 * Use @p updateTimeStrings to update.
 *
 * @return pointer to null terminated static char array
 */
char* getRemainingTimeString();

#endif /* UI_UI_SHARED_CONTENT_H_ */
