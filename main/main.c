#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_event.h"
#include "esp_err.h"
#include "esp_console.h"
#include "state.h"
#include "actuator.h"
#if CONFIG_APP_WITH_SENSOR
#include "sensor.h"
#endif
#if CONFIG_APP_WITH_COMMANDS
#include "terminal.h"
#endif
#include "wifi.h"
#include "config.h"
#include "web-server.h"
#include "driver/gpio.h"
#include "web-socket-handler.h"
#define TAG "main"

#if CONFIG_APP_WITH_COMMANDS
static int cmd_dump(int argc, char **argv)
{
    config_print();
    printf("\nWeb Files:\n");
    for (int i = 0; i < ((sizeof web_files) / sizeof(webfs_t)); i++)
    {
        const webfs_t *f = &web_files[i];

        printf("\t%d, %s, %s %s\n", i, f->name, f->type, f->gzip ? "(gzip)" : "");
    }
    return 0;
}
#endif

void app_main()
{

    uint8_t mac[6];
    wifi_init_config_t init_config = WIFI_INIT_CONFIG_DEFAULT();

    config_load();
    actuator_setup(config.switch_values);

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    APP_STATE = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_mac_init());
    if (esp_efuse_mac_get_default(mac) == ESP_OK)
    {
        if (esp_base_mac_addr_set(mac) != ESP_OK)
            ESP_LOGW(TAG, "Fail set mac");
    }
    else
        ESP_LOGW(TAG, "Fail get efuse mac");

    tcpip_adapter_init();

    ESP_ERROR_CHECK(esp_wifi_init(&init_config));

#if CONFIG_APP_WITH_SENSOR
    sensor_setup();
#endif
    ESP_ERROR_CHECK_WITHOUT_ABORT(app_wifi_start());

    web_server_main();
#if CONFIG_APP_WITH_COMMANDS
    const esp_console_cmd_t dump_cmd = {
        .command = "dump",
        .help = "System dump info",
        .hint = NULL,
        .func = &cmd_dump,
        .argtable = NULL};

    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_console_cmd_register(&dump_cmd));

    initialize_terminal();
#endif
}
