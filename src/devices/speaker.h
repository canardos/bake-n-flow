/**
 * Audio output functions.
 *
 * Audio output is generated by MCU DAC and sent to external amplifier/speaker.
 */
#ifndef SRC_UI_SPEAKER_H_
#define SRC_UI_SPEAKER_H_

#include <cstdint>

/**
 * Initialize speaker hardware.
 */
void initSpeaker();

/**
 * Enable or mute speaker output.
 *
 * @param enable
 */
void enableSpeaker(bool enable);

enum class Sound : uint8_t {
    click, error, completed
};

void playSound(Sound sound);

#endif /* SRC_UI_SPEAKER_H_ */