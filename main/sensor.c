#include "sensor.h"
#include "config.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "terminal.h"
#include "esp_log.h"
#include "web-socket-handler.h"

#define SENSOR_PIN_S0 GPIO_NUM_4
#define SENSOR_PIN_S1 GPIO_NUM_5
#define SENSOR_PIN_S2 GPIO_NUM_15

static struct
{
    char msgId;
    char len; // also for padding
    uint16_t values[APP_SENSOR_COUNT];
} sensor_msg = {
    .msgId = WS_MSG_ID_SENSOR,
    .len = APP_SENSOR_COUNT & 0xff,
    .values = {0}};

static struct
{
    struct arg_int *num;
    struct arg_end *end;
} sensor_args;

static void read_sensors()
{
    for (;;)
    {
        for (int i = 0; i < APP_SENSOR_COUNT; i++)
        {
            // skip if disabled
            if ((config.sensor_status & (1 << i)) == 0)
                continue;
            gpio_set_level(SENSOR_PIN_S0, (i & 0b001) > 0);
            gpio_set_level(SENSOR_PIN_S1, (i & 0b010) > 0);
            gpio_set_level(SENSOR_PIN_S2, (i & 0b100) > 0);
            // vTaskDelay(1000 / portTICK_PERIOD_MS);
            adc_read(&sensor_msg.values[i]);
        }
        vTaskDelay(config.sensor_delay / portTICK_PERIOD_MS);
        // broadcast to all connected ws client
        ws_sendframe_bin(NULL, (void *)&sensor_msg, sizeof(sensor_msg));
    }
}

static int sensor_cmd(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&sensor_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, sensor_args.end, argv[0]);
        return 1;
    }

    printf("Sensor:\n");
    for (int i = 0; i < APP_SENSOR_COUNT; i++)
    {
        // skip if disabled
        if ((config.sensor_status & ((1 << i) & 0xff)) == 0)
            continue;
        printf("\t%d: %u\n", i, sensor_msg.values[i]);
    }

    return 0;
}

void sensor_setup()
{
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1 << SENSOR_PIN_S0) | (1 << SENSOR_PIN_S1) | (1 << SENSOR_PIN_S2);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;

    ESP_ERROR_CHECK(gpio_config(&io_conf));
    gpio_set_level(SENSOR_PIN_S0, 0);
    gpio_set_level(SENSOR_PIN_S1, 0);
    gpio_set_level(SENSOR_PIN_S2, 0);
    adc_config_t cfg = {
        .mode = ADC_READ_TOUT_MODE,
        .clk_div = 8,
    };

    ESP_ERROR_CHECK(adc_init(&cfg));

    // web_socket_add_handler(0, &on_ws_client);
    // web_socket_add_handler(WS_MSG_ID_SENSOR, &on_ws_get);
    // register terminal command
    sensor_args.num = arg_int0(NULL, NULL, "<n>", "Num (1 - 8, 0 = all)");
    sensor_args.end = arg_end(1);

    const esp_console_cmd_t cmd = {
        .command = "sens",
        .help = "Get sensor value",
        .hint = NULL,
        .func = &sensor_cmd,
        .argtable = &sensor_args,
    };

    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));

    xTaskCreate(&read_sensors, "sens", 1024, NULL, 1, NULL);
}
