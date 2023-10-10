# Setup

## Setup Toolchain (Compiler)

- [doc](https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/get-started/linux-setup.html)

```sh
wget https://dl.espressif.com/dl/xtensa-lx106-elf-gcc8_4_0-esp-2020r3-linux-amd64.tar.gz
tar -xzf xtensa-lx106-elf-gcc8_4_0-esp-2020r3-linux-amd64.tar.gz

```

## Setup SDK (Library)

- [doc](https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/get-started/index.html#get-started-get-esp-idf)

```sh
# Ubuntu 22
sudo apt-get install gcc git wget make libncurses-dev flex bison gperf python-is-python3 python3-serial
```

## Configure Project

```sh
PYTHON=python3 make menuconfig
```

# Device ID

- Efuse
- Internal flash chip ID
- Firmware Info + time stamp ?

# 1 MB OTA

| Name     | Type | SubType | Offset  | Size    | Offset D | Size D |
| -------- | ---- | ------- | ------- | ------- | -------- | ------ |
| nvs      | data | nvs     | 0x9000  | 0x4000  | 36 KB    | 16 KB  |
| otadata  | data | ota     | 0xd000  | 0x2000  | 52 KB    | 8 KB   |
| phy_init | data | phy     | 0xf000  | 0x1000  | 60 KB    | 4 KB   |
| ota_0    | 0    | ota_0   | 0x10000 | 0x70000 | 64 KB    | 448 KB |
| ota_1    | 0    | ota_1   | 0x80000 | 0x70000 | 512 KB   | 448 KB |
|          |      |         |         |         | TOTAL    | 924 KB |