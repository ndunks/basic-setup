#ifndef APP_STATE_H
#define APP_STATE_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "sdkconfig.h"

/* Global App State */
volatile EventGroupHandle_t APP_STATE;

#define STATE() xEventGroupGetBits(APP_STATE);
#define STATE_SET(x) xEventGroupSetBits(APP_STATE, x);
#define STATE_CLR(x) xEventGroupClearBits(APP_STATE, x);
#define STATE_WAIT(x) xEventGroupWaitBits(APP_STATE, x, 1, 1, CONFIG_APP_WIFI_ACTION_TIMEOUT / portTICK_PERIOD_MS);
#endif

/* AP mode started */
#define STATE_AP_STARTED BIT1
/* Wifi disabled */
#define STATE_WIFI_DISABLED BIT2
/* Wifi in error state */
#define STATE_WIFI_ERROR BIT3
/* Connected to the internet */
#define STATE_INTERNET_CONNECTED BIT4
/* Trying to connect AP */
#define STATE_STA_CONNECTING BIT5
/* Connected to AP */
#define STATE_STA_CONNECTED BIT6

#define STATE_STA_DISCONNECTED BIT7
/* Connect Failed after x times */
#define STATE_STA_FAIL BIT8
/* STA has IP */
#define STATE_STA_HAS_IP BIT9
/* Disable auto connect on lost connection */
#define STATE_STA_DISABLE_AUTO_CONNECT BIT10

