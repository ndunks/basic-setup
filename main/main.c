/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_spi_flash.h"
#include "esp8266/gpio_struct.h"
#define LED_PIN 2

static void blink_task(void *null)
{
    uint32_t pin = 0;

    // output mode
    GPIO.enable_w1ts |= (0x1 << LED_PIN);

    while (1)
    {
        // &GPIO = 0x60000300
        printf("Tick %d %d %p %p %p \n", pin, soc_get_ccount(), (void *)&GPIO, (void *)&GPIO.out, (void *)&GPIO.status);
        fflush(stdout);
        // OFF
        GPIO.out_w1tc |= (0x1 << LED_PIN);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        // ON
        GPIO.out_w1ts |= (0x1 << LED_PIN);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    printf("Hello world!\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is ESP8266 chip with %d CPU cores, WiFi, ",
           chip_info.cores);
    printf("silicon revision %d, ", chip_info.revision);
    printf("%dMB flash\n", spi_flash_get_chip_size() / (1024 * 1024));
    xTaskCreate(&blink_task, "blink", 1024, NULL, 3, NULL);
}
