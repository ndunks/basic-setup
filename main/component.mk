#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

COMPONENT_OBJS := actuator.o config.o main.o wifi_action.o wifi.o
CFLAGS += -DBUILD_DATE="\"$(shell date)\""

ifeq ($(CONFIG_APP_WITH_COMMANDS),y)
COMPONENT_OBJS += terminal.o
endif
ifeq ($(CONFIG_APP_WITH_SENSOR),y)
COMPONENT_OBJS += sensor.o
endif