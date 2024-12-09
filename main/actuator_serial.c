#include "config.h"
#include "main.h"
#include "actuator.h"
#include "web-socket-handler.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_vfs_dev.h"
#include <freertos/timers.h>
#define BUF_SIZE 256

// LC Tech relay refs:
// - https://github.com/xoseperez/espurna/blob/62ad7da332f3f904ad8241a2b738be9820b196e4/code/espurna/relay.ino#L251
// - https://devices.esphome.io/devices/ESP-01-4-Channel-Relay-LC-Technology
// - https://templates.blakadder.com/LC-ESP01-4R-12V.html
static void on_ws_update(ws_cli_conn_t *client, const unsigned char *msg, uint64_t size, int type)
{
    actuator_update(msg[0] >> 4, msg[0] & 0b1111);
}

// void actuator_debug_mcu()
// {
//     //  Debug serial from other MCU, but no output?
//     uint8_t data[BUF_SIZE];
//     while (1)
//     {
//         int len = uart_read_bytes(UART_NUM_0, data, BUF_SIZE, 20 / portTICK_RATE_MS); // Read with 20ms timeout
//         if (len > 0)
//         {
//             // Optionally print the received data for debugging
//             // printf("Received %d bytes: %s\n", len, data);
//             // ws_sendframe(NULL, "DEBUG", 5, WS_FR_OP_TXT);
//             ws_sendframe(NULL, (char *)data, len, WS_FR_OP_TXT);
//         }
//         vTaskDelay(200 / portTICK_PERIOD_MS);
//     }
// }

void actuator_setup_async()
{
    // Wait relay MCU ready
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    uint8_t data[BUF_SIZE];
    // vTaskDelay(100 / portTICK_PERIOD_MS);
    // Wait any message from MCU, I found: AT+CIPSTO=360
    while (1)
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
        int len = uart_read_bytes(UART_NUM_0, data, BUF_SIZE, 20 / portTICK_RATE_MS); // Read with 20ms timeout
        if (len > 0)
        {
            ws_sendframe(NULL, (char *)data, len, WS_FR_OP_TXT);
            break;
        }
        // printf("AT\n");
    }
    printf("WIFI CONNECTED\n");
    printf("WIFI GOT IP\n");
    printf("AT+CIPMUX=1\n");
    printf("AT+CIPSERVER=1,8080\n");
    printf("AT+CIPSTO=360\n");

    int state = 0;
    unsigned char initial_value = config.switch_values;
    for (int i = 0; i < APP_SWITCH_COUNT; i++)
    {
        state = initial_value & (1 << i);

        if (state > 0)
        {
            actuator_update(i, state);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }
    // xTaskCreate(actuator_debug_mcu, "actuator_debug_mcu", 2048, NULL, 10, NULL);
    vTaskDelete(NULL);
}
void actuator_save()
{
    uint8_t current_value = config.switch_values;
    while (1)
    {
        vTaskDelay(1000 * 10 / portTICK_PERIOD_MS);
        if (current_value != config.switch_values)
        {
            config_save(NULL);
            current_value = config.switch_values;
            ws_sendframe(NULL, "SWITCH SAVED", 12, WS_FR_OP_TXT);
        }
    }
}

void actuator_setup(unsigned char initial_value)
{
    /* Disable buffering on stdin */
    // setvbuf(stdin, NULL, _IONBF, 0);
    /* Disable buffering on stdout */
    setvbuf(stdout, NULL, _IONBF, 0);

    uart_config_t uart_config = {
        .baud_rate = CONFIG_ESP_CONSOLE_UART_BAUDRATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
    };
    ESP_ERROR_CHECK(uart_param_config(CONFIG_ESP_CONSOLE_UART_NUM, &uart_config));

    /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK(uart_driver_install(CONFIG_ESP_CONSOLE_UART_NUM,
                                        256, 0, 0, NULL, 0));

    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);

    // printf("BOOT printf\n");
    // fwrite("BOOT fwrite\n", 12, 1, stdout);
    // write(STDOUT_FILENO,"BOOT stdout\n",12);

    xTaskCreate(actuator_setup_async, "actuator_setup", 4096, NULL, 50, NULL);
    // xTaskCreate(actuator_debug_mcu, "actuator_debug_mcu", 2048, NULL, 10, NULL);
    xTaskCreate(actuator_save, "actuator_store_config", 2048, NULL, 10, NULL);

    web_socket_add_handler(WS_MSG_ID_ACTUATOR, &on_ws_update);
}

#if CONFIG_APP_ESP01_SUPPORT_LC // Serial
// void custom_printf(char * buffer, const char *fmt, ...) {
//     va_list args;

//     va_start(args, fmt);
//     vsprintf(buffer, fmt, args);
//     va_end(args);
// }
void actuator_update(int switch_id, int value)
{
    /* Relay 1 state 1: A0 01 01 A2 | A0 + 01 + 01
     * Relay 1 state 0: A0 01 00 A1 | A0 + 01 + 00
     * Relay 2 state 1: A0 02 01 A3 | A0 + 02 + 01
     * Relay 2 state 0: A0 02 00 A2 | A0 + 02 + 00
     * Relay 3 state 1: A0 03 01 A4 | A0 + 03 + 01
     * Relay 3 state 0: A0 03 00 A3 | A0 + 03 + 00
     * Relay 4 state 1: A0 04 01 A5 | A0 + 04 + 01
     * Relay 4 state 0: A0 04 00 A4 | A0 + 04 + 00
     */
    // Convert to one or zero
    value = value > 0 ? 1 : 0;
    
    // char data[BUF_SIZE];
    // custom_printf(data, "INIT %d %d", switch_id, value);
    // ws_sendframe(NULL, (char *)data, 8, WS_FR_OP_TXT);

    // ID is start from 1, switch_id is zero based
    const unsigned char lc_command[] = {0xA0, switch_id + 1, value, 0xA1 + switch_id + value};
    // printf("Update serial %d %d\n", switch_id, value);
    // Send to serial
    write(STDOUT_FILENO, lc_command, 4);
    // fflush(stdout);
    // fflush(stdout);

    config.switch_values = (config.switch_values & ~((uint8_t)1 << switch_id)) | ((uint8_t)value << switch_id);

    // Broadcast to other client
    const char ws_msg[2] = {WS_MSG_ID_ACTUATOR, config.switch_values};
    ws_sendframe(NULL, ws_msg, 2, WS_FR_OP_BIN);
}
#endif