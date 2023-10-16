#include "state.h"
#include "wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "config.h"

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

    // web_socket_add_handler_auth(WS_MSG_ID_LOGOUT, &on_ws_logout, true);
    // web_socket_add_handler_auth(WS_MSG_ID_UPDATE_HOSTNAME, &on_ws_update_hostname, true);

    return ESP_OK;

error3:
    esp_event_handler_unregister(IP_EVENT, ESP_EVENT_ANY_ID, &handle_ip_event);
error2:
    esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &handle_wifi_event);
    ESP_LOGE(TAG, "%x app_wifi_start: %s", err, esp_err_to_name(err));
    STATE_SET(STATE_WIFI_ERROR);
    return err;
}
