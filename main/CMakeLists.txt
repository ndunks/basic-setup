set(srcs,
    "main.c"
    "wifi.c"
    "config.c"
    "wifi_action.c"
    "actuator.c"
    )


if(CONFIG_APP_WITH_SENSOR)
    list(APPEND srcs "sensor.c")
endif()

if(CONFIG_APP_WITH_COMMANDS)
    list(APPEND srcs "terminal.c")
endif()

idf_component_register(
    SRCS "${srcs}"
    INCLUDE_DIRS "$(PROJECT_PATH)/include")
