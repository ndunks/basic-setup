#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_event.h"
#include "esp_err.h"
#include "esp_console.h"
#include "state.h"
#include "terminal.h"
#include "wifi.h"

static int cmd_state(int argc, char **argv)
{
    EventBits_t bits = STATE();
    printf("[%d] STA_CONNECTING\n", (bits & STATE_STA_CONNECTING) == STATE_STA_CONNECTING);
    printf("[%d] STA_CONNECTED\n", (bits & STATE_STA_CONNECTED) == STATE_STA_CONNECTED);
    printf("[%d] STA_FAIL\n", (bits & STATE_STA_FAIL) == STATE_STA_FAIL);
    printf("[%d] STA_HAS_IP\n", (bits & STATE_STA_HAS_IP) == STATE_STA_HAS_IP);
    printf("[%d] AP_STARTED\n", (bits & STATE_AP_STARTED) == STATE_AP_STARTED);
    printf("[%d] WIFI_DISABLED\n", (bits & STATE_WIFI_DISABLED) == STATE_WIFI_DISABLED);
    printf("[%d] WIFI_ERROR\n", (bits & STATE_WIFI_ERROR) == STATE_WIFI_ERROR);
    printf("[%d] INTERNET_CONNECTED\n", (bits & STATE_INTERNET_CONNECTED) == STATE_INTERNET_CONNECTED);
    return 0;
}

void app_main()
{
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    APP_STATE = xEventGroupCreate();
    // initialize_nvs();

    ESP_ERROR_CHECK_WITHOUT_ABORT(app_wifi_start());

    initialize_terminal();

    const esp_console_cmd_t state_cmd = {
        .command = "state",
        .help = "System state info",
        .hint = NULL,
        .func = &cmd_state,
        .argtable = NULL};

    ESP_ERROR_CHECK(esp_console_cmd_register(&state_cmd));
}
