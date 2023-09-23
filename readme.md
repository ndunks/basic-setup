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