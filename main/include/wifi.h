#ifndef APP_WIFI_H
#define APP_WIFI_H

#include "esp_err.h"

esp_err_t app_wifi_start(void);
esp_err_t app_wifi_connect(const char *ssid, const char *pass);
esp_err_t app_wifi_disconnect(void);
esp_err_t app_wifi_ap_start(const char *ssid, const char *pass);
esp_err_t app_wifi_ap_stop();

#endif