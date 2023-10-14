#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "tcpip_adapter.h"
// Only support for 8
#define APP_SWITCH_COUNT 8
#define APP_SENSOR_COUNT 0
#define APP_NAME_MAX_SIZE 28
extern const char *APP_NVS_NAMESPACE;
struct app_config
{
    uint16_t config_version;
    uint8_t switch_len;
    uint8_t sensor_len;
    uint8_t switch_values; // APP_SWITCH_COUNT / 8
    char hostname[TCPIP_HOSTNAME_MAX_SIZE];
    char password[APP_NAME_MAX_SIZE];
    char switches[APP_SWITCH_COUNT][APP_NAME_MAX_SIZE];
    char sensors[APP_SENSOR_COUNT][APP_NAME_MAX_SIZE];
};

extern struct app_config config;

esp_err_t config_load();
esp_err_t config_save(void *handle);
void config_reset();
void config_print();

#endif