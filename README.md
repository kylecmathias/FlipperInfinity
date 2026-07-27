# Flipper Infinity - Embedded Multi-Protocol Analysis Device
<img width="3024" height="4032" alt="20260522_135707" src="https://github.com/user-attachments/assets/d2a83646-06da-43d4-baa3-0bdc26a2cb9f" />

*Ongoing — Hardware complete, PCB designed, firmware in active development*

A handheld multi-protocol signal analysis device built on a custom ESP32 PCB, integrating NFC/RFID, sub-GHz RF, infrared, and WiFi into a single embedded platform with an on-device touchscreen interface.

---

## Hardware

| Module | Protocol | Function |
|---|---|---|
| PN532 | SPI | NFC/RFID read/write/emulate |
| CC1101 | SPI | Sub-GHz RF (433MHz) |
| IR Transmitter | GPIO | Infrared signal transmission |
| IR Receiver | GPIO | Infrared signal capture |
| Arduino Nano ESP32 | — | Main controller, WiFi, promiscuous packet capture |
| ILI9341 TFT Touchscreen | SPI | On-device UI (XPT2046 touch controller) |
| NiMH AA x6 | — | 7.2V battery pack, onboard voltage monitoring |

Custom 2-layer PCB designed in KiCad integrating all modules on a single board with a shared SPI bus, dedicated power rails, and RF-aware layout.

---

## PCB

Designed from scratch in KiCad. Two-layer board with:
- Shared SPI bus (SCK/MOSI/MISO) daisy-chained across TFT, PN532, and CC1101
- Dedicated 1mm VCC and GND power rails
- 100nF decoupling capacitors on all RF modules
- 10µF bulk capacitor on the ESP32 3V3 rail
- Ground pour on both copper layers
- PN532 antenna keepout zone to preserve NFC read range
- CC1101 module positioned at board edge for unobstructed antenna radiation

<img width="539" height="771" alt="image" src="https://github.com/user-attachments/assets/6acbae4a-2d81-46dd-b218-9958b169d4bd" />



### Pin Mapping

| Signal | ESP32 Pin | Connected To |
|---|---|---|
| SCK | D13 | TFT, PN532, CC1101 (shared SPI bus) |
| MOSI | D11 | TFT, PN532, CC1101 (shared SPI bus) |
| MISO | D12 | TFT, PN532, CC1101 (shared SPI bus) |
| TFT_SS | A2 | TFT display CS |
| T_SS | A4 | TFT touch CS |
| TFT_RST | D8 | TFT reset |
| DC | D9 | TFT data/command |
| LED | D10 | TFT backlight |
| T_IRQ | D2 | Touch interrupt |
| PN532_SS | A5 | PN532 CS |
| PN532_IRQ | D5 | PN532 interrupt |
| PN532_RST0 | D6 | PN532 reset |
| CC1101_SS | A7 | CC1101 CS |
| CC1101_GDO0 | D3 | CC1101 packet interrupt |
| IR_TX_DAT | D1 | IR transmitter |
| IR_RX_DAT | D0 | IR receiver |
| BATTERY_LVL | A3 | Battery voltage ADC (150k/100k divider) |
| BATTERY_SRC | D7 | USB/battery source detect (10k/20k divider) |
| VIN | VIN | 7.2V NiMH battery pack |

---

## Firmware

Written in C++ using ESP-IDF with FreeRTOS. Each protocol runs as an isolated task, scheduled cooperatively to avoid resource contention across shared buses.

- **Multi-protocol scheduling** — FreeRTOS tasks isolate NFC, RF, IR, and WiFi logic
- **Bus management** — SPI peripherals share the ESP32's interface without conflicts via chip select
- **Touchscreen UI** — LVGL-based interface built with SquareLine Studio for on-device workflow control
- **WiFi** — promiscuous mode packet capture for raw 802.11 frame analysis

---

## Planned Features

### WiFi
Network scanning, packet monitoring, device inventory, deauthentication, file server, MAC spoofing, captive portal, beacon spam

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

## Repository Structure

```
FlipperInfinity/
├── include/                # Header files to include (.hpp)
├── src/                    # Firmware source (.cpp, ESP-IDF) and build files (CMakeLists, idf_component)
│   ├── ui/                 # UI files generated with EEZ Studio (.c, .h)
├── hardware/
│   ├── schematic/          # KiCad schematic (.kicad_sch)
│   ├── pcb/                # KiCad PCB layout (.kicad_pcb)
│   └── gerbers/
│       └── v1.0/           # Manufacturing files for JLCPCB
├── partitions.csv          # ESP32s3 partitions
├── sdkconfig.defaults      # Configuration overrides
├── platformio.ini          # Platformio configuration
└── README.md
```

---

## Tech Stack

ESP32 · ESP-IDF · FreeRTOS · LVGL · SquareLine Studio · C++ · SPI · PN532 · CC1101 · KiCad
