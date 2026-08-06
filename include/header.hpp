#pragma once

#include <stdio.h>
#include <sys/stat.h>

#include <string>
#include <atomic>
#include <optional>
#include <array>
#include <format>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>

#include <driver/spi_master.h>
#include <driver/gpio.h>
#include <esp_littlefs.h>
#include <esp_heap_caps.h>
#include <esp_random.h>
#include <esp_timer.h>
#include <esp_private/periph_ctrl.h>

#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>

#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_ili9341.h>
#include <esp_lcd_touch_xpt2046.h>
#include <esp_lvgl_port.h>

#include <nvs_flash.h>
#include <esp_mac.h>
#include <esp_netif.h>
#include <esp_event.h>
#include <esp_wifi_default.h>
#include <esp_wifi.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_netif_net_stack.h>
#include <lwip/sockets.h>
#include <lwip/etharp.h>
#include <lwip/netif.h>
#include <lwip/ip4_addr.h>
#include <mdns.h>

#include <esp_bt.h>
#include <esp_nimble_hci.h>
#include <nimble/nimble_port.h>
#include <nimble/nimble_port_freertos.h>
#include <host/ble_hs.h>
#include <host/ble_gap.h>
#include <host/ble_hs_adv.h>
#include <host/util/util.h>
#include <services/gap/ble_svc_gap.h>
#include <pn532_driver.hpp>

#include <rom/ets_sys.h>

#include <driver/rmt_tx.h>
#include <driver/rmt_rx.h>

#include "helpers.hpp"

inline constexpr const char* LITTLEFS_MOUNT = "/flipperinf";
inline constexpr const char* LITTLEFS_PART_LABEL = "storage";
inline constexpr size_t FILENAME_LEN = 64;
inline constexpr size_t FILEPATH_LEN = 128;

inline constexpr uint8_t HIGH = 1;
inline constexpr uint8_t LOW = 0;

inline constexpr const char* DISPLAY_HOSTNAME = "iPhone";
inline constexpr size_t HOSTNAME_LEN = 32;

inline constexpr size_t LINE_BUFFER_LEN = 128;

inline constexpr spi_host_device_t LCD_HOST = SPI2_HOST;
inline constexpr uint64_t LCD_PIXEL_CLK = (40 * 1000 * 1000);
inline constexpr uint64_t LCD_TOUCH_CLK = (2 * 1000 * 1000);
inline constexpr uint16_t LCD_H_RES = 240;
inline constexpr uint16_t LCD_V_RES = 320;
inline constexpr float LCD_X_MULTIPLIER = 1.00f;
inline constexpr float LCD_Y_MULTIPLIER = 1.00f;
inline constexpr int16_t LCD_X_OFFSET = 0;
inline constexpr int16_t LCD_Y_OFFSET = 15;
inline constexpr uint16_t Y_DRIFT_END = 199;
inline constexpr uint16_t X_DRIFT_START = 320;
inline constexpr uint8_t ADC_SAMPLES = 8;
inline constexpr float V_MAX = 3.3f;
inline constexpr uint8_t ADC_RES = 12;
inline constexpr uint16_t ADC_MAX = (1UL << ADC_RES) - 1;
inline constexpr uint16_t BATT_ADC_MAX = ADC_MAX;
inline constexpr uint16_t BATT_ADC_MIN = 2.4f * ADC_MAX / V_MAX;

inline constexpr size_t MAX_SCAN_NETWORKS = 40;
inline constexpr size_t MAX_INVENTORY_DEVICES = 256;
inline constexpr size_t MAC_STR_LEN = 18;
inline constexpr size_t MANUFACTURER_STR_LEN = 32;
inline constexpr int8_t RSSI_MIN = -100;
inline constexpr int8_t RSSI_MAX = -20;
inline constexpr size_t MAX_UUIDS16 = 4;

inline constexpr spi_host_device_t PN532_HOST = SPI2_HOST;
inline constexpr uint32_t PN532_CLK = 2500000;

namespace GPIO_PINS {
    static constexpr gpio_num_t NONE = GPIO_NUM_NC;

    static constexpr gpio_num_t SCK = GPIO_NUM_48; //D13
    static constexpr gpio_num_t MOSI = GPIO_NUM_38; //D11
    static constexpr gpio_num_t MISO = GPIO_NUM_47; //D12

    static constexpr gpio_num_t TFT_SS = GPIO_NUM_3; //A2
    static constexpr gpio_num_t TFT_DC = GPIO_NUM_18; //D9
    static constexpr gpio_num_t TFT_RST = GPIO_NUM_17; //D8
    static constexpr gpio_num_t TFT_BKL = GPIO_NUM_21; //D10 but rn is on 3V3

    static constexpr gpio_num_t TOUCH_SS = GPIO_NUM_11; //A4
    static constexpr gpio_num_t TOUCH_IRQ = GPIO_NUM_5; //D2
    
    static constexpr gpio_num_t CC1101_SS = GPIO_NUM_14; //A7
    static constexpr gpio_num_t CC1101_GDO0 = GPIO_NUM_6; //A1

    static constexpr gpio_num_t PN532_SS = GPIO_NUM_12; //A5
    static constexpr gpio_num_t PN532_IRQ = GPIO_NUM_8; //D5
    static constexpr gpio_num_t PN532_RST = GPIO_NUM_9; //D6

    static constexpr gpio_num_t IR_RX = GPIO_NUM_44; //D0
    static constexpr gpio_num_t IR_TX = GPIO_NUM_43; //D1

    static constexpr gpio_num_t BATT_SRC = GPIO_NUM_10; //D7
    static constexpr gpio_num_t BATT_LVL = GPIO_NUM_4; //A3
};

inline constexpr uint8_t BATTERY_LEVEL_MIN = 0;
inline constexpr uint8_t BATTERY_LEVEL_MAX = 100;