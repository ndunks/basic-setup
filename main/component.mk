#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

$(call compile_only_if,$(CONFIG_APP_WITH_COMMANDS),terminal.o)

$(call compile_only_if,$(CONFIG_APP_ESP01_SUPPORT_LC),actuator_serial.o)
$(call compile_only_if_not,$(CONFIG_APP_ESP01_SUPPORT_LC),actuator.o)