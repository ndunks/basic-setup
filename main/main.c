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
#include "web-server.h"
#include "driver/gpio.h"

static int cmd_state(int argc, char **argv)
{
    EventBits_t bits = STATE();
    printf("STATE: %08x\n", bits);
    printf("STATE: %08x\n", STATE_CURRENT);
    printf("[%d] STATE_AP_STARTED\n", IS_BITS(STATE_AP_STARTED, STATE_CURRENT));
    printf("[%d] STATE_WIFI_ERROR\n", IS_BITS(STATE_WIFI_ERROR, STATE_CURRENT));
    printf("[%d] STATE_INTERNET_CONNECTED\n", IS_BITS(STATE_INTERNET_CONNECTED, STATE_CURRENT));
    printf("[%d] STATE_STA_CONNECTING\n", IS_BITS(STATE_STA_CONNECTING, STATE_CURRENT));
    printf("[%d] STATE_STA_CONNECTED\n", IS_BITS(STATE_STA_CONNECTED, STATE_CURRENT));
    printf("[%d] STATE_STA_FAIL\n", IS_BITS(STATE_STA_FAIL, STATE_CURRENT));
    return 0;
}

static void blink_test()
{
re:
    gpio_set_level(GPIO_NUM_12, 1);
    gpio_set_level(GPIO_NUM_13, 1);
    gpio_set_level(GPIO_NUM_14, 1);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    gpio_set_level(GPIO_NUM_12, 0);
    gpio_set_level(GPIO_NUM_13, 0);
    gpio_set_level(GPIO_NUM_14, 0);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    goto re;
}

void app_main()
{
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    APP_STATE = xEventGroupCreate();

    // todo: load last state from storage
    actuator_setup(0b11111111u);

    ESP_ERROR_CHECK_WITHOUT_ABORT(app_wifi_start());

    web_server_main();

    initialize_terminal();

    const esp_console_cmd_t state_cmd = {
        .command = "state",
        .help = "System state info",
        .hint = NULL,
        .func = &cmd_state,
        .argtable = NULL};

    ESP_ERROR_CHECK(esp_console_cmd_register(&state_cmd));

    //xTaskCreate(&blink_test, "blink", 1024, NULL, 2, NULL);
}
