#include "driver/gpio.h"
#include "actuator.h"
#include "main.h"
#include "config.h"
#include "terminal.h"
#include "web-socket-handler.h"

#if CONFIG_APP_ESP01_SUPPORT
#include "driver/uart.h"
// SHCP (Shift Register Clock Input)
#define ACTUATOR_PIN_CLOCK GPIO_NUM_3
// DS   (Serial Data Input)
#define ACTUATOR_PIN_DS GPIO_NUM_2
// STCP (Storage Register Clock Input) // RXD
#define ACTUATOR_PIN_STCP GPIO_NUM_0
#else
// SHCP (Shift Register Clock Input)
#define ACTUATOR_PIN_CLOCK GPIO_NUM_13
// DS   (Serial Data Input)
#define ACTUATOR_PIN_DS GPIO_NUM_16
// STCP (Storage Register Clock Input)
#define ACTUATOR_PIN_STCP GPIO_NUM_12
#endif
static struct
{
    struct arg_int *byte;
    struct arg_end *end;
} actuator_args;

static int actuator_cmd(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&actuator_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, actuator_args.end, argv[0]);
        return 1;
    }

    actuator_update(actuator_args.byte->ival[0] & 0xffu);
    printf("%0x\n", config.switch_values);
    return 0;
}

static void on_ws_update(ws_cli_conn_t *client, const unsigned char *msg, uint64_t size, int type)
{
    actuator_update(msg[0]);
}

void actuator_setup(unsigned char initial_value)
{

#ifdef CONFIG_APP_ESP01_SUPPORT
    uart_disable_rx_intr(UART_NUM_0);
    // set pin function support for ESP01
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_GPIO3);
#endif

    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
#ifdef ACTUATOR_PIN_ENABLE
    io_conf.pin_bit_mask = (1 << ACTUATOR_PIN_CLOCK) | (1 << ACTUATOR_PIN_DS) | (1 << ACTUATOR_PIN_STCP) | (1 << ACTUATOR_PIN_ENABLE);
#else
    io_conf.pin_bit_mask = (1 << ACTUATOR_PIN_CLOCK) | (1 << ACTUATOR_PIN_DS) | (1 << ACTUATOR_PIN_STCP);
#endif
    io_conf.pull_down_en = 1;
    io_conf.pull_up_en = 0;

    gpio_config(&io_conf);
    gpio_set_level(ACTUATOR_PIN_CLOCK, 0);
    gpio_set_level(ACTUATOR_PIN_DS, 0);
    gpio_set_level(ACTUATOR_PIN_STCP, 0);
#ifdef ACTUATOR_PIN_ENABLE
    gpio_set_level(ACTUATOR_PIN_ENABLE, 0);
#endif
    actuator_update(initial_value);

    // web_socket_add_handler(0, &on_ws_client);
    web_socket_add_handler(WS_MSG_ID_ACTUATOR, &on_ws_update);
    // register terminal command
    actuator_args.byte = arg_int0(NULL, NULL, "<n>", "Byte");
    actuator_args.end = arg_end(1);

    const esp_console_cmd_t cmd = {
        .command = "act",
        .help = "Set actuator value",
        .hint = NULL,
        .func = &actuator_cmd,
        .argtable = &actuator_args,
    };

    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

void actuator_update(unsigned char value)
{
    // static uint32_t delay = (1);

    for (int8_t i = 7; i >= 0; i--)
    {
        // Bit data
        // Relay module is ACTIVE LOW, 1 mean OFF, 0 mean ON
        gpio_set_level(ACTUATOR_PIN_DS, !((value >> i) & 1));

        // Clock pulse
        gpio_set_level(ACTUATOR_PIN_CLOCK, 1);
        // vTaskDelay(500 / portTICK_PERIOD_MS);
        gpio_set_level(ACTUATOR_PIN_CLOCK, 0);
        // vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    // latch
    gpio_set_level(ACTUATOR_PIN_STCP, 1);
    // vTaskDelay(500 / portTICK_PERIOD_MS);
    gpio_set_level(ACTUATOR_PIN_STCP, 0);
    // vTaskDelay(500 / portTICK_PERIOD_MS);
    config.switch_values = value;
    config_save(NULL);

    // Broadcast to other client
    const char ws_msg[2] = {WS_MSG_ID_ACTUATOR, value};
    ws_sendframe(NULL, ws_msg, 2, WS_FR_OP_BIN);
}