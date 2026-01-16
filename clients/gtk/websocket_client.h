#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H
#include <stdbool.h>
#include <stdint.h>
#include <sys/queue.h>
#include <libwebsockets.h>

#define WS_ON_OPEN 0x00
#define WS_MSG_ID_ACTUATOR 0x01
#define WS_MSG_ID_CONFIG 0x02
#define WS_MSG_ID_SENSOR 0x03
#define WS_MSG_ID_LOGIN 0x04
#define WS_MSG_ID_LOGOUT 0x05
#define WS_MSG_ID_MESSAGE 0x06
#define WS_MSG_ID_UPDATE_HOSTNAME 0x07
#define WS_MSG_ID_UPDATE_SWITCHES 0x08

#define WS_MSG_ID_WIFI_CONFIG_STA 0x09
#define WS_MSG_ID_WIFI_CONFIG_AP 0x0a
#define WS_MSG_ID_WIFI_STA_SCAN 0x0b
#define WS_MSG_ID_UPDATE_SENSORS 0x0c
#define WS_MSG_ID_UPDATE_PASSWORD 0x0d
#define WS_MSG_ID_RESET_CONFIG 0x0e
#define WS_MSG_ID_RESTART 0x0f

#define APP_SWITCH_COUNT 4
#define APP_SENSOR_COUNT 0
#define APP_NAME_MAX_SIZE 24
#define TCPIP_HOSTNAME_MAX_SIZE 32
struct app_config
{
    uint16_t config_version;
    uint8_t switch_len;
    uint8_t sensor_len;
    /** Digital value in bits, 1: on, 0: off*/
    uint8_t switch_values; // APP_SWITCH_COUNT / 8
    /** Switch status (msb: enable/disabled, type) */
    uint8_t switch_cfg[APP_SWITCH_COUNT];
    /** Sensor status (msb: enable/disabled, type) */
    uint8_t sensor_cfg[APP_SENSOR_COUNT];
    /** reserved 1*/
    uint8_t reserved1; // aligned
    /** delay in ms: min is 100 */
    uint16_t sensor_delay; // APP_SENSOR_COUNT / 8
    /** reserved 2*/
    uint16_t reserved2; // aligned
    char hostname[TCPIP_HOSTNAME_MAX_SIZE];
    /** Switches names */
    char switches[APP_SWITCH_COUNT][APP_NAME_MAX_SIZE];
    /** Sensor names */
    char sensors[APP_SENSOR_COUNT][APP_NAME_MAX_SIZE];
};

typedef void (*web_socket_handler)(struct lws *client, const unsigned char *msg, size_t size);
int web_socket_add_handler(unsigned char code, web_socket_handler handler);

// Initialize the WebSocket client
void websocket_client_init(const char *url, const char *protocol);

// Start the WebSocket service in a separate thread
void websocket_client_start_threaded(void);

// Set a callback for received messages
void websocket_set_on_message(void (*callback)(const char *message));

// Send a message through the WebSocket
void websocket_send_message(const char *message);
void websocket_send_binary(const char *buf, size_t len);

// Stop the WebSocket client
void websocket_client_stop(void);

#endif // WEBSOCKET_CLIENT_H
