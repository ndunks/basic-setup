#include "stdio.h"
#include "stdlib.h"
#include "main.h"
#include "config.h"
#include "web-socket-handler.h"

typedef struct ws_item
{
    unsigned char code;
    web_socket_handler handler;
    SLIST_ENTRY(ws_item)
    next;
} ws_item_t;

static SLIST_HEAD(ws_handler_, ws_item) ws_handler;

int web_socket_add_handler(unsigned char code, web_socket_handler handler)
{
    ws_item_t *new = calloc(1, sizeof(ws_item_t));
    if (new == NULL)
    {
        printf("web_socket_add_handler: No mem\n");
        return -1;
    }
    new->code = code;
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

void ws_onopen(ws_cli_conn_t *client)
{
    ws_item_t *cur;
    char *cli;
    cli = ws_getaddress(client);
    printf("Connection opened, addr: %s\n", cli);

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
    ws_item_t *cur;
    char *cli = ws_getaddress(client);
    printf("I receive a message: %s (size: %u, type: %d), from: %s\n", msg, (uint32_t)size, type, cli);
    if (size < 1)
        return; // too short

    SLIST_FOREACH(cur, &ws_handler, next)
    {
        if (cur->code == msg[0])
            cur->handler(client, msg, size, type);
    }
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
    //ws_sendframe(NULL, (char *)msg, size, type);
}