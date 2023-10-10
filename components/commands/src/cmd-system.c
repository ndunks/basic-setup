#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_console.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "argtable3/argtable3.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "commands.h"
#include "nvs.h"
#include "esp_partition.h"

#ifdef CONFIG_FREERTOS_USE_STATS_FORMATTING_FUNCTIONS
#define WITH_TASKS_INFO 1
#endif

static const char *TAG = "cmd_system";

static void register_free();
static void register_info();
static void register_restart();
#if WITH_TASKS_INFO
static void register_tasks();
#endif

void register_system()
{
    register_free();
    register_info();
    register_restart();
#if WITH_TASKS_INFO
    register_tasks();
#endif
}

/* 'info' command */
static int get_info(int argc, char **argv)
{
    esp_chip_info_t info;
    esp_chip_info(&info);

    printf("IDF Version:%s\r\n", esp_get_idf_version());
    printf("Chip info:\r\n");
    printf("\tmodel:%s\r\n", info.model == CHIP_ESP8266 ? "ESP8266" : "Unknow");
    printf("\tcores:%d\r\n", info.cores);
    printf("\tfeature:%s%s%s%s%d%s\r\n",
           info.features & CHIP_FEATURE_WIFI_BGN ? "/802.11bgn" : "",
           info.features & CHIP_FEATURE_BLE ? "/BLE" : "",
           info.features & CHIP_FEATURE_BT ? "/BT" : "",
           info.features & CHIP_FEATURE_EMB_FLASH ? "/Embedded-Flash:" : "/External-Flash:",
           spi_flash_get_chip_size() / (1024 * 1024), " MB");
    printf("\trevision number:%d\r\n", info.revision);

    esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_DATA,
                                                     ESP_PARTITION_SUBTYPE_ANY,
                                                     NULL);
    printf("\nData Partition Info:\n");
    /* has to be at least one app partition */
    if (it == NULL)
    {
        printf("\tNo data partition found\n");
    }

    bool is_data = false;
    nvs_stats_t stats = {0};
    esp_err_t err;
    while (it != NULL)
    {
        const esp_partition_t *p = esp_partition_get(it);
        is_data = strcmp(p->label, NVS_DEFAULT_PART_NAME) == 0;

        printf("%02x %10s%c %p %d KiB\n",
               p->subtype,
               p->label,
               is_data ? '*' : ' ',
               (void *)p->address,
               p->size / 1024);

        it = esp_partition_next(it);
    }
    printf("\nNVS Entries: ");
    if ((err = nvs_get_stats(NVS_DEFAULT_PART_NAME, &stats)) == ESP_OK)
    {
        printf("used: %d, free: %d, total: %d, namespaces: %d\n",
               stats.used_entries, stats.free_entries, stats.total_entries, stats.namespace_count);

        nvs_iterator_t it = nvs_entry_find(NVS_DEFAULT_PART_NAME, NULL, NVS_TYPE_ANY);
        while (it != NULL)
        {
            nvs_entry_info_t info;
            nvs_entry_info(it, &info);
            it = nvs_entry_next(it);
            printf("%16s %20s (%0x)\n", info.namespace_name, info.key, info.type);
        };
    }
    else
        printf("Get stats Failed: %s (%0x)\n", esp_err_to_name(err), err);

    return 0;
}

static void register_info()
{
    const esp_console_cmd_t cmd = {
        .command = "info",
        .help = "Get info of chip and SDK",
        .hint = NULL,
        .func = &get_info,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

/** 'restart' command restarts the program */

static int restart(int argc, char **argv)
{
    ESP_LOGI(TAG, "Restarting");
    esp_restart();
}

static void register_restart()
{
    const esp_console_cmd_t cmd = {
        .command = "restart",
        .help = "Software reset of the chip",
        .hint = NULL,
        .func = &restart,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

/** 'free' command prints available heap memory */

static int free_mem(int argc, char **argv)
{
    printf("Free: %d\r\nMin : %d\r\n",
           esp_get_free_heap_size(),
           heap_caps_get_minimum_free_size(MALLOC_CAP_32BIT));
    return 0;
}

static void register_free()
{
    const esp_console_cmd_t cmd = {
        .command = "free",
        .help = "Get the current size of free heap memory",
        .hint = NULL,
        .func = &free_mem,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

/** 'tasks' command prints the list of tasks and related information */
#if WITH_TASKS_INFO

static int tasks_info(int argc, char **argv)
{
    const size_t bytes_per_task = 40; /* see vTaskList description */
    char *task_list_buffer = malloc(uxTaskGetNumberOfTasks() * bytes_per_task);
    if (task_list_buffer == NULL)
    {
        ESP_LOGE(TAG, "failed to allocate buffer for vTaskList output");
        return 1;
    }
    fputs("Task Name\tStatus\tPrio\tHWM\tTask#", stdout);
#ifdef CONFIG_FREERTOS_VTASKLIST_INCLUDE_COREID
    fputs("\tAffinity", stdout);
#endif
    fputs("\n", stdout);
    vTaskList(task_list_buffer);
    fputs(task_list_buffer, stdout);
    free(task_list_buffer);
    return 0;
}

static void register_tasks()
{
    const esp_console_cmd_t cmd = {
        .command = "tasks",
        .help = "Get information about running tasks",
        .hint = NULL,
        .func = &tasks_info,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

#endif // WITH_TASKS_INFO
