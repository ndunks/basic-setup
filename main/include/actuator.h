#ifndef APP_ACTUATOR_H
#define APP_ACTUATOR_H

static uint8_t actuator_value = 0x00;

void actuator_setup(unsigned char initial_value);
void actuator_update(unsigned char value);
void actuator_register_command();

#endif
