#include "esp_console.h"
#include "driver/gpio.h"
#include "actuator.h"
#include "argtable3/argtable3.h"
#include "FreeRTOS.h"
#include "freertos/task.h"

// GPIO12 -> SHCP (Shift Register Clock Input)
#define ACTUATOR_PIN_CLOCK GPIO_NUM_12
// GPIO13 -> DS   (Serial Data Input)
#define ACTUATOR_PIN_DS GPIO_NUM_13
// GPIO14 -> STCP (Storage Register Clock Input)
#define ACTUATOR_PIN_STCP GPIO_NUM_14

void actuator_setup(unsigned char initial_value)
{

    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1 << ACTUATOR_PIN_CLOCK) | (1 << ACTUATOR_PIN_DS) | (1 << ACTUATOR_PIN_STCP);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;

    gpio_config(&io_conf);
    gpio_set_level(ACTUATOR_PIN_CLOCK, 0);
    gpio_set_level(ACTUATOR_PIN_DS, 0);
    gpio_set_level(ACTUATOR_PIN_STCP, 0);
    vTaskDelay(1);
    actuator_update(initial_value);
}

void actuator_update(unsigned char value)
{

    for (int8_t i = 7; i >= 0; i--)
    {
        // Bit data
        gpio_set_level(ACTUATOR_PIN_DS, (value >> i) & 1);

        // Clock pulse
        gpio_set_level(ACTUATOR_PIN_CLOCK, 1);
        vTaskDelay(1);
        gpio_set_level(ACTUATOR_PIN_CLOCK, 0);
        vTaskDelay(1);
    }
    // latch
    gpio_set_level(ACTUATOR_PIN_STCP, 1);
    vTaskDelay(1);
    gpio_set_level(ACTUATOR_PIN_STCP, 0);
    vTaskDelay(1);
}

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
    return 0;
}

void actuator_register_command()
{
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
