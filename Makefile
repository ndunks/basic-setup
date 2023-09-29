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
mbedtls \
esp-tls \
http_parser \
pthread \
esp_gdbstub \
console \
mdns \


EXCLUDE_COMPONENTS := \
coap \
esp-wolfssl \
esp_http_client \
esp_http_server \
esp_https_ota \
fatfs \
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

include $(IDF_PATH)/make/project.mk
