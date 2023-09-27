#ifndef APP_STATE_H
#define APP_STATE_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

/* Trying to connect AP */
#define STATE_STA_CONNECTING BIT0
/* Connected to AP */
#define STATE_STA_CONNECTED BIT1
/* Connect Failed after x times */
#define STATE_STA_FAIL BIT2
/* STA has IP */
#define STATE_STA_HAS_IP BIT3
/* AP mode started */
#define STATE_AP_STARTED BIT4
/* Wifi disabled */
#define STATE_WIFI_DISABLED BIT5
/* Wifi in error state */
#define STATE_WIFI_ERROR BIT6
/* Connected to the internet */
#define STATE_INTERNET_CONNECTED BIT7

/* WIFI State of WIFI_STATE_* */
EventGroupHandle_t state;

#define STATE_SET(x) xEventGroupSetBits(state, x);
#define STATE_CLR(x) xEventGroupClearBits(state, x);
#endif