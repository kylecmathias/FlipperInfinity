#pragma once

#include <driver/spi_master.h>
#include <driver/i2c_master.h>
#include <driver/uart.h>
#include <driver/gpio.h>

#include <variant>

enum class PN532NFCType : uint8_t {
    UNKNOWN = 0,
    MIFARE_CLASSIC_1K,
    MIFARE_CLASSIC_MINI,
    MIFARE_CLASSIC_4K,
    MIFARE_ULTRALIGHT,
    MIFARE_ULTRALIGHT_C,
    MIFARE_ULTRALIGHT_EV1,
    NTAG213,
    NTAG215,
    NTAG216,
    MIFARE_PLUS_2K,
    MIFARE_PLUS_4K,
    MIFARE_DESFIRE
};

enum class PN532Key : uint8_t {
    KEY_A = 0x60,
    KEY_B = 0x61
};

namespace PN532Err {
    inline constexpr int16_t TIMEOUT = -2;
    inline constexpr int16_t INVALID_FRAME = -3;
    inline constexpr int16_t NO_SPACE = -4;
};

struct PN532UID {
    uint8_t uid[10];
    int8_t uid_length;
    uint8_t sak;
    PN532NFCType subtype;
    uint8_t _pad;
    uint16_t atqa;
    uint16_t block_size;
    uint16_t blocks_count;
};

struct PN532SPIConfig {
    spi_host_device_t host;
    gpio_num_t mosi;
    gpio_num_t miso;
    gpio_num_t sck;
    gpio_num_t cs;
};

struct PN532I2CConfig {
    i2c_port_t port;
    gpio_num_t sda;
    gpio_num_t scl;
    uint8_t addr;
};

struct PN532UARTConfig {
    uart_port_t port;
    gpio_num_t tx;
    gpio_num_t rx;
    uint32_t baud;
};

struct PN532Config {
    std::variant<PN532SPIConfig, PN532I2CConfig, PN532UARTConfig> interface;

    gpio_num_t rst;
    gpio_num_t irq;
};

class PN532 {
    private:
        PN532Config config;
        bool is_initialized;

        uint8_t tx_buffer[260]; 
        uint8_t rx_buffer[260];

        volatile TaskHandle_t waiting_task = nullptr;
        spi_device_handle_t spi_dev_handle;
        i2c_master_bus_handle_t i2c_bus_handle;
        i2c_master_dev_handle_t i2c_dev_handle;

        uint8_t command = 0;

    public:
        PN532(PN532Config& cfg);
        ~PN532();
        PN532(const PN532&) = delete;

        void wakeup();
        bool sam_config();
        bool set_passive_activation_retries(uint8_t max_retries);
        bool get_firmware_version(uint32_t& version);
        bool read_targets(PN532UID* buffer, size_t len, size_t* num_read);
        bool detect_attributes(PN532UID* uid, uint16_t* block_count, uint16_t* block_size);
        bool transceive(const uint8_t* send, size_t len, uint8_t* response, int16_t* response_len);
        bool block_read(uint16_t block, uint8_t* buffer, size_t buf_len);
        int16_t authenticate(PN532Key key_type, const uint8_t* key, PN532UID* uid, uint8_t block);
        void hard_reset();
    private:
        bool write_data(const uint8_t* data, uint8_t len);
        int16_t read_data(uint8_t* buffer, size_t len, uint32_t timeout);
        bool wait_ready(uint32_t timeout_ms);
        bool check_ack();
    private:
        static void IRAM_ATTR irq_handler(void* arg);
};