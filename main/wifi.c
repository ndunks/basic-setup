#include <string.h>
#include "state.h"
#include "wifi.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"

/**
 * Wifi logic and functionaly goes here
 */

#define TAG "app_wifi"

static int retry_cntr = 0;

static void handle_wifi_event(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data)
{
    EventBits_t state = 0;
    switch (event_id)
    {
    case WIFI_EVENT_AP_START:
        STATE_SET(STATE_AP_STARTED);
        break;
    case WIFI_EVENT_AP_STOP:
        STATE_CLR(STATE_AP_STARTED);
        break;
    case WIFI_EVENT_STA_START:
        state = STATE();
        ESP_LOGI(TAG, "STA start, state %08x", state);

        if ((state & STATE_STA_DISABLE_AUTO_CONNECT) == STATE_STA_DISABLE_AUTO_CONNECT)
        {
        }
        else
        {
            esp_wifi_connect();
            STATE_SET(STATE_STA_CONNECTING | STATE_STA_DISCONNECTED);
        }
        break;

    case WIFI_EVENT_STA_STOP:
        STATE_CLR(STATE_STA_CONNECTING);
        ESP_LOGI(TAG, "STA STOP");
        break;

    case WIFI_EVENT_STA_CONNECTED:
        STATE_SET(STATE_STA_CONNECTED);
        STATE_CLR(STATE_STA_CONNECTING | STATE_STA_FAIL | STATE_STA_DISCONNECTED);
        retry_cntr = 0;
        break;

    case WIFI_EVENT_STA_DISCONNECTED:
        state = STATE_CLR(STATE_STA_CONNECTED);
        wifi_event_sta_disconnected_t *disconnected = (wifi_event_sta_disconnected_t *)event_data;

        ESP_LOGI(TAG, "Disconnect reason : %d, state %08x", disconnected->reason, state);
        if (retry_cntr < CONFIG_APP_WIFI_MAX_RETRY)
        {
            if (retry_cntr == 0)
            {
                if (disconnected->reason == WIFI_REASON_BASIC_RATE_NOT_SUPPORT)
                {
                    /*Switch to 802.11 bgn mode */
                    esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N);
                }
                else if (disconnected->reason == WIFI_REASON_NO_AP_FOUND)
                {
                    // Delay before retry connect
                    ESP_LOGI(TAG, "Delay before reconnect");
                    vTaskDelay(5000 / portTICK_PERIOD_MS);
                }
            }
            if ((state & STATE_STA_DISABLE_AUTO_CONNECT) == STATE_STA_DISABLE_AUTO_CONNECT)
            {
                ESP_LOGI(TAG, "No retry, auto connect disabled");
                STATE_SET(STATE_STA_DISCONNECTED);
            }
            else
            {
                retry_cntr++;
                ESP_LOGI(TAG, "retry %d to connect to the AP", retry_cntr);
                esp_wifi_connect();
                STATE_SET(STATE_STA_CONNECTING | STATE_STA_DISCONNECTED);
            }
        }
        else
        {
            STATE_SET(STATE_STA_FAIL | STATE_STA_DISCONNECTED);
            STATE_CLR(STATE_STA_CONNECTING);
            retry_cntr = 0;
            ESP_LOGW(TAG, "connect to the AP fail");
        }
        break;
    default:
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
        STATE_SET(STATE_STA_HAS_IP);
        // Check internet connection
        break;

    case IP_EVENT_STA_LOST_IP:
        ESP_LOGI(TAG, "STA lost ip");
        STATE_CLR(STATE_STA_HAS_IP);
        break;
    }
}

esp_err_t app_wifi_connect(const char *ssid, const char *pass)
{
    wifi_mode_t mode;
    esp_err_t err;
    wifi_config_t wifi_config = {0};

    if ((err = esp_wifi_get_mode(&mode)) != ESP_OK)
        return err;

    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    if (pass)
    {
        strncpy((char *)wifi_config.sta.password, pass, sizeof(wifi_config.sta.password));
    }

    STATE_CLR(STATE_STA_DISABLE_AUTO_CONNECT);
    ESP_ERROR_CHECK(esp_wifi_set_auto_connect(true));

    // warn: ESP_ERROR_CHECK can cause app exit
    if ((mode & WIFI_MODE_STA) != WIFI_MODE_STA)
    {
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    }
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_connect());

    int bits = STATE_WAIT(STATE_STA_CONNECTED);
    return (bits & STATE_STA_CONNECTED) != 0;
}

esp_err_t app_wifi_disconnect(void)
{
    STATE_SET(STATE_STA_DISABLE_AUTO_CONNECT);

    ESP_ERROR_CHECK(esp_wifi_set_auto_connect(false));

    esp_err_t err = esp_wifi_disconnect();

    if (err == ESP_OK)
    {
        EventBits_t bits = STATE_WAIT(STATE_STA_DISCONNECTED);
        return !((bits & STATE_STA_DISCONNECTED) == STATE_STA_DISCONNECTED);
    }

    return err;
}

/* Called on system boot */
esp_err_t app_wifi_start(void)
{
    uint8_t mac[6];
    bool vBool;
    // wifi_config_t config = {0};
    wifi_mode_t mode;
    esp_err_t err;
    wifi_init_config_t init_config = WIFI_INIT_CONFIG_DEFAULT();
    uint32_t init_step = 0;

    // Initial state
    EventBits_t initialState = STATE_STA_DISCONNECTED;

    if (esp_efuse_mac_get_default(mac) == ESP_OK)
    {
        if (esp_base_mac_addr_set(mac) != ESP_OK)
            ESP_LOGW(TAG, "Fail base mac");
    }
    else
        ESP_LOGW(TAG, "No efuse mac");

    if ((err = esp_mac_init()) != ESP_OK)
        goto error;

    tcpip_adapter_init();

    if ((err = esp_wifi_init(&init_config)) != ESP_OK)
        goto error;

    if (esp_wifi_get_auto_connect(&vBool) == ESP_OK)
    {
        // Disable auto connect on event handle
        if (!vBool)
            initialState |= STATE_STA_DISABLE_AUTO_CONNECT;
        ESP_LOGI(TAG, "Auto connect %d", vBool);
    }

    STATE_SET(initialState);

    init_step = 1;
    if ((err = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &handle_wifi_event, NULL)) != ESP_OK)
        goto error;
    init_step = 2;
    if ((err = esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &handle_ip_event, NULL)) != ESP_OK)
        goto error;
    init_step = 3;

    if ((err = esp_wifi_get_mode(&mode)) != ESP_OK)
        goto error;

    if ((err = esp_wifi_start()) != ESP_OK)
        goto error;

    return ESP_OK;

error:
    ESP_LOGE(TAG, "%x app_wifi_start: %s", err, esp_err_to_name(err));
    STATE_SET(STATE_WIFI_ERROR);
    switch (init_step)
    {
    case 3:
        esp_event_handler_unregister(IP_EVENT, ESP_EVENT_ANY_ID, &handle_ip_event);
    case 2:
        esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &handle_wifi_event);
    case 1:
        esp_wifi_deinit();
        break;
    }
    return err;
}
