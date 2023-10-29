#include <string.h>
#include "state.h"
#include "wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "config.h"
#include "web-socket-handler.h"

/**
 * Wifi logic and functionaly goes here
 */

#define TAG "app_wifi"

static int retry_cntr = 0;

static void handle_wifi_event(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "Wifi Event %d", event_id);
    bool auto_connect;
    uint8_t reason = 0;

    switch (event_id)
    {

    case WIFI_EVENT_AP_START:
        ESP_ERROR_CHECK_WITHOUT_ABORT(tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_AP, config.hostname));
        STATE_SET(STATE_AP_STARTED);
        break;

    case WIFI_EVENT_AP_STOP:
        STATE_CLR(STATE_AP_STARTED);
        break;

    case WIFI_EVENT_STA_START:
        ESP_LOGI(TAG, "STA START");
        esp_wifi_get_auto_connect(&auto_connect);
        if (auto_connect)
        {
            ESP_ERROR_CHECK_WITHOUT_ABORT(tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, config.hostname));
            STATE_CLR(STATE_STA_FAIL);
            STATE_SET(STATE_STA_CONNECTING);
            esp_wifi_connect(); // app_wifi_connect(NULL, NULL);
        }
        break;

    case WIFI_EVENT_STA_STOP:
        STATE_CLR(STATE_STA_CONNECTING);
        ESP_LOGI(TAG, "STA STOP");
        break;

    case WIFI_EVENT_STA_CONNECTED:
        STATE_CLR(STATE_STA_CONNECTING | STATE_STA_FAIL);
        STATE_SET(STATE_STA_CONNECTED);
        retry_cntr = 0;
        break;

    case WIFI_EVENT_STA_DISCONNECTED:

        reason = ((wifi_event_sta_disconnected_t *)event_data)->reason;
        ESP_LOGI(TAG, "Disconnect reason : %d, state %08x", reason, STATE_CURRENT);

        if (reason == WIFI_REASON_ASSOC_LEAVE)
        {
            // its disconected by user ?
            ESP_LOGI(TAG, "No retry, disconnected by user");
            STATE_CLR(STATE_STA_CONNECTING | STATE_STA_FAIL | STATE_STA_CONNECTED);
            return;
        }

        if (retry_cntr >= CONFIG_APP_WIFI_MAX_RETRY)
        {
            STATE_CLR(STATE_STA_CONNECTING);
            STATE_SET(STATE_STA_FAIL);
            retry_cntr = 0;
            ESP_LOGW(TAG, "connect to the AP fail");
            return;
        }

        STATE_CLR(STATE_STA_CONNECTED);
        // Will reconnect
        retry_cntr++;
        // STATE_SET(STATE_STA_CONNECTING);

        switch (reason)
        {
        case WIFI_REASON_BASIC_RATE_NOT_SUPPORT:
            /*Switch to 802.11 bgn mode */
            esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N);
            break;
        case WIFI_REASON_NO_AP_FOUND:
            ESP_LOGI(TAG, "Delay before reconnect");
            vTaskDelay(10000 / portTICK_PERIOD_MS);
            break;
        }

        // check if forced disconect by user
        if (!STATE_IS(STATE_STA_CONNECTING))
        {
            ESP_LOGI(TAG, ("Reconnect aborted by user"));
            return;
        }
        ESP_LOGI(TAG, "retry %d to connect to the AP", retry_cntr);
        esp_wifi_connect();
        break;
    }
}

static void handle_ip_event(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;

    switch (event_id)
    {
    case IP_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "STA got ip:%s",
                 ip4addr_ntoa(&event->ip_info.ip));
        retry_cntr = 0;
        // STATE_SET(STATE_STA_HAS_IP);
        // Check internet connection
        break;

    case IP_EVENT_STA_LOST_IP:
        STATE_CLR(STATE_INTERNET_CONNECTED);
        break;
    }
}
struct wifi_config_sta
{
    unsigned char msg_id;
    unsigned char is_enabled;
    unsigned char mac[6];
    tcpip_adapter_ip_info_t net;
    char ssid[32];
    char password[64];
    bool auto_connect;
    unsigned char padding2[3]; // padding
};

static void
on_ws_get_set_config_sta(ws_cli_conn_t *client, const unsigned char *msg, uint64_t size, int type)
{
    wifi_config_t cfg = {0};
    wifi_mode_t mode;
    struct wifi_config_sta ret = {0};

    if (size == sizeof(struct wifi_config_sta) - 1)
    {
        // exclude the msg_id
        memcpy((((void *)&ret) + 1), msg, size);

        ESP_LOGI(TAG, "Update %d " MACSTR "\nIPs " IPSTR " " IPSTR " " IPSTR " ssid: %s, p: %s\n %d", ret.is_enabled == 1, MAC2STR(ret.mac),
                 IP2STR((ip4_addr_t *)&ret.net.ip), IP2STR((ip4_addr_t *)&ret.net.netmask), IP2STR((ip4_addr_t *)&ret.net.gw),
                 ret.ssid, ret.password, ret.auto_connect);
        if (ret.is_enabled)
            app_wifi_connect(ret.ssid, ret.password);
        else
            app_wifi_disconnect();
    }

    ret.msg_id = WS_MSG_ID_WIFI_CONFIG_STA;
    esp_wifi_get_mode(&mode);
    ret.is_enabled = mode == WIFI_MODE_APSTA || mode == WIFI_MODE_STA;
    esp_read_mac(ret.mac, ESP_MAC_WIFI_STA);
    tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ret.net);
    esp_wifi_get_config(ESP_IF_WIFI_STA, &cfg);
    memcpy(ret.ssid, cfg.sta.ssid, sizeof(cfg.sta.ssid));
    memcpy(ret.password, cfg.sta.password, sizeof(cfg.sta.password));
    esp_wifi_get_auto_connect(&ret.auto_connect);

    ws_sendframe_bin(client, (void *)&ret, sizeof(struct wifi_config_sta));
}
struct wifi_config_ap
{
    unsigned char msg_id;
    unsigned char is_enabled;
    unsigned char mac[6];
    tcpip_adapter_ip_info_t net;
    char ssid[32];
    char password[64];
    bool is_started;
    unsigned char authmode;
    unsigned char padding2[2]; // padding
};

static void
on_ws_get_set_config_ap(ws_cli_conn_t *client, const unsigned char *msg, uint64_t size, int type)
{
    wifi_config_t cfg = {0};
    wifi_mode_t mode;
    struct wifi_config_ap ret = {0};
    EventBits_t bits = STATE();

    if (size == sizeof(struct wifi_config_ap) - 1)
    {
        // Request for update
        // exclude the msg_id
        memcpy((((void *)&ret) + 1), msg, size);

        ESP_LOGI(TAG, "Update %d " MACSTR "\nIPs " IPSTR " " IPSTR " " IPSTR " ssid: %s, p: %s\n %d %d", ret.is_enabled == 1, MAC2STR(ret.mac),
                 IP2STR((ip4_addr_t *)&ret.net.ip), IP2STR((ip4_addr_t *)&ret.net.netmask), IP2STR((ip4_addr_t *)&ret.net.gw),
                 ret.ssid, ret.password, ret.is_started, ret.authmode);
        if (ret.is_enabled)
            app_wifi_ap_start(ret.ssid, ret.password);
        else
            app_wifi_ap_stop();
    }

    ret.msg_id = WS_MSG_ID_WIFI_CONFIG_AP;
    esp_wifi_get_mode(&mode);
    ret.is_enabled = mode == WIFI_MODE_APSTA || mode == WIFI_MODE_AP;

    esp_read_mac(ret.mac, ESP_MAC_WIFI_SOFTAP);
    tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &ret.net);
    esp_wifi_get_config(ESP_IF_WIFI_AP, &cfg);
    memcpy(ret.ssid, cfg.ap.ssid, cfg.ap.ssid_len);
    memcpy(ret.password, cfg.ap.password, sizeof(cfg.ap.password));
    ret.is_started = (bits & STATE_AP_STARTED) == STATE_AP_STARTED;
    ret.authmode = cfg.ap.authmode & 0xff;
    ws_sendframe_bin(client, (void *)&ret, sizeof(struct wifi_config_ap));
}
struct ws_scan_result
{
    uint8_t bssid[6]; /**< MAC address of AP */
    uint8_t ssid[33]; /**< SSID of AP */
    int8_t rssi;      /**< signal strength of AP */
    uint8_t authmode;
    uint8_t padding[3];
};

static void send_scan_results(uint16_t *len, wifi_ap_record_t *records, void *param)
{
    size_t size = sizeof(struct ws_scan_result) * (*len) + 1;
    char *msg = malloc(size);
    struct ws_scan_result *res = (void *)(msg + 1);

    msg[0] = WS_MSG_ID_WIFI_STA_SCAN;

    ws_cli_conn_t *client = param;

    for (int i = 0; i < *len; i++)
    {
        // scan_ptr =  msg + 1 + (sizeof(struct ws_scan_result) * i);
        memcpy(res[i].bssid, records[i].bssid, sizeof(records[i].bssid));
        memcpy(res[i].ssid, records[i].ssid, sizeof(records[i].ssid));
        res[i].rssi = records[i].rssi;
        res[i].authmode = records[i].authmode & 0xff;
    }
    ws_sendframe_bin(client, msg, size);
}

static void
on_ws_sta_scan(ws_cli_conn_t *client, const unsigned char *msg, uint64_t size, int type)
{

    app_wifi_scan(&send_scan_results, client);
}

/* Called on system boot */
esp_err_t app_wifi_start(void)
{
    wifi_mode_t mode;
    esp_err_t err;

    if ((err = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &handle_wifi_event, NULL)) != ESP_OK)
        goto error2;

    if ((err = esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &handle_ip_event, NULL)) != ESP_OK)
        goto error3;

    if ((err = esp_wifi_get_mode(&mode)) != ESP_OK)
        goto error3;

    if (mode != WIFI_MODE_NULL)
    {
        if ((err = esp_wifi_start()) != ESP_OK)
            goto error3;
    }

    web_socket_add_handler_auth(WS_MSG_ID_WIFI_CONFIG_AP, &on_ws_get_set_config_ap, true);
    web_socket_add_handler_auth(WS_MSG_ID_WIFI_CONFIG_STA, &on_ws_get_set_config_sta, true);
    web_socket_add_handler_auth(WS_MSG_ID_WIFI_STA_SCAN, &on_ws_sta_scan, true);

    return ESP_OK;

error3:
    esp_event_handler_unregister(IP_EVENT, ESP_EVENT_ANY_ID, &handle_ip_event);
error2:
    esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &handle_wifi_event);
    ESP_LOGE(TAG, "%x app_wifi_start: %s", err, esp_err_to_name(err));
    STATE_SET(STATE_WIFI_ERROR);
    return err;
}
