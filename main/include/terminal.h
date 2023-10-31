#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "FreeRTOS.h"
#include "freertos/task.h"

    volatile TaskHandle_t terminal_task_handle;

    void initialize_terminal(void);

#ifdef __cplusplus
}
#endif
