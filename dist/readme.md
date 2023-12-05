# Cara Flash

## Install ESP Tool

- [Install Python 3](https://pip.pypa.io/en/stable/installation/)
- pip install esptool
- pip install setuptools

More info:
https://docs.espressif.com/projects/esptool/en/latest/esp8266/installation.html


## Jalankan Flasher
``` sh

esptool.py --port XXX --chip esp8266 --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size 1MB 0x0 bootloader.bin 0xf000 phy_init_data.bin 0x10000 app.bin 0x8000 partitions_singleapp.bin

```