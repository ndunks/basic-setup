#ifndef APP_STATE_H
#define APP_STATE_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "sdkconfig.h"

/* Global App State */
volatile EventGroupHandle_t APP_STATE;
#define STATE_CURRENT (*(uint32_t *)APP_STATE)
#define STATE() xEventGroupGetBits(APP_STATE);
#define STATE_SET(x) xEventGroupSetBits(APP_STATE, x);
#define STATE_CLR(x) xEventGroupClearBits(APP_STATE, x);
#define STATE_WAIT(x) xEventGroupWaitBits(APP_STATE, x, 1, 1, CONFIG_APP_WIFI_ACTION_TIMEOUT / portTICK_PERIOD_MS);
//#define STATE_IS(x) ((*(uint32_t *)APP_STATE & x) == x)
#define STATE_IS(x) ((xEventGroupGetBits(APP_STATE) & x) == x)
#define IS_BITS(x, xx) ((xx & (x)) == (x))
#endif

/* AP mode started */
#define STATE_AP_STARTED BIT1
/* Wifi in error state */
#define STATE_WIFI_ERROR BIT2
/* Connected to the internet */
#define STATE_INTERNET_CONNECTED BIT3
/* Trying to connect AP */
#define STATE_STA_CONNECTING BIT4
/* Connected to AP */
#define STATE_STA_CONNECTED BIT5
/* Fail to connect after several retry */
#define STATE_STA_FAIL BIT6

