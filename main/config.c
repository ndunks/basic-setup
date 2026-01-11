#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_system.h"
#include "nvs.h"
#include "esp8266/rom_functions.h"
#include "esp_sha.h"
#include "config.h"
#if CONFIG_APP_WITH_COMMANDS
#include "terminal.h"
#endif
#include "tcpip_adapter.h"
#include "web-socket-handler.h"

#define TAG "config"
struct app_config config;

// extern esp_rom_spiflash_chip_t g_rom_flashchip

const char *APP_NVS_NAMESPACE = "app";
static const char *CONFIG_NAMEKEY = "cfg";
static const char *CONFIG_PWDKEY = "pwd";
static const char CONFIG_DEFAULT_PASSWORD[] = "admin";

static esp_err_t nvs_save(void *handle, const char *key, const char *data, size_t data_len)
{
    esp_err_t err;
    nvs_handle local_handle = 0u;
    if (handle == NULL)
    {
        err = nvs_open(APP_NVS_NAMESPACE, NVS_READWRITE, &local_handle);

        if (err == ESP_ERR_NVS_NOT_INITIALIZED)
        {
            ESP_LOGE(TAG, "%s: NVS not ready", __func__);
            return err;
        }
        else if (err != ESP_OK)
        {
            ESP_LOGW(TAG, "%s: %s (0x%x)", __func__, esp_err_to_name(err), err);
            return err;
        }
        handle = &local_handle;
    }

    err = nvs_set_blob(*(nvs_handle *)handle, key, data, data_len);
    if (err != ESP_OK)
        ESP_LOGW(TAG, "%s: %s (0x%x)", __func__, esp_err_to_name(err), err);

    // close if we opened it
    if (local_handle)
        nvs_close(local_handle);

    return err;
}

esp_err_t nvs_clear(void *handle)
{
    esp_err_t err;
    nvs_handle local_handle = 0u;
    if (handle == NULL)
    {
        err = nvs_open(APP_NVS_NAMESPACE, NVS_READWRITE, &local_handle);

        if (err == ESP_ERR_NVS_NOT_INITIALIZED)
        {
            ESP_LOGE(TAG, "%s: NVS not ready", __func__);
            return err;
        }
        else if (err != ESP_OK)
        {
            ESP_LOGW(TAG, "%s: %s (0x%x)", __func__, esp_err_to_name(err), err);
            return err;
        }
        handle = &local_handle;
    }

    err = nvs_erase_all(*(nvs_handle *)handle);
    if (err != ESP_OK)
        ESP_LOGW(TAG, "%s: %s (0x%x)", __func__, esp_err_to_name(err), err);

    // close if we opened it
    if (local_handle)
        nvs_close(local_handle);

    return err;
}

static int password_hash(const char *plain, size_t len, char *out32byte)
{
    esp_sha256_t ctx = {0};
    esp_sha256_init(&ctx);
    esp_sha256_update(&ctx, plain, len);
    return esp_sha256_finish(&ctx, out32byte);
}

static esp_err_t password_check(const char *plain, size_t len)
{
    esp_err_t err;
    nvs_handle handle;
    char input_pwd[32] = {0};
    char stored_pwd[32] = {0};
    size_t stored_len = sizeof(stored_pwd);

    if (password_hash(plain, len, input_pwd) != 0)
        return ESP_FAIL;

    err = nvs_open(APP_NVS_NAMESPACE, NVS_READONLY, &handle);

    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "nvs_open: %s (0x%x)", esp_err_to_name(err), err);
        return err;
    }

    err = nvs_get_blob(handle, CONFIG_PWDKEY, &stored_pwd, &stored_len);
    if (err == ESP_OK)
    {
        if (stored_len != sizeof(stored_pwd))
        {
            ESP_LOGW(TAG, "%s: invalid length of pwd (%d)", __func__, stored_len);
            err = ESP_FAIL;
        }
        else if (memcmp(stored_pwd, input_pwd, sizeof(stored_pwd)) != 0)
            err = ESP_FAIL;
    }
    else
        ESP_LOGW(TAG, "nvs_get_blob: %s (0x%x)", esp_err_to_name(err), err);

    nvs_close(handle);
    return err;
}

static esp_err_t update_password(const char *plain, size_t len, void *handle)
{
    char hashed[32] = {0};
    if (password_hash(plain, len, hashed) != 0)
        return ESP_FAIL;

    return nvs_save(NULL, CONFIG_PWDKEY, hashed, sizeof(hashed));
}

esp_err_t config_save(void *handle)
{
    return nvs_save(handle, CONFIG_NAMEKEY, (void *)&config, sizeof(struct app_config));
}

static void config_reset()
{
    uint8_t mac[6];
    int i;
    // todo: verify is correct ?
    uint32_t chip_id = g_rom_flashchip.device_id;
    char device_id[7 * 2 + 1];

    ESP_LOGI(TAG, __func__);
    // reset values
    memset(&config, 0, sizeof(struct app_config));

    // 3 hex last efuse + null
    // char *efuse_id[7] = {0};

    if (esp_efuse_mac_get_default(mac) != ESP_OK)
        ESP_LOGW(TAG, "Fail get efuse mac");
    // mac: 42F5 200AAF7B
    // cid:      005E6014
    // 0A AF 7B EF 40 16
#ifndef CONFIG_LOG_BOOTLOADER_LEVEL_NONE
    sprintf(device_id, "%02X%02X%02X%08X",
            mac[3], mac[4], mac[5],
            chip_id);
#endif
    config.config_version = APP_CONFIG_VERSION;
    config.switch_len = APP_SWITCH_COUNT;
    config.sensor_len = APP_SENSOR_COUNT;
    config.switch_values = 0;
    // All enabled, type = switch
    memset(config.switch_cfg, 1 << 7 | BUTTON_TYPE_SWITCH, APP_SWITCH_COUNT);
    // All enabled, type = bar
#if (APP_SENSOR_COUNT != 0)
    memset(config.sensor_cfg, 1 << 7 | SENSOR_TYPE_BAR, APP_SENSOR_COUNT);
#endif
    config.sensor_delay = 1000;
#ifndef CONFIG_LOG_BOOTLOADER_LEVEL_NONE
    sprintf(config.hostname, "OSH-%s", device_id);
    for (i = 0; i < APP_SWITCH_COUNT; i++)
    {
        sprintf(config.switches[i], "Switch %d", i + 1);
    }
    for (i = 0; i < APP_SENSOR_COUNT; i++)
    {
        sprintf(config.sensors[i], "Sensor %d", i + 1);
    }
#endif
    // reset wifi mode too ? or separate command
}

void config_print()
{
    printf("App Config:\n"
           "\tconfig_version: %u\n"
           "\tswitch_len: %u\n"
           "\tswitch_values: %u\n"
           "\tsensor_len: %u\n"
           "\tsensor_delay: %u\n"
           "\thostname: %s\n",
           config.config_version,
           config.switch_len,
           config.switch_values,
           config.sensor_len,
           config.sensor_delay,
           config.hostname);
}

static void on_ws_client(ws_cli_conn_t *client, const unsigned char *_, uint64_t size, int type)
{

    // Send config struct without password
    size_t len = sizeof(struct app_config) + 1;
    char *ws_msg = malloc(len);
    ws_msg[0] = WS_MSG_ID_CONFIG;
    memcpy(ws_msg + 1, &config, len - 1);
    ws_sendframe_bin(client, ws_msg, len);
    free(ws_msg);
}

static void on_ws_login(ws_cli_conn_t *client, const unsigned char *msg, uint64_t size, int type)
{

    // Send config struct without password
    char ws_msg[2] = {WS_MSG_ID_LOGIN, client->is_login};

    if (!client->is_login)
    {
        if (size > 0 && size <= APP_PASSWORD_MAX_SIZE)
        {
            if (password_check((const char *)msg, size) == ESP_OK)
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
    if (size > 0 && size <= TCPIP_HOSTNAME_MAX_SIZE)
    {
        // todo: validating allowed hostname chars
        memcpy(config.hostname, msg, size);
        if (size < TCPIP_HOSTNAME_MAX_SIZE)
            config.hostname[size] = 0x00; // terminate with null

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

static void on_ws_update_password(ws_cli_conn_t *client, const unsigned char *msg, uint64_t size, int type)
{
    char ws_msg[2] = {WS_MSG_ID_UPDATE_PASSWORD, false};

    if (size > 0 && size <= APP_PASSWORD_MAX_SIZE)
    {
        if (update_password((const char *)msg, size, NULL) == ESP_OK)
            ws_msg[1] = true;
    }
    // Send reply to specific client
    ws_sendframe_bin(client, ws_msg, 2);
}

static void on_ws_update_switches(ws_cli_conn_t *client, const unsigned char *msg, uint64_t size, int type)
{
    char ws_msg[2] = {WS_MSG_ID_UPDATE_SWITCHES, false};
    ssize_t expectedLen = sizeof(config.switches) + APP_SWITCH_COUNT;
    if (size == expectedLen)
    {
        memcpy(config.switches, msg, sizeof(config.switches));
        // set status and type
        memcpy(config.switch_cfg, msg + sizeof(config.switches), APP_SWITCH_COUNT);

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

static void on_ws_update_sensors(ws_cli_conn_t *client, const unsigned char *msg, uint64_t size, int type)
{
    char ws_msg[2] = {WS_MSG_ID_UPDATE_SENSORS, false};
    ssize_t expectedLen = sizeof(config.sensors) + APP_SENSOR_COUNT;
    if (size == expectedLen)
    {
        memcpy(config.sensors, msg, sizeof(config.sensors));
        // set status and type
        memcpy(config.sensor_cfg, msg + sizeof(config.sensors), APP_SENSOR_COUNT);

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
    // Possible race
    client->is_login = false;
}

static void on_ws_restart(ws_cli_conn_t *client, const unsigned char *msg, uint64_t size, int type)
{
    web_socket_close_all_clients();
    vTaskDelay(500 / portTICK_PERIOD_MS);
    esp_restart();
}

static esp_err_t reset_wifi_config()
{
    esp_err_t err;
    nvs_handle handle;
    err = nvs_open("nvs.net80211", NVS_READWRITE, &handle);

    if (err == ESP_ERR_NVS_NOT_INITIALIZED)
    {
        ESP_LOGE(TAG, "%s: NVS not ready", __func__);
        return err;
    }
    else if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "nvs_open: %s (0x%x)", esp_err_to_name(err), err);
        return err;
    }
    err = nvs_erase_all(handle);
    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "nvs_erase net80211: %s (0x%x)", esp_err_to_name(err), err);
        return err;
    }
    nvs_close(handle);
    return err;
}

static int cmd_reset(int argc, char **argv)
{
    nvs_clear(NULL);
    web_socket_close_all_clients();
    // Clear wifi config
    reset_wifi_config();
    vTaskDelay(500 / portTICK_PERIOD_MS);
    // system_restore();
    esp_restart();
    return 0;
}

static void on_ws_reset_config(ws_cli_conn_t *client, const unsigned char *msg, uint64_t size, int type)
{
    cmd_reset(0, NULL);
}

esp_err_t config_load()
{
    // if no config, set the default
    esp_err_t err;
    nvs_handle handle;
    err = nvs_open(APP_NVS_NAMESPACE, NVS_READWRITE, &handle);
    size_t len;

    if (err == ESP_ERR_NVS_NOT_INITIALIZED)
    {
        ESP_LOGE(TAG, "%s: NVS not ready", __func__);
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
        ESP_LOGW(TAG, "(!) Reset configuration");
        config_reset();
        update_password(CONFIG_DEFAULT_PASSWORD, sizeof(CONFIG_DEFAULT_PASSWORD) - 1, &handle);
        config_save(&handle);
    }
    else
    {
        if (config.sensor_delay < 100)
        {
            ESP_LOGW(TAG, "%s: sensor delay too short: %u, resetting it", __func__, config.sensor_delay);
            config.sensor_delay = 1000;
            config_save(&handle);
        }
    }
#ifndef CONFIG_LOG_BOOTLOADER_LEVEL_NONE
    config_print();
#endif
    nvs_close(handle);
#if CONFIG_APP_WITH_COMMANDS
    const esp_console_cmd_t reset_cmd = {
        .command = "reset",
        .help = "Reset configuration",
        .hint = NULL,
        .func = &cmd_reset,
        .argtable = NULL};

    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_console_cmd_register(&reset_cmd));
#endif

    web_socket_add_handler(WS_ON_OPEN, &on_ws_client);
    web_socket_add_handler(WS_MSG_ID_LOGIN, &on_ws_login);
    web_socket_add_handler_auth(WS_MSG_ID_LOGOUT, &on_ws_logout, true);
    web_socket_add_handler_auth(WS_MSG_ID_UPDATE_HOSTNAME, &on_ws_update_hostname, true);
    web_socket_add_handler_auth(WS_MSG_ID_UPDATE_SWITCHES, &on_ws_update_switches, true);
    web_socket_add_handler_auth(WS_MSG_ID_UPDATE_SENSORS, &on_ws_update_sensors, true);
    web_socket_add_handler_auth(WS_MSG_ID_UPDATE_PASSWORD, &on_ws_update_password, true);
    web_socket_add_handler_auth(WS_MSG_ID_RESET_CONFIG, &on_ws_reset_config, true);
    web_socket_add_handler_auth(WS_MSG_ID_RESTART, &on_ws_restart, true);
    return err;
}
