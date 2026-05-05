# Flipper Infinity - Embedded Multi-Protocol Analysis Device
*Ongoing — Hardware complete, firmware in active development*

A handheld multi-protocol signal analysis device built on a custom ESP32 PCB, integrating NFC/RFID, sub-GHz RF, infrared, and WiFi into a single embedded platform with an on-device touchscreen interface.

---

## Hardware

| Module | Protocol | Function |
|---|---|---|
| PN532 | I2C | NFC/RFID read |
| CC1101 | SPI | Sub-GHz RF (315/433/868/915 MHz) |
| IR Transmitter | GPIO | Infrared signal transmission |
| IR Receiver | GPIO | Infrared signal capture |
| ESP32 | — | Main controller, WiFi, promiscuous packet capture |
| TFT Touchscreen | SPI | On-device UI |

Custom PCB designed to integrate all modules cleanly without a rats nest of breakout boards.

---

## Firmware

Written in C++ using ESP-IDF with FreeRTOS. Each protocol runs as an isolated task, scheduled cooperatively to avoid resource contention across shared buses.

- **Multi-protocol scheduling** — FreeRTOS tasks isolate NFC, RF, IR, and WiFi logic
- **Bus management** — I2C and SPI peripherals share the ESP32's interfaces without conflicts
- **Touchscreen UI** — LVGL-based interface built with SquareLine Studio for on-device workflow control
- **WiFi** — promiscuous mode packet capture for raw 802.11 frame analysis

---

## Planned Features

### WiFi
Network scanning, packet monitoring, device inventory, file server, MAC spoofing

### Bluetooth
BLE device scanner, packet sniffer, device logger

### NFC
Read, write, emulate, sniff — supports Mifare Classic cards

### RFID
Read, write, emulate

### IR
Transmit, receive, universal remote, signal replay

### RF (CC1101)
Frequency analyzer, listen, decode, replay, transmit

---

## Tech Stack

ESP32 · ESP-IDF · FreeRTOS · LVGL · SquareLine Studio · C++ · I2C · SPI · PN532 · CC1101
