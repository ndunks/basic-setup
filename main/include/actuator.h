#ifndef APP_ACTUATOR_H
#define APP_ACTUATOR_H

/**
 * Current actuator state
 */
// extern unsigned char actuator_value;

void actuator_setup(unsigned char initial_value);

#if CONFIG_APP_ESP01_SUPPORT_LC // Serial
void actuator_update(int switch_id, int value);
#else
void actuator_update(unsigned char value);
#endif

#endif
