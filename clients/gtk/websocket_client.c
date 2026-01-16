#include "websocket_client.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

static struct lws_context *context = NULL;
static struct lws *websocket = NULL;
static const char *websocket_url;
static const char *websocket_protocol;

static int interrupted = 0;
static void (*on_message_callback)(const char *message) = NULL;
static pthread_t websocket_thread;
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
        lwsl_user("web_socket_add_handler: No mem\n");
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

// Signal handler for graceful shutdown
static void sigint_handler(int sig)
{
    interrupted = 1;
}

// WebSocket callback function
static int websocket_callback(struct lws *wsi, enum lws_callback_reasons reason,
                              void *user, void *in, size_t len)
{
    ws_item_t *cur;
    switch (reason)
    {
    case LWS_CALLBACK_CLIENT_ESTABLISHED:
        lwsl_user("Connected to the server\n");
        break;

    case LWS_CALLBACK_CLIENT_RECEIVE:
        if (lws_frame_is_binary(wsi))
        {
            // Handle binary message
            unsigned char *binary_data = (unsigned char *)in;
            lwsl_user("Received binary data of length %zu\n", len);
            // Process binary data here, e.g., save it to a file or buffer
            if (len < 1)
                return 0; // too short
            int handledCount = 0;
            unsigned char *msg = in;

            SLIST_FOREACH(cur, &ws_handler, next)
            {
                if (cur->code == msg[0])
                {
                    handledCount++;
                    cur->handler(wsi, msg + 1, len - 1);    
                }
            }
            if (handledCount == 0)
            {
                lwsl_user("Unhandled binary data: %0x\n", msg[0]);
                // errCmdNotfound[0] = msg[0];
                // errCmdNotfound[1] = WS_MSG_ID_MESSAGE;
                // ws_sendframe_bin(client, errCmdNotfound, sizeof(errCmdNotfound) - 1);
            }
        }
        else
        {
            // Handle text message
            lwsl_user("Received text message: %s\n", (char *)in);
            if (on_message_callback)
            {
                on_message_callback((const char *)in);
            }
        }
        break;

    case LWS_CALLBACK_CLIENT_WRITEABLE:
        break;

    case LWS_CALLBACK_CLIENT_CLOSED:
        lwsl_user("Connection closed\n");
        websocket = NULL;
        break;

    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        lwsl_user("Connection error\n");
        websocket = NULL;
        break;

    default:
        break;
    }
    return 0;
}

// Initialize the WebSocket client
void websocket_client_init(const char *url, const char *protocol)
{
    websocket_url = url;
    websocket_protocol = protocol;

    lws_set_log_level(
        LLL_ERR |
            LLL_WARN |
            // LLL_NOTICE |
            // LLL_INFO |
            // LLL_DEBUG |
            // LLL_PARSER |
            LLL_HEADER |
            // LLL_EXT |
            // LLL_CLIENT |
            // LLL_LATENCY |
            LLL_USER,
        NULL);

    struct lws_protocols protocols[] = {
        {
            .name = protocol,
            .callback = websocket_callback,
            .per_session_data_size = 0,
            .rx_buffer_size = 0,
        },
        {NULL, NULL, 0, 0}};

    struct lws_context_creation_info context_info = {
        .protocols = protocols,
        .options = 0,
    };

    context = lws_create_context(&context_info);
    if (!context)
    {
        lwsl_err("Failed to create context\n");
        exit(1);
    }

    signal(SIGINT, sigint_handler);
    lwsl_user("WebSocket client initialized\n");
}

// Connect to the WebSocket server
static void connect_websocket()
{
    if (websocket)
    {
        return; // Already connected
    }

    struct lws_client_connect_info connect_info = {
        .context = context,
        .address = websocket_url,
        .port = 80,
        .path = "/ws",
        .host = lws_canonical_hostname(context),
        .origin = "origin",
        .ssl_connection = 0,
        .protocol = websocket_protocol,
        .pwsi = &websocket,
    };

    if (!lws_client_connect_via_info(&connect_info))
    {
        lwsl_user("Failed to initiate connection\n");
    }
    else
    {
        lwsl_user("Initiating connection\n");
    }
}

// Set the callback for received messages
void websocket_set_on_message(void (*callback)(const char *message))
{
    on_message_callback = callback;
}

// Send a message through the WebSocket
void websocket_send_message(const char *message)
{
    if (!websocket)
    {
        lwsl_user("WebSocket is not connected\n");
        return;
    }

    size_t len = strlen(message);
    unsigned char buf[LWS_PRE + len];
    memcpy(&buf[LWS_PRE], message, len);

    lws_callback_on_writable(websocket);
    lws_write(websocket, &buf[LWS_PRE], len, LWS_WRITE_TEXT);
}
// Send a message through the WebSocket
void websocket_send_binary(const char *buf, size_t len)
{
    if (!websocket)
    {
        lwsl_user("WebSocket is not connected\n");
        return;
    }
    lws_callback_on_writable(websocket);
    lws_write(websocket, buf, len, LWS_WRITE_BINARY);
}

// WebSocket service loop
static void *websocket_service_loop(void *arg)
{
    lwsl_user("WebSocket client running in thread\n");
    while (!interrupted)
    {
        connect_websocket();
        lws_service(context, 1000);

        if (!websocket)
        {
            lwsl_user("Reconnecting in 3 seconds...\n");
            sleep(3);
        }
    }
    lwsl_user("Exiting WebSocket service loop\n");
    return NULL;
}

// Start the WebSocket client in a separate thread
void websocket_client_start_threaded()
{
    if (pthread_create(&websocket_thread, NULL, websocket_service_loop, NULL) != 0)
    {
        lwsl_err("Failed to create WebSocket thread\n");
        exit(1);
    }
}

// Stop the WebSocket client
void websocket_client_stop()
{
    interrupted = 1;

    if (pthread_join(websocket_thread, NULL) != 0)
    {
        lwsl_err("Failed to join WebSocket thread\n");
    }

    if (context)
    {
        lws_context_destroy(context);
        context = NULL;
    }
}
