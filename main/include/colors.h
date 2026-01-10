#ifndef APP_COLORS_H
#define APP_COLORS_H

#include "esp_log.h"
#if CONFIG_LOG_COLORS
#define COLOR_OK(str) LOG_COLOR(LOG_COLOR_GREEN) str LOG_RESET_COLOR
#define COLOR_WARN(str) LOG_COLOR(LOG_COLOR_BROWN) str LOG_RESET_COLOR
#define COLOR_ERR(str) LOG_COLOR(LOG_COLOR_RED) str LOG_RESET_COLOR
#define COLOR_INFO(str) LOG_COLOR(LOG_COLOR_CYAN) str LOG_RESET_COLOR
#else
#define COLOR_OK(str) str
#define COLOR_WARN(str) str
#define COLOR_ERR(str) str
#define COLOR_INFO(str) str
#endif

#endif