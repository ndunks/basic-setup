#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "tcpip_adapter.h"
// Only support for 8
#define APP_SWITCH_COUNT 8
#define APP_SENSOR_COUNT 8
#define APP_NAME_MAX_SIZE 24
#define APP_PASSWORD_MAX_SIZE 128
#define BUTTON_TYPE_SWITCH 1
#define BUTTON_TYPE_PUSH 2
#define SENSOR_TYPE_BAR 1
#define SENSOR_TYPE_CIRCLE 2
/** uint16_t */
#define APP_CONFIG_VERSION 2
extern const char *APP_NVS_NAMESPACE;
// 4 byte aligned
struct app_config
{
    uint16_t config_version;
    uint8_t switch_len;
    uint8_t sensor_len;
    /** Digital value in bits, 1: on, 0: off*/
    uint8_t switch_values; // APP_SWITCH_COUNT / 8
    /** Switch status (msb: enable/disabled, type) */
    uint8_t switch_cfg[APP_SWITCH_COUNT];
    /** Sensor status (msb: enable/disabled, type) */
    uint8_t sensor_cfg[APP_SENSOR_COUNT];
    /** reserved 1*/
    uint8_t reserved1; // aligned
    /** delay in ms: min is 100 */
    uint16_t sensor_delay; // APP_SENSOR_COUNT / 8
    /** reserved 2*/
    uint16_t reserved2; // aligned
    char hostname[TCPIP_HOSTNAME_MAX_SIZE];
    /** Switches names */
    char switches[APP_SWITCH_COUNT][APP_NAME_MAX_SIZE];
    /** Sensor names */
    char sensors[APP_SENSOR_COUNT][APP_NAME_MAX_SIZE];
};

extern struct app_config config;

esp_err_t config_load();
esp_err_t config_save(void *handle);
esp_err_t nvs_clear(void *handle);
void config_print();

#endif