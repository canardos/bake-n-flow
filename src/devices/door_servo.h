/**
 * Door open/close servo hardware functions.
 */
#ifndef SRC_DEVICES_DOOR_SERVO_H_
#define SRC_DEVICES_DOOR_SERVO_H_

#include <cstdint>

void initDoorServo();

void setDoorServo(uint16_t angle);




#endif /* SRC_DEVICES_DOOR_SERVO_H_ */
