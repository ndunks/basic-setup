#include "sensor.h"
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
    struct arg_int *num;
    struct arg_end *end;
} sensor_args;

static uint16_t sensor_read(int num)
{
    uint16_t value = 0;
    gpio_set_level(SENSOR_PIN_S0, (num & 0b001) > 0);
    gpio_set_level(SENSOR_PIN_S1, (num & 0b010) > 0);
    gpio_set_level(SENSOR_PIN_S2, (num & 0b100) > 0);
    // printf("V %i%i%i \n", num & 0b100, num & 0b010, num & 0b001);
    //vTaskDelay(1000 / portTICK_PERIOD_MS);
    adc_read(&value);
    return value;
}

static void read_sensors()
{
    uint16_t value = 0;
again:
    printf("Sensor:\n");
    for (int i = 0; i < 8; i++)
    {
        value = sensor_read(i);
        printf("\t%d: %u\n", i, value);
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);
    goto again;
}

static int sensor_cmd(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&sensor_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, sensor_args.end, argv[0]);
        return 1;
    }
    // read_sensors();
    return 0;
}

static void on_ws_get(ws_cli_conn_t *client, const unsigned char *msg, uint64_t size, int type)
{
    // sensor_update(msg[0]);
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
    web_socket_add_handler(WS_MSG_ID_SENSOR, &on_ws_get);
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

    xTaskCreate(&read_sensors, "sens", 1024, NULL, 2, NULL);
}
