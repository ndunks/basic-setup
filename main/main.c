#include <stdio.h>
#include <string.h>
#include "terminal.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"

#define TAG "main"

static void initialize_nvs()
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

void app_main()
{
    initialize_nvs();
    initialize_terminal();
}
