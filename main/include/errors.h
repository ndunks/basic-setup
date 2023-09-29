#ifndef APP_ERRORS_H
#define APP_ERRORS_H

#define APP_OK 0x0

#define APP_ERR_BASE 0xbad00
#define APP_ERR_ERROR 0xbad00
#define APP_ERR_ARGUMENT_ERROR 0xbad01

// General Wifi
#define APP_ERR_WIFI_ERROR 0xbad02

// Wifi STA Mode
#define APP_ERR_ALREADY_CONNECTED 0xbad03
#define APP_ERR_NOT_CONNECTED 0xbad04

// Wifi AP Mode
#define APP_ERR_ALREADY_STARTED 0xbad05
#define APP_ERR_NOT_STARTED 0xbad06

#endif