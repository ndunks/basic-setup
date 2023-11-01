#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "tcpip_adapter.h"
// Only support for 8
#define APP_SWITCH_COUNT 8
#define APP_SENSOR_COUNT 8
#define APP_NAME_MAX_SIZE 24
#define APP_PASSWORD_MAX_SIZE 128
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
    /** Switch status (enable/disabled) */
    uint8_t switch_status; // APP_SWITCH_COUNT / 8
    /** Sensor status (enable/disabled) */
    uint8_t sensor_status; // APP_SENSOR_COUNT / 8
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
void config_reset();
void config_print();

#endif