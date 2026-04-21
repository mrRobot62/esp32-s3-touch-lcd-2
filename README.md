# ESP32-S3 Touch LCD 2 – Working Display + Touch Starter Project (Display driver : ST7789T3)

This project provides a clean, working, and easy-to-understand starting point for the ESP32-S3 Touch LCD 2.

While the hardware itself is interesting and powerful, there are still very few practical examples available that properly demonstrate how to use:

* the LCD display
* the capacitive touch controller
* stable initialization
* usable project structure
* a clean foundation for custom GUI development

Most official demos are either minimal, outdated, overly complex, or difficult to adapt for real projects. This repository was created to solve exactly that problem.  

What this project offers

✅ Working display initialization<br>
✅ Functional touch support<br>
✅ Clean and readable source code<br>
✅ Good base for LVGL / Arduino / custom UI projects<br>
✅ Easier to understand than vendor demo packages<br>
✅ Ready to extend for your own applications<br>

## Why this repository exists

Many developers buy this board because the hardware looks great — but quickly discover that getting display + touch running reliably is harder than expected.

This project saves that time by providing a tested starting point instead of forcing users to reverse-engineer vendor demos or search through forum posts.

## Ideal for

* Makers
* ESP32 developers
* UI / touchscreen projects
* Rapid prototyping
* Learning how this board works
* Building your own custom firmware

## Goal

To become a practical community reference project for anyone working with the ESP32-S3 Touch LCD 2.



**This demo is without camera integration**

## Display
<img width="951" alt="image" src="https://github.com/user-attachments/assets/48341bc4-64f1-41cd-ad30-bc6819e4698e" />

<img width="951" alt="image" src="https://github.com/user-attachments/assets/2908c70a-ffde-4a06-a817-3b773dec8d38" />

<img width="951" alt="image" src="https://github.com/user-attachments/assets/f74c02cf-123d-4389-b80e-1f93a696c2e7" />

### Pinout
<img width="951" alt="image" src="https://github.com/user-attachments/assets/0f019452-c372-494e-8679-4042d7e96835" />

### What's on board
<img width="951" alt="image" src="https://github.com/user-attachments/assets/aec6aff6-ad2a-48de-9e7c-a4e608373e4e" />

### Key features
- Equipped with ESP32-S3R8 Xtensa 32-bit LX7 dual-core processor, up to 240MHz main frequency
- Supports 2.4GHz Wi-Fi (802.11 b/g/n) and Bluetooth 5 (LE), with onboard antenna
- Built in 512KB of SRAM and 384KB ROM, with onboard 8MB PSRAM and an external 16MB Flash memory
- Type-C connector, improving device compatibility, easier to use
- Onboard 2inch capacitive touch display for clear color picture display, 240 × 320 resolution, 262K color
- Built-in ST7789T3 display driver and CST816D capacitive touch chip, using SPI and I2C communication respectively, effectively saving the IO resources
- Onboard QMI8658 6-axis IMU (3-axis accelerometer and 3-axis gyroscope)
- Onboard 3.7V MX1.25 Lithium battery recharge/discharge header
- Onboard USB Type-C port for power supply, program downloading, and debugging, more convenient for development use
- Onboard TF card slot for external TF card storage of pictures or files
- Adapting 22 × GPIO pins for flexible configuration of pin function
- Onboard camera interface, compatible with mainstream cameras such as OV2640 and OV5640 for image and video acquisition
