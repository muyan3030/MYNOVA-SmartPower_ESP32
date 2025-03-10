<div id="top">
<p align="center">
  <h1 align="center">MYNOVA-SmartPower</h1>
</p>
</div>


# Introduction
This is a modified ESP32 version from https://github.com/Tomosawa/MYNOVA-SmartPower

Corresponding PCB https://oshwhub.com/muyan2020/dell-fu-wu-qi-dian-yuan-qu-dian-ban-esp32

## Supported Chips

- ESP32

## Tested PSU


## Directory Structure

- MYNOVA_POWER: Firmware code for the chip.
- ESPVue: Web-related application code.

## Compilation Environment

- Firmware: Arduino IDE 1.8.19
- Web: Node.js v20.15.1

## Firmware Dependencies

- Install ESP32 official development board
- Install U8g2lib.
- Install ESPAsyncWebServer server component.
- Install ArduinoJson library.

## Firmware Flashing Method

1. Depending on the flash size of your ESP32-S3 chip (16MB or 8MB), download the corresponding bin file from the [Release](https://github.com/Tomosawa/MYNOVA-SmartPower/releases) page.
2. Download the official [flash download tools](https://www.espressif.com/en/support/download/other-tools) or use the [esptool.py](https://github.com/espressif/esptool) command line tool.
3. Select the appropriate `.bin` firmware file and set the flash offset address to `0x0000`.
4. Do not check the `DoNotChgBin` option, then click start to begin the flashing process.

![flashdownloadtools](images/flashtools.png)

## Filesystem Image

I have generated a littlefs image file, which can be uploaded using the following method, otherwise the web interface will not be opened

https://blog.csdn.net/armcsdn/article/details/140730648

## Contribution Guidelines

Contributions to this project are welcome. Please follow these guidelines:
- Fork the repository and create your feature branch.
- Commit your changes and push to your fork.
- Submit a Pull Request.

## Copyright Statement

This project is prohibited for any commercial use and is intended for learning and DIY purposes only.

## License [LICENSE](LICENSE)

This project is licensed under the [GPLv3](LICENSE). [GPLv3 (GNU General Public License version 3)](LICENSE) is a free, open-source software license that guarantees users the freedoms to run, study, share, and modify the software.
The complete text of the [GPLv3](LICENSE) license is included in the [LICENSE](LICENSE) file of this project. Before using, modifying, or distributing the code of this project, make sure you have read and understood the entire [GPLv3](LICENSE) license.

## Acknowledgments

- [Multibutton](https://github.com/0x1abin/MultiButton)
