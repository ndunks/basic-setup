set(srcs,
    "main.c"
    "wifi.c"
    "config.c"
    "wifi_action.c"
    "actuator.c"
    )


ifeq($(CONFIG_APP_WITH_SENSOR),y)
    list(APPEND srcs "sensor.c")
endif()

ifeq($(CONFIG_APP_WITH_COMMANDS),y)
    list(APPEND srcs "terminal.c")
endif()

idf_component_register(
    SRCS "${srcs}"
    INCLUDE_DIRS "$(PROJECT_PATH)/include")
