#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_event.h"
#include "esp_err.h"
#include "esp_console.h"
#include "state.h"
#include "terminal.h"
#include "actuator.h"
#include "wifi.h"
#include "config.h"
#include "web-server.h"
#include "driver/gpio.h"
#include "web-socket-handler.h"

static int cmd_dump(int argc, char **argv)
{
    // EventBits_t bits = STATE();
    // printf("STATE: %08x\n", bits);
    // printf("STATE: %08x\n", STATE_CURRENT);
    // printf("[%d] STATE_AP_STARTED\n", IS_BITS(STATE_AP_STARTED, STATE_CURRENT));
    // printf("[%d] STATE_WIFI_ERROR\n", IS_BITS(STATE_WIFI_ERROR, STATE_CURRENT));
    // printf("[%d] STATE_INTERNET_CONNECTED\n", IS_BITS(STATE_INTERNET_CONNECTED, STATE_CURRENT));
    // printf("[%d] STATE_STA_CONNECTING\n", IS_BITS(STATE_STA_CONNECTING, STATE_CURRENT));
    // printf("[%d] STATE_STA_CONNECTED\n", IS_BITS(STATE_STA_CONNECTED, STATE_CURRENT));
    // printf("[%d] STATE_STA_FAIL\n", IS_BITS(STATE_STA_FAIL, STATE_CURRENT));

    config_print();
    printf("\nWeb Files:\n");
    for (int i = 0; i < ((sizeof web_files) / sizeof(webfs_t)); i++)
    {
        const webfs_t *f = &web_files[i];

        printf("\t%d, %s, %s %s\n", i, f->name, f->type, f->gzip ? "(gzip)" : "");
    }
    return 0;
}

// static void broadcast_state()
// {
//     char msg[2] = {WS_MSG_ID_STATE, 0x00};
//     while (1)
//     {
//         xEventGroupWaitBits(APP_STATE, 0xff, 1, 0, 10000 / portTICK_PERIOD_MS);
//         msg[1] = (char) STATE_CURRENT;
//         ws_sendframe_bin(NULL, msg, 2);
//     }
// }

void app_main()
{

    wifi_init_config_t init_config = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    APP_STATE = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_mac_init());

    tcpip_adapter_init();

    ESP_ERROR_CHECK(esp_wifi_init(&init_config));

    config_load();

    ESP_ERROR_CHECK_WITHOUT_ABORT(app_wifi_start());

    web_server_main();

    initialize_terminal();

    // todo: load last state from storage
    actuator_setup(config.switch_values);

    const esp_console_cmd_t dump_cmd = {
        .command = "dump",
        .help = "System dump info",
        .hint = NULL,
        .func = &cmd_dump,
        .argtable = NULL};

    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_console_cmd_register(&dump_cmd));
    // xTaskCreate(&broadcast_state, "bcastState", 768, NULL, 2, NULL);
}
