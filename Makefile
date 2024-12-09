#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

#PROJECT_NAME := opensmarthome
PYTHON := python3
#COMPONENTS := esp8266 bootloader partition_table
#bootloader \
esptool_py \
partition_table \
log \
esp8266 \
freertos \
heap \
esp_common \
esp_ringbuf \
spi_flash \
app_update \
bootloader_support \
esp_event \
tcp_transport \
tcpip_adapter \
lwip \
vfs \
wpa_supplicant \
nvs_flash \
newlib \
pthread \
esp_gdbstub \
console \
mbedtls \
esp-tls \
http_parser \


EXCLUDE_COMPONENTS := \
mdns \
fatfs \
coap \
esp_http_server \
esp-wolfssl \
esp_http_client \
esp_https_ota \
freemodbus \
jsmn \
json \
libsodium \
mqtt \
openssl \
protobuf-c \
protocomm \
spi_ram \
spiffs \
wear_levelling \
wifi_provisioning \

# ifneq ($(CONFIG_APP_WITH_COMMANDS),y)
# EXCLUDE_COMPONENTS += commands
# endif

include $(IDF_PATH)/make/project.mk

web:
	cd $(PROJECT_PATH)/components/web && npm run build && node web.js
