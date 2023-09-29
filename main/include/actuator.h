#ifndef APP_ACTUATOR_H
#define APP_ACTUATOR_H

void actuator_setup(unsigned char initial_value);
void actuator_update(unsigned char value);
void actuator_register_command();

#endif
