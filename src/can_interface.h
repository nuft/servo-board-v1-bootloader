#ifndef CAN_INTERFACE_H
#define CAN_INTERFACE_H

#include <stdint.h>

int can_interface_init(void);

int can_interface_read_message(uint32_t *message_id, uint8_t *message);

void can_interface_write_message(uint32_t message_id, uint8_t *message, uint8_t length);

#endif // CAN_INTERFACE_H
