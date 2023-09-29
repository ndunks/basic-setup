#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
    #include "FreeRTOS.h"
    #include "freertos/task.h"

    volatile TaskHandle_t terminal_task_handle;

    void initialize_terminal(void);

#ifdef __cplusplus
}
#endif
