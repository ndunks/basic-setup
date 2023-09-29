#include <string.h>
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "wifi.h"
#include "state.h"
#include "errors.h"

#define TAG "app_wifi_action"

static void set_wifi_mode(wifi_mode_t set_mode)
{
    wifi_mode_t cur_mode;
    if ((esp_wifi_get_mode(&cur_mode)) != ESP_OK)
        return;

    if (cur_mode == WIFI_MODE_NULL)
    {
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_set_mode(set_mode));
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_start());
    }
    else if (cur_mode != WIFI_MODE_APSTA && cur_mode != set_mode)
    {
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_set_mode(WIFI_MODE_APSTA));
    }

    // if (set_mode == WIFI_MODE_STA)
    //     ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_set_auto_connect(true));
}

static void unset_wifi_mode(wifi_mode_t set_mode)
{
    wifi_mode_t cur_mode;
    // if ((ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_get_mode(&cur_mode))) != ESP_OK)
    if (esp_wifi_get_mode(&cur_mode) != ESP_OK)
        return;

    if (cur_mode == WIFI_MODE_APSTA)
    {
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_set_mode(set_mode == WIFI_MODE_AP ? WIFI_MODE_STA : WIFI_MODE_AP));
    }
    else if (cur_mode == set_mode)
    {
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_set_mode(WIFI_MODE_NULL));
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_stop());
    }
}

esp_err_t app_wifi_ap_start(const char *ssid, const char *pass)
{
    esp_err_t err;

    if ((STATE_IS(STATE_AP_STARTED)))
    {
        // ESP_LOGI(TAG, "AP Already started");
        return APP_ERR_ALREADY_STARTED;
    }

    set_wifi_mode(WIFI_MODE_AP);

    if (ssid)
    {
        ESP_LOGI(TAG, "Configuring AP ssid: '%s'", ssid);
        wifi_config_t cfg = {0}, cfg_old = {0};

        if (esp_wifi_get_config(WIFI_IF_AP, &cfg_old) != ESP_OK)
        {
            return APP_ERR_ERROR;
        }

        strncpy((char *)cfg.ap.ssid, ssid, sizeof(cfg.ap.ssid));

        if (pass)
        {
            strncpy((char *)cfg.ap.password, pass, sizeof(cfg.ap.password));
            cfg.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
        }
        else
            cfg.ap.authmode = WIFI_AUTH_OPEN;

        if (cfg_old.ap.max_connection > 0 && cfg_old.ap.max_connection <= CONFIG_APP_WIFI_MAX_CLIENT)
            cfg.ap.max_connection = cfg_old.ap.max_connection;
        else
            cfg.ap.max_connection = CONFIG_APP_WIFI_MAX_CLIENT;

        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_set_config(ESP_IF_WIFI_STA, &cfg));
    }

    // If not connected because WIFI_START not triggered, call connect not
    EventBits_t bits = xEventGroupWaitBits(APP_STATE, STATE_AP_STARTED, 1, 1, 3000 / portTICK_PERIOD_MS);
    if ((bits & STATE_AP_STARTED) != STATE_AP_STARTED)
    {
        return APP_ERR_NOT_STARTED;
    }

    return APP_OK;
}

esp_err_t app_wifi_ap_stop()
{
    if (STATE_IS(STATE_AP_STARTED))
    {
        unset_wifi_mode(WIFI_MODE_AP);
        return APP_OK;
    }
    else
        return APP_ERR_NOT_CONNECTED;
}

esp_err_t app_wifi_connect(const char *ssid, const char *pass)
{
    EventBits_t bits = STATE();

    if ((bits & (STATE_STA_CONNECTED | STATE_STA_CONNECTING)))
    {
        ESP_LOGI(TAG, "Already connecting %08x", bits);
        return APP_ERR_ALREADY_CONNECTED;
    }

    set_wifi_mode(WIFI_MODE_STA);
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_set_auto_connect(true));

    if (ssid)
    {
        ESP_LOGI(TAG, "Connecting to '%s'", ssid);
        wifi_config_t wifi_config = {0};
        strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));

        if (pass)
            strncpy((char *)wifi_config.sta.password, pass, sizeof(wifi_config.sta.password));

        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    }
    else
    {
        ESP_LOGI(TAG, "Connecting to saved ssid");
    }
    // If not connected because WIFI_START not triggered, call connect not
    bits = xEventGroupWaitBits(APP_STATE, STATE_STA_CONNECTING, 1, 1, 3000 / portTICK_PERIOD_MS);
    if ((bits & STATE_STA_CONNECTING) != STATE_STA_CONNECTING)
    {
        STATE_SET(STATE_STA_CONNECTING)
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_connect());
    }

    return APP_OK;
}

esp_err_t app_wifi_disconnect(void)
{
    EventBits_t bits = STATE();

    if (IS_BITS(STATE_STA_CONNECTING, bits))
    {
        ESP_LOGI(TAG, "Canceling reconnect from cmd");
        bits = STATE_CLR(STATE_STA_CONNECTING);
    }

    if ((bits & (STATE_STA_CONNECTED | STATE_STA_CONNECTING)))
    {
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_set_auto_connect(false));
        unset_wifi_mode(WIFI_MODE_STA);
        // just change the mode, no need to call this
        // ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_disconnect());
        return APP_OK;
    }
    else
    {
        ESP_LOGI(TAG, "Sta not connected");
        return APP_ERR_NOT_CONNECTED;
    }
}
