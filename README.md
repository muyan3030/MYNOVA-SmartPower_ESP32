<div id="top">
<p align="center">
  <h1 align="center">MYNOVA-SmartPower</h1>
</p>
</div>


# Introduction
This is a modified ESP32 version from https://github.com/Tomosawa/MYNOVA-SmartPower

**The description of the original project will not be repeated here. If necessary, you can view the original project**

Corresponding PCB https://oshwhub.com/muyan2020/dell-fu-wu-qi-dian-yuan-qu-dian-ban-esp32

## Update

20250317

Updated support for WebOTA

mynova_esp32_V1.1.2.bin

littlefs_1.1.2.bin

20250311

Updated support for HA

mynova_esp32_V1.1.1.bin

littlefs_1.1.1.bin

## Supported Chips

- ESP32

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
