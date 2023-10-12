#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_err.h"
#include "nvs.h"
#include "esp8266/rom_functions.h"
#include "config.h"
#include "tcpip_adapter.h"

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
        sprintf(config.switches[i], "Sensor %d", i + 1);
    }
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
    return err;
}
