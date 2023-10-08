#include "stdio.h"
#include "stdlib.h"
#include "web-socket.h"
#include "actuator.h"
#include "main.h"

void ws_onopen(ws_cli_conn_t *client)
{
    char *cli;
    cli = ws_getaddress(client);
    printf("Connection opened, addr: %s\n", cli);

    char ws_msg[2] = {WS_MSG_ID_ACTUATOR, actuator_value};

    ws_sendframe_bin(client, ws_msg, 2);
}

/**
 * @brief Called when a client disconnects to the server.
 *
 * @param client Client connection. The @p client parameter is used
 * in order to send messages and retrieve informations about the
 * client.
 */
void ws_onclose(ws_cli_conn_t *client)
{
    char *cli;
    cli = ws_getaddress(client);
    printf("Connection closed, addr: %s\n", cli);
}

/**
 * @brief Called when a client connects to the server.
 *
 * @param client Client connection. The @p client parameter is used
 * in order to send messages and retrieve informations about the
 * client.
 *
 * @param msg Received message, this message can be a text
 * or binary message.
 *
 * @param size Message size (in bytes).
 *
 * @param type Message type.
 */
void ws_onmessage(ws_cli_conn_t *client,
                  const unsigned char *msg, uint64_t size, int type)
{
    char *cli = ws_getaddress(client);
    printf("I receive a message: %s (size: %u, type: %d), from: %s\n", msg, (uint32_t)size, type, cli);

    /**
     * Mimicks the same frame type received and re-send it again
     *
     * Please note that we could just use a ws_sendframe_txt()
     * or ws_sendframe_bin() here, but we're just being safe
     * and re-sending the very same frame type and content
     * again.
     *
     * Client equals to NULL: broadcast
     */
    ws_sendframe(NULL, (char *)msg, size, type);
}