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

    typedef void (*web_socket_handler)(ws_cli_conn_t *client, const unsigned char *msg, uint64_t size, int type);
    /**
     * null code mean invoke on new client connected
     */
    int web_socket_add_handler(unsigned char code, web_socket_handler handler);
#ifdef __cplusplus
}
#endif

#endif /* WS_HANDLER_H */