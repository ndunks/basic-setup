#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_err.h"
#include "nvs.h"
#include "esp8266/rom_functions.h"
#include "config.h"
#include "tcpip_adapter.h"
#include "web-socket-handler.h"

#define TAG "config"
static const uint16_t APP_CONFIG_VERSION = 1u;
struct app_config config;

// extern esp_rom_spiflash_chip_t g_rom_flashchip

const char *APP_NVS_NAMESPACE = "app";
static const char *CONFIG_NAMEKEY = "cfg";

void config_reset()
{
    uint8_t mac[6];
    esp_err_t err;
    int i;
    // todo: verify is correct ?
    uint32_t chip_id = g_rom_flashchip.device_id;
    const char device_id[7 * 2 + 1];

    ESP_LOGI(TAG, __func__);
    // reset values
    memset(&config, 0, sizeof(struct app_config));

    // 3 hex last efuse + null
    // char *efuse_id[7] = {0};

    if (esp_efuse_mac_get_default(mac) == ESP_OK)
    {
        if (esp_base_mac_addr_set(mac) != ESP_OK)
            ESP_LOGW(TAG, "Fail set mac");
    }
    else
        ESP_LOGW(TAG, "Fail get efuse mac");
    // mac: 42F5 200AAF7B
    // cid:      005E6014
    // 0A AF 7B EF 40 16
    sprintf(device_id, "%02X%02X%02X%08X",
            mac[3], mac[4], mac[5],
            chip_id);

    config.config_version = APP_CONFIG_VERSION;
    config.switch_len = APP_SWITCH_COUNT;
    config.sensor_len = APP_SENSOR_COUNT;
    config.switch_values = 0;
    sprintf(config.hostname, "OSH-%s", device_id);
    sprintf(config.password, "admin");
    for (i = 0; i < APP_SWITCH_COUNT; i++)
    {
        sprintf(config.switches[i], "Switch %d", i + 1);
    }
    for (i = 0; i < APP_SENSOR_COUNT; i++)
    {
        sprintf(config.sensors[i], "Sensor %d", i + 1);
    }

    // reset wifi mode too ? or separate command
}

void config_print()
{
    printf("App Config:\n"
           "\tconfig_version: %u\n"
           "\tswitch_len: %u\n"
           "\tsensor_len: %u\n"
           "\tswitch_values: %u\n"
           "\thostname: %s\n"
           "\tpassword: %s\n",
           config.config_version,
           config.switch_len,
           config.sensor_len,
           config.switch_values,
           config.hostname,
           config.password);
}

esp_err_t config_save(void *handle)
{
    esp_err_t err;
    nvs_handle_t local_handle = 0u;
    if (handle == NULL)
    {
        err = nvs_open(APP_NVS_NAMESPACE, NVS_READWRITE, &local_handle);

        if (err == ESP_ERR_NVS_NOT_INITIALIZED)
        {
            ESP_LOGE(TAG, "%s: NVS has not been initialized. ", __func__);
            return err;
        }
        else if (err != ESP_OK)
        {
            ESP_LOGW(TAG, "nvs_open: %s (0x%x)", esp_err_to_name(err), err);
            return err;
        }
        handle = &local_handle;
    }

    err = nvs_set_blob(*(nvs_handle_t *)handle, CONFIG_NAMEKEY, &config, sizeof(struct app_config));
    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "nvs_get_blob: %s (0x%x)", esp_err_to_name(err), err);
    }

    // close if we opened it
    if (local_handle)
        nvs_close(local_handle);

    return err;
}

static void on_ws_client(ws_cli_conn_t *client, const unsigned char *msg, uint64_t size, int type)
{

    // Send config struct without password
    size_t len = sizeof(struct app_config) + 1;
    char *ws_msg = malloc(len);
    ws_msg[0] = WS_MSG_ID_CONFIG;
    memcpy(ws_msg + 1, &config, len - 1);
    // Nulled password
    memset(ws_msg + 1 + (offsetof(struct app_config, password)), '*', APP_NAME_MAX_SIZE);
    ws_sendframe_bin(client, ws_msg, len);
    free(ws_msg);
}
static void on_ws_login(ws_cli_conn_t *client, const unsigned char *msg, uint64_t size, int type)
{

    // Send config struct without password
    char ws_msg[2] = {WS_MSG_ID_LOGIN, client->is_login};

    if (!client->is_login)
    {
        if ((size - 1) > 0 && (size - 1) <= APP_NAME_MAX_SIZE)
        {
            if (strncmp(config.password, (const char *)(msg + 1), size - 1) == 0)
            {
                // password matched
                ws_msg[1] = true;
                client->is_login = true;
            }
        }
    }

    ws_sendframe_bin(client, ws_msg, 2);
}

static void on_ws_update_hostname(ws_cli_conn_t *client, const unsigned char *msg, uint64_t size, int type)
{

    char ws_msg[2] = {WS_MSG_ID_UPDATE_HOSTNAME, false};
    if ((size - 1) > 0 && (size - 1) <= TCPIP_HOSTNAME_MAX_SIZE)
    {
        memcpy(config.hostname, msg + 1, size - 1);
        if (size - 1 < TCPIP_HOSTNAME_MAX_SIZE)
            config.hostname[size - 1] = 0x00; // terminate with null

        if (config_save(NULL) == ESP_OK)
        {
            ws_msg[1] = true;
        }
    }
    // Send reply to specific client
    ws_sendframe_bin(client, ws_msg, 2);

    // Broadcast update config
    if (ws_msg[1] == true)
        on_ws_client(NULL, NULL, 0, WS_FR_OP_BIN);
}

static void on_ws_logout(ws_cli_conn_t *client, const unsigned char *msg, uint64_t size, int type)
{
    client->is_login = false;
}

esp_err_t config_load()
{
    // if no config, set the default
    esp_err_t err;
    nvs_handle_t handle;
    err = nvs_open(APP_NVS_NAMESPACE, NVS_READWRITE, &handle);
    size_t len;

    if (err == ESP_ERR_NVS_NOT_INITIALIZED)
    {
        ESP_LOGE(TAG, "%s: NVS has not been initialized. ", __func__);
        return err;
    }
    else if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "nvs_open: %s (0x%x)", esp_err_to_name(err), err);
        return err;
    }
    len = sizeof(struct app_config);
    err = nvs_get_blob(handle, CONFIG_NAMEKEY, &config, &len);
    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "nvs_get_blob: %s (0x%x)", esp_err_to_name(err), err);
    }
    else if (len != sizeof(struct app_config))
    {
        ESP_LOGW(TAG, "%s: invalid length of config (%d)", __func__, len);
        err = ESP_FAIL;
    }

    if (err != ESP_OK)
    {
        config_reset();
        config_save(&handle);
    }

    config_print();

    nvs_close(handle);
    web_socket_add_handler(WS_ON_OPEN, &on_ws_client);
    web_socket_add_handler(WS_MSG_ID_LOGIN, &on_ws_login);
    web_socket_add_handler_auth(WS_MSG_ID_LOGOUT, &on_ws_logout, true);
    web_socket_add_handler_auth(WS_MSG_ID_UPDATE_HOSTNAME, &on_ws_update_hostname, true);
    return err;
}
