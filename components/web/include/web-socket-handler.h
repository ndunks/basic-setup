#ifndef WS_HANDLER_H
#define WS_HANDLER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "web-socket.h"

#define WS_ON_OPEN 0x00
#define WS_MSG_ID_ACTUATOR 0x01
#define WS_MSG_ID_CONFIG 0x02
#define WS_MSG_ID_SENSOR 0x03
#define WS_MSG_ID_LOGIN 0x04
#define WS_MSG_ID_LOGOUT 0x05
#define WS_MSG_ID_MESSAGE 0x06
#define WS_MSG_ID_UPDATE_HOSTNAME 0x07
#define WS_MSG_ID_UPDATE_SWITCHES 0x08

#define WS_MSG_ID_WIFI_CONFIG 0x09
#define WS_MSG_ID_WIFI_STA_SCAN 0x0a
#define WS_MSG_ID_WIFI_STA_UPDATE 0x0b
#define WS_MSG_ID_WIFI_AP_UPDATE 0x0c
#define WS_MSG_ID_UPDATE_PASSWORD 0x0d
#define WS_MSG_ID_RESET_CONFIG 0x0e
#define WS_MSG_ID_RESTART 0x0f

    typedef void (*web_socket_handler)(ws_cli_conn_t *client, const unsigned char *msg, uint64_t size, int type);

    /**
     * null code mean invoke on new client connected
     */
    int web_socket_add_handler_auth(unsigned char code, web_socket_handler handler, bool authenticated);
    int web_socket_add_handler(unsigned char code, web_socket_handler handler);
    int web_socket_close_all_clients();
#ifdef __cplusplus
}
#endif

#endif /* WS_HANDLER_H */