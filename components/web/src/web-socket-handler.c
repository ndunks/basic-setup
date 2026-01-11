#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "config.h"
#include "web-socket-handler.h"
#include <esp_log.h>
#define TAG "ws-handler"

typedef struct ws_item
{
    unsigned char code;
    bool authenticated;
    web_socket_handler handler;
    SLIST_ENTRY(ws_item)
    next;
} ws_item_t;

static SLIST_HEAD(ws_handler_, ws_item) ws_handler;
int web_socket_add_handler_auth(unsigned char code, web_socket_handler handler, bool authenticated)
{
    ws_item_t *new = calloc(1, sizeof(ws_item_t));
    if (new == NULL)
    {
        ESP_LOGE(TAG, "web_socket_add_handler: No mem");
        return -1;
    }
    new->code = code;
    new->authenticated = authenticated;
    new->handler = handler;

    ws_item_t *end = SLIST_FIRST(&ws_handler);
    if (end == NULL)
    {
        SLIST_INSERT_HEAD(&ws_handler, new, next);
    }
    else
    {
        ws_item_t *cur;
        while ((cur = SLIST_NEXT(end, next)) != NULL)
            end = cur;

        SLIST_INSERT_AFTER(end, new, next);
    }
    return 0;
}

int web_socket_close_all_clients()
{
    int cc = WS_CLSE_NORMAL;
    unsigned char clse_code[2];
    clse_code[0] = (cc >> 8);
    clse_code[1] = (cc & 0xFF);
    return ws_sendframe(NULL, (const char *)clse_code, sizeof(char) * 2, WS_FR_OP_CLSE);
}

int web_socket_add_handler(unsigned char code, web_socket_handler handler)
{
    return web_socket_add_handler_auth(code, handler, false);
}

void ws_onopen(ws_cli_conn_t *client)
{
    ws_item_t *cur;
    ESP_LOGI(client->info, "WS opened");

    SLIST_FOREACH(cur, &ws_handler, next)
    {
        if (cur->code == 0)
            cur->handler(client, NULL, 0, 0);
    }
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
    ESP_LOGW(client->info, "WS closed");
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
    ws_item_t *cur;
    // char *cli = ws_getaddress(client);
    static char errUnauthorized[] = "??Unauthorized";
    static char errCmdNotfound[] = "??Invalid command";

    // printf("I receive a message: %s (size: %u, type: %d), from: %s\n", msg, (uint32_t)size, type, cli);
    if (size < 1)
        return; // too short
    int handledCount = 0;

    SLIST_FOREACH(cur, &ws_handler, next)
    {
        if (cur->code == msg[0])
        {
            handledCount++;
            if (cur->authenticated && !client->is_login)
            {
                // send forbidden
                errUnauthorized[0] = msg[0];
                errUnauthorized[1] = WS_MSG_ID_MESSAGE;
                ws_sendframe_bin(client, errUnauthorized, sizeof(errUnauthorized) - 1);
            }
            else
            {
                // Remove code
                cur->handler(client, msg + 1, size - 1, type);
            }
        }
    }
    if (handledCount == 0)
    {
        errCmdNotfound[0] = msg[0];
        errCmdNotfound[1] = WS_MSG_ID_MESSAGE;
        ws_sendframe_bin(client, errCmdNotfound, sizeof(errCmdNotfound) - 1);
    }
}