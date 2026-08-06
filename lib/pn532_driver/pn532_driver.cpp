#include "pn532_driver.hpp"

#include <cstring>
#include <variant>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_timer.h>

inline constexpr uint8_t HIGH = 1;
inline constexpr uint8_t LOW = 0;
inline constexpr uint8_t BYTE = 8;

inline constexpr uint32_t CLK_FREQ = 2e6;
inline constexpr uint8_t I2C_ADDR = 0x24;

inline constexpr uint8_t PREAMBLE = 0x00;
inline constexpr uint8_t STARTCODE1 = 0x00;
inline constexpr uint8_t STARTCODE2 = 0xFF;
inline constexpr uint8_t HOSTTOPN532 = 0xD4;
inline constexpr uint8_t PN532TOHOST = 0xD5;
inline constexpr uint8_t POSTAMBLE = 0x00;

inline constexpr uint8_t GETFIRMWAREVERSION = 0x02;
inline constexpr uint8_t INDATAEXCHANGE = 0x40;
inline constexpr uint8_t INLISTPASSIVETARGET = 0x4A;
inline constexpr uint8_t INLISTPASSIVETARGET_RESP = 0x4B;

inline constexpr uint8_t SAMCONFIGURATION = 0x14;
inline constexpr uint8_t RFCONFIGURATION = 0x32;

inline constexpr uint8_t MIFARE_CMD_AUTH_A = 0x60;
inline constexpr uint8_t MIFARE_CMD_AUTH_B = 0x61;
inline constexpr uint8_t MIFARE_CMD_READ = 0x30;

inline constexpr const uint8_t ACK[] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};

inline constexpr uint8_t DATA_WRITE = 1;
inline constexpr uint8_t STATUS_READ = 2;
inline constexpr uint8_t DATA_READ = 3;

static const char* TAG = "PN532_DRIVER";

PN532::PN532(PN532Config& cfg) : config(cfg), is_initialized(false) {
    ESP_LOGI(TAG, "Initializing PN532");

    if (config.rst != GPIO_NUM_NC) {
        gpio_config_t rst_cfg = {};
        rst_cfg.pin_bit_mask = (1ULL << config.rst);
        rst_cfg.mode = GPIO_MODE_OUTPUT;
        rst_cfg.pull_up_en = GPIO_PULLUP_DISABLE;
        rst_cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
        rst_cfg.intr_type = GPIO_INTR_DISABLE;
        gpio_config(&rst_cfg);

        gpio_set_level(config.rst, 1);
    }

    if (config.irq != GPIO_NUM_NC) {
        gpio_config_t irq_cfg = {};
        irq_cfg.pin_bit_mask = (1ULL << config.irq);
        irq_cfg.mode = GPIO_MODE_INPUT;
        irq_cfg.pull_up_en = GPIO_PULLUP_ENABLE;
        irq_cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
        irq_cfg.intr_type = GPIO_INTR_DISABLE; 

        gpio_config(&irq_cfg);
    }

    if (std::holds_alternative<PN532SPIConfig>(config.interface)) {
        auto& spi_cfg = std::get<PN532SPIConfig>(config.interface);
        ESP_LOGI(TAG, "Configuring PN532 on SPI");

        gpio_config_t cs_cfg = {};
        cs_cfg.pin_bit_mask = (1ULL << spi_cfg.cs);
        cs_cfg.mode = GPIO_MODE_OUTPUT;
        cs_cfg.pull_up_en = GPIO_PULLUP_DISABLE;
        cs_cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
        cs_cfg.intr_type = GPIO_INTR_DISABLE;
        gpio_config(&cs_cfg);
        gpio_set_level(spi_cfg.cs, 1); 

        spi_bus_config_t bus_cfg = {};
        bus_cfg.mosi_io_num = spi_cfg.mosi;
        bus_cfg.miso_io_num = spi_cfg.miso;
        bus_cfg.sclk_io_num = spi_cfg.sck;
        bus_cfg.quadwp_io_num = -1;
        bus_cfg.quadhd_io_num = -1;
        bus_cfg.max_transfer_sz = sizeof(tx_buffer);

        esp_err_t err = spi_bus_initialize(spi_cfg.host, &bus_cfg, SPI_DMA_CH_AUTO);
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
            ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(err));
            return;
        }

        spi_device_interface_config_t dev_cfg = {};
        dev_cfg.command_bits = 0;
        dev_cfg.address_bits = 0;
        dev_cfg.dummy_bits = 0;
        dev_cfg.clock_speed_hz = CLK_FREQ; 
        dev_cfg.mode = 0; 
        dev_cfg.spics_io_num = -1; 
        dev_cfg.queue_size = 3;
        dev_cfg.flags = SPI_DEVICE_BIT_LSBFIRST;

        spi_bus_add_device(spi_cfg.host, &dev_cfg, &this->spi_dev_handle);
    }

    else if (std::holds_alternative<PN532I2CConfig>(config.interface)) {
        auto& i2c_cfg = std::get<PN532I2CConfig>(config.interface);
        ESP_LOGI(TAG, "Configuring PN532 on I2C");

        i2c_master_bus_config_t bus_cfg = {};
        bus_cfg.clk_source = I2C_CLK_SRC_DEFAULT;
        bus_cfg.i2c_port = I2C_NUM_0;
        bus_cfg.sda_io_num = i2c_cfg.sda;
        bus_cfg.scl_io_num = i2c_cfg.scl;
        bus_cfg.glitch_ignore_cnt = 7;
        bus_cfg.flags = {
            .enable_internal_pullup = true,
            .allow_pd = false
        };

        esp_err_t err = i2c_new_master_bus(&bus_cfg, &this->i2c_bus_handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize I2C bus: %s", esp_err_to_name(err));
            return;
        }

        i2c_device_config_t dev_cfg = {};
        dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
        dev_cfg.device_address = i2c_cfg.addr;
        dev_cfg.scl_speed_hz = 100000;

        err = i2c_master_bus_add_device(this->i2c_bus_handle, &dev_cfg, &this->i2c_dev_handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to add master to I2C bus: %s", esp_err_to_name(err));
            return;
        }
        
    }

    else if (std::holds_alternative<PN532UARTConfig>(config.interface)) {
        auto& uart_cfg = std::get<PN532UARTConfig>(config.interface);
        ESP_LOGI(TAG, "Configuring PN532 on UART");

        uart_config_t bus_cfg = {};
        bus_cfg.baud_rate = uart_cfg.baud;
        bus_cfg.data_bits = UART_DATA_8_BITS;
        bus_cfg.parity = UART_PARITY_DISABLE;
        bus_cfg.stop_bits = UART_STOP_BITS_1;
        bus_cfg.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
        bus_cfg.source_clk = UART_SCLK_DEFAULT;
        
        esp_err_t err = uart_param_config(uart_cfg.port, &bus_cfg);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize UART bus: %s", esp_err_to_name(err));
            return;
        }
        err = uart_set_pin(uart_cfg.port, uart_cfg.tx, uart_cfg.rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to set UART pins: %s", esp_err_to_name(err));
            return;
        }
        err = uart_driver_install(uart_cfg.port, sizeof(tx_buffer) * 2, sizeof(rx_buffer) * 2, 0, NULL, 0);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to set install UART driver: %s", esp_err_to_name(err));
            return;
        }
    }

    is_initialized = true;
    ESP_LOGI(TAG, "PN532 initialized successfully");
}

PN532::~PN532() {
    if (!is_initialized) {
        return;
    }

    if (std::holds_alternative<PN532SPIConfig>(config.interface)) {
        spi_bus_remove_device(this->spi_dev_handle);
    }
    else if (std::holds_alternative<PN532I2CConfig>(config.interface)) {
        i2c_master_bus_rm_device(this->i2c_dev_handle);
    }
    else if (std::holds_alternative<PN532UARTConfig>(config.interface)) {
        auto& uart_cfg = std::get<PN532UARTConfig>(config.interface);
        uart_driver_delete(uart_cfg.port);
        gpio_reset_pin(uart_cfg.rx);
        gpio_reset_pin(uart_cfg.tx);
    }

    is_initialized = false;
}

void IRAM_ATTR PN532::irq_handler(void* arg) {
    PN532* instance = static_cast<PN532*>(arg);

    if (instance->waiting_task == nullptr) return;
    
    BaseType_t high_task_woken = pdFALSE;

    vTaskNotifyGiveFromISR(instance->waiting_task, &high_task_woken);

    if (high_task_woken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

void PN532::wakeup() {
    if (std::holds_alternative<PN532SPIConfig>(config.interface)) {
        auto& spi_cfg = std::get<PN532SPIConfig>(config.interface);

        spi_device_acquire_bus(this->spi_dev_handle, portMAX_DELAY);

        gpio_set_level(spi_cfg.cs, LOW);
        vTaskDelay(pdMS_TO_TICKS(2));
        gpio_set_level(spi_cfg.cs, HIGH);

        spi_device_release_bus(this->spi_dev_handle);
    }
    else if (std::holds_alternative<PN532I2CConfig>(config.interface)) {}
    else if (std::holds_alternative<PN532UARTConfig>(config.interface)) {
        auto& uart_cfg = std::get<PN532UARTConfig>(config.interface);
        tx_buffer[0] = 0x55;
        uart_write_bytes(uart_cfg.port, this->tx_buffer, 1);
        uart_write_bytes(uart_cfg.port, this->tx_buffer, 1);
        tx_buffer[0] = 0x00;
        uart_write_bytes(uart_cfg.port, this->tx_buffer, 1);
        uart_write_bytes(uart_cfg.port, this->tx_buffer, 1);
        uart_write_bytes(uart_cfg.port, this->tx_buffer, 1);

        uart_flush_input(uart_cfg.port);
    }

    vTaskDelay(pdMS_TO_TICKS(5));
}

bool PN532::sam_config() {
    uint8_t params[4];
    params[0] = SAMCONFIGURATION;
    params[1] = 0x01; 
    params[2] = 0x14; 
    params[3] = 0x01; 

    if (!write_data(params, 4)) return false;
    if (!wait_ready(1000)) return false;
    if (!check_ack()) return false;
    if (!wait_ready(1000)) return false;

    uint8_t response[1];
    return read_data(response, sizeof(response), 1000) >= 0;
}

bool PN532::set_passive_activation_retries(uint8_t max_retries) {
    uint8_t params[5];
    params[0] = RFCONFIGURATION;
    params[1] = 0x05; 
    params[2] = 0xFF;
    params[3] = 0x01; 
    params[4] = max_retries; 

    if (!write_data(params, 5)) return false;
    if (!wait_ready(1000)) return false;
    if (!check_ack()) return false;
    if (!wait_ready(1000)) return false;

    uint8_t response[1];
    return read_data(response, sizeof(response), 1000) >= 0;
}

bool PN532::get_firmware_version(uint32_t& version) {
    uint8_t params[1] = {GETFIRMWAREVERSION};

    if (!write_data(params, 1)) return false;
    if (!wait_ready(1000)) return false;
    if (!check_ack()) return false;
    if (!wait_ready(1000)) return false;

    uint8_t response[4];
    if (read_data(response, sizeof(response), 1000) != 4) return false;

    version = (response[0] << 24) | (response[1] << 16) | (response[2] << 8) | response[3];
    return true;
}

bool PN532::read_targets(PN532UID* buffer, size_t len, size_t* num_read) {
    if (num_read != nullptr) *num_read = 0;
    if (buffer == nullptr || len == 0) return false;
    if (len == 0) return false;

    if (len > 2) len = 2;

    uint8_t params[3];
    params[0] = INLISTPASSIVETARGET;
    params[1] = len;
    params[2] = 0x00;

    if (!write_data(params, 3)) return false;
    if (!wait_ready(1000)) return false;
    if (!check_ack()) return false;
    if (!wait_ready(1000)) return false;

    uint8_t response[64];
    int16_t resp_len = read_data(response, sizeof(response), 1000);

    if (resp_len < 1) return false;

    uint8_t targets_found = response[0];

    if (targets_found == 0) return true;

    size_t count = 0;
    size_t idx = 1;

    for (uint8_t i = 0; i < targets_found && i < len; i++) {
        if (idx >= resp_len) break;

        idx++;

        buffer[i].atqa = (response[idx] << 8) | response[idx + 1];
        idx += 2;

        buffer[i].sak = response[idx++];

        uint8_t uid_len = response[idx++];
        buffer[i].uid_length = uid_len;

        if (uid_len > 10) uid_len = 10;
        memcpy(buffer[i].uid, &response[idx], uid_len);
        idx += uid_len;

        buffer[i].subtype = PN532NFCType::UNKNOWN;
        buffer[i].block_size = 0;
        buffer[i].blocks_count = 0;

        count++;
    } 

    if (num_read != nullptr) *num_read = count;
    return true;
}

bool PN532::detect_attributes(PN532UID* uid, uint16_t* blocks_count, uint16_t* block_size) {
    if (uid == nullptr || blocks_count == nullptr || block_size == nullptr) {
        return false;
    }

    uid->subtype  = PN532NFCType::UNKNOWN;
    *blocks_count = 0;
    *block_size   = 0;

    switch (uid->sak & 0x7F) {
    case 0x00:
        uid->subtype  = PN532NFCType::MIFARE_ULTRALIGHT;
        *blocks_count = 16;
        *block_size   = 4;
        break;
    case 0x08:
        uid->subtype  = PN532NFCType::MIFARE_CLASSIC_1K;
        *blocks_count = 64;
        *block_size   = 16;
        break;
    case 0x09:
        uid->subtype  = PN532NFCType::MIFARE_CLASSIC_MINI;
        *blocks_count = 20;
        *block_size   = 16;
        break;
    case 0x10:
    case 0x11:
        uid->subtype  = PN532NFCType::MIFARE_PLUS_2K;
        *blocks_count = 128;
        *block_size   = 16;
        break;
    case 0x18:
    case 0x38:
        uid->subtype  = PN532NFCType::MIFARE_CLASSIC_4K;
        *blocks_count = 256;
        *block_size   = 16;
        break;
    case 0x20:
    case 0x24:
        uid->subtype  = PN532NFCType::MIFARE_DESFIRE;
        *blocks_count = 0;
        *block_size   = 1;
        break;
    case 0x28:
        uid->subtype  = PN532NFCType::MIFARE_CLASSIC_1K;
        *blocks_count = 64;
        *block_size   = 16;
        break;
    default:
        break;
    }

    uid->blocks_count = *blocks_count;
    uid->block_size   = *block_size;
    return true;
}

bool PN532::transceive(const uint8_t* send, size_t send_len, uint8_t* response, int16_t* response_len) {
    uint8_t params[260];
    params[0] = INDATAEXCHANGE; 
    params[1] = 0x01;

    memcpy(&params[2], send, send_len);

    if (!write_data(params, send_len + 2)) return false;
    if (!wait_ready(1000)) return false;
    if (!check_ack()) return false;
    if (!wait_ready(1000)) return false;

    uint8_t rx_buf[260];
    int16_t len = read_data(rx_buf, sizeof(rx_buf), 1000);

    if (len < 1 || rx_buf[0] != 0x00) return false;

    *response_len = len - 1;
    memcpy(response, &rx_buf[1], *response_len);

    return true;
}

int16_t PN532::authenticate(PN532Key key_type, const uint8_t* key, PN532UID* uid, uint8_t block) {
    uint8_t params[20];
    size_t idx = 0;
    params[idx++] = INDATAEXCHANGE;
    params[idx++] = 0x01;  
    params[idx++] = static_cast<uint8_t>(key_type);  
    params[idx++] = block;
    
    memcpy(&params[idx], key, 6);
    idx += 6;

    uint8_t copy_len = (uid->uid_length > 4) ? 4 : uid->uid_length;
    memcpy(&params[idx], uid->uid, copy_len); 
    idx += copy_len;

    if (!write_data(params, idx)) return PN532Err::TIMEOUT;
    if (!wait_ready(1000)) return PN532Err::TIMEOUT;
    if (!check_ack()) return PN532Err::TIMEOUT;
    if (!wait_ready(1000)) return PN532Err::TIMEOUT;

    uint8_t response[2];
    int16_t len = read_data(response, sizeof(response), 1000);
    
    if (len < 0) return len; 
    if (len < 1) return PN532Err::INVALID_FRAME;

    if (response[0] == 0x00) return 1;

    return 0;
}

bool PN532::block_read(uint16_t block, uint8_t* buffer, size_t buf_len) {
    uint8_t params[4];
    params[0] = INDATAEXCHANGE;
    params[1] = 0x01;
    params[2] = MIFARE_CMD_READ;
    params[3] = static_cast<uint8_t>(block);

    if (!write_data(params, 4)) return false;
    if (!wait_ready(1000)) return false;
    if (!check_ack()) return false;
    if (!wait_ready(1000)) return false;

    uint8_t response[18];
    int16_t len = read_data(response, sizeof(response), 1000);
    
    if (len < 17 || response[0] != 0x00) return false;

    size_t copy_len = (buf_len < 16) ? buf_len : 16;
    memcpy(buffer, &response[1], copy_len); 

    return true;
}

bool PN532::write_data(const uint8_t* data, uint8_t len) {    
    len++;
    uint8_t sum = HOSTTOPN532;
    uint16_t idx = 0;

    tx_buffer[idx++] = DATA_WRITE;
    tx_buffer[idx++] = PREAMBLE;
    tx_buffer[idx++] = STARTCODE1;
    tx_buffer[idx++] = STARTCODE2;
    
    tx_buffer[idx++] = len;
    tx_buffer[idx++] = ~len + 1;

    tx_buffer[idx++] = HOSTTOPN532;

    this->command = data[0];

    for (size_t i = 0; i < len - 1; i++) {
        tx_buffer[idx++] = data[i];
        sum += data[i];
    }

    tx_buffer[idx++] = ~sum + 1;
    tx_buffer[idx++] = POSTAMBLE;

    esp_err_t err;

    wakeup();
    
    if (std::holds_alternative<PN532SPIConfig>(config.interface)) {
        auto& spi_cfg = std::get<PN532SPIConfig>(config.interface);

        spi_transaction_t t = {};
        t.length = BYTE * idx; 
        t.tx_buffer = tx_buffer;

        spi_device_acquire_bus(this->spi_dev_handle, portMAX_DELAY);
        
        gpio_set_level(spi_cfg.cs, LOW);

        spi_device_transmit(this->spi_dev_handle, &t);

        gpio_set_level(spi_cfg.cs, HIGH);

        spi_device_release_bus(this->spi_dev_handle);    

        return err == ESP_OK;
    }
    else if (std::holds_alternative<PN532I2CConfig>(config.interface)) {        
        err = i2c_master_transmit(this->i2c_dev_handle, &tx_buffer[1], idx - 1, -1);

        return err == ESP_OK;
    }
    else if (std::holds_alternative<PN532UARTConfig>(config.interface)) {
        auto& uart_cfg = std::get<PN532UARTConfig>(config.interface);
        
        int written = uart_write_bytes(uart_cfg.port, &tx_buffer[1], idx - 1);

        return written == (idx - 1);
    }

    return false;
}

int16_t PN532::read_data(uint8_t* buffer, size_t len, uint32_t timeout) {
    memset(tx_buffer, 0, sizeof(tx_buffer));
    memset(rx_buffer, 0, sizeof(rx_buffer));

    uint8_t* rx_ptr = rx_buffer;
    
    if (std::holds_alternative<PN532SPIConfig>(config.interface)) {
        auto& spi_cfg = std::get<PN532SPIConfig>(config.interface);

        if (!wait_ready(timeout)) return PN532Err::TIMEOUT;
        
        tx_buffer[0] = DATA_READ;

        spi_transaction_t t = {};
        t.length = BYTE * sizeof(rx_buffer);
        t.tx_buffer = this->tx_buffer;
        t.rx_buffer = this->rx_buffer;

        spi_device_acquire_bus(this->spi_dev_handle, portMAX_DELAY);

        gpio_set_level(spi_cfg.cs, LOW);

        spi_device_transmit(this->spi_dev_handle, &t);

        gpio_set_level(spi_cfg.cs, HIGH);

        spi_device_release_bus(this->spi_dev_handle);

        uint16_t offset = 0;
        
        while (offset < sizeof(rx_buffer) - 3) {
            if (rx_buffer[offset] == PREAMBLE && rx_buffer[offset+1] == STARTCODE1 && rx_buffer[offset+2] == STARTCODE2) {
                break;
            }
            offset++;
        }
        
        if (offset >= sizeof(rx_buffer) - 3) return PN532Err::INVALID_FRAME;
        rx_ptr = &rx_buffer[offset];
    }
    else if (std::holds_alternative<PN532I2CConfig>(config.interface)) {
        if (!wait_ready(timeout)) return PN532Err::TIMEOUT;

        esp_err_t err = i2c_master_receive(this->i2c_dev_handle, this->rx_buffer, sizeof(rx_buffer), timeout);
        if (err != ESP_OK) return PN532Err::TIMEOUT;

        rx_ptr++;
    }
    else if (std::holds_alternative<PN532UARTConfig>(config.interface)) {
        auto& uart_cfg = std::get<PN532UARTConfig>(config.interface);

        if (!wait_ready(timeout)) return PN532Err::TIMEOUT;

        size_t read_bytes = uart_read_bytes(uart_cfg.port, this->rx_buffer, 6, pdMS_TO_TICKS(timeout));
        if (read_bytes < 6) return PN532Err::TIMEOUT;

        uint8_t expected_length = this->rx_buffer[3];
        
        int remaining_bytes = expected_length + 1;
        read_bytes += uart_read_bytes(uart_cfg.port, this->rx_buffer + 6, remaining_bytes, pdMS_TO_TICKS(timeout));
        
        if (read_bytes < (6 + remaining_bytes)) return PN532Err::TIMEOUT;
    }

    size_t received = 3;

    if (rx_ptr[0] != PREAMBLE || rx_ptr[1] != STARTCODE1 || rx_ptr[2] != STARTCODE2) {
        return PN532Err::INVALID_FRAME;
    }

    uint8_t length = rx_ptr[received++];
    if (static_cast<uint8_t>(length + rx_ptr[received++]) != 0) {
        return PN532Err::INVALID_FRAME;
    }

    uint8_t cmd = this->command + 1;
    if (rx_ptr[received++] != PN532TOHOST || rx_ptr[received++] != cmd) {
        return PN532Err::INVALID_FRAME;
    }

    length -= 2;
    if (length > len) {
        return PN532Err::NO_SPACE;
    }

    uint8_t sum = PN532TOHOST + cmd;
    for (size_t i = 0; i < length; i++) {
        buffer[i] = rx_ptr[received++];
        sum += buffer[i];
    }

    uint8_t checksum = rx_ptr[received++];
    if (static_cast<uint8_t>(sum + checksum)) {
        return PN532Err::INVALID_FRAME;
    }

    return length;
}

bool PN532::wait_ready(uint32_t timeout_ms) {
    waiting_task = xTaskGetCurrentTaskHandle();

    ulTaskNotifyTake(pdTRUE, 0);

    gpio_set_intr_type(config.irq, GPIO_INTR_NEGEDGE);
    gpio_isr_handler_add(config.irq, irq_handler, this);

    uint32_t notification_value = 0;

    if (gpio_get_level(config.irq) == 0) {
        notification_value = 1;
    } else {
        notification_value = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(timeout_ms));
    }

    gpio_isr_handler_remove(config.irq);
    gpio_set_intr_type(config.irq, GPIO_INTR_DISABLE);
    waiting_task = nullptr;

    if (notification_value > 0) {
        return true;
    }
    else {
        ESP_LOGE(TAG, "Hardware timeout waiting for PN532 IRQ");

        if (std::holds_alternative<PN532SPIConfig>(config.interface)) {
            auto& spi_cfg = std::get<PN532SPIConfig>(config.interface);
            
            tx_buffer[0] = DATA_WRITE;
            memcpy(&tx_buffer[1], ACK, sizeof(ACK));
            
            spi_transaction_t t = {};
            t.length = BYTE * (sizeof(ACK) + 1);
            t.tx_buffer = tx_buffer;

            spi_device_acquire_bus(this->spi_dev_handle, portMAX_DELAY);
            
            gpio_set_level(spi_cfg.cs, LOW);

            spi_device_transmit(this->spi_dev_handle, &t);

            gpio_set_level(spi_cfg.cs, HIGH);

            spi_device_release_bus(this->spi_dev_handle);
        }
        else if (std::holds_alternative<PN532I2CConfig>(config.interface)) {
            i2c_master_transmit(this->i2c_dev_handle, ACK, sizeof(ACK), 10);
        }
        else if (std::holds_alternative<PN532UARTConfig>(config.interface)) {
            auto& uart_cfg = std::get<PN532UARTConfig>(config.interface);
            uart_write_bytes(uart_cfg.port, ACK, sizeof(ACK));
        }

        return false;
    }
}

bool PN532::check_ack() {
    uint8_t ackBuf[16] = {0};
    memset(tx_buffer, 0, sizeof(tx_buffer));
    uint8_t* ackPtr = ackBuf;

    if (std::holds_alternative<PN532SPIConfig>(config.interface)) {
        auto& spi_cfg = std::get<PN532SPIConfig>(config.interface);

        tx_buffer[0] = DATA_READ;

        
        
        spi_transaction_t t = {};
        t.length = BYTE * sizeof(ackBuf);
        t.tx_buffer = this->tx_buffer;
        t.rx_buffer = ackBuf;

        spi_device_acquire_bus(this->spi_dev_handle, portMAX_DELAY);

        gpio_set_level(spi_cfg.cs, LOW);

        esp_err_t err = spi_device_transmit(this->spi_dev_handle, &t);
        if (err != ESP_OK) {
            return false;
        }

        gpio_set_level(spi_cfg.cs, HIGH);

        spi_device_release_bus(this->spi_dev_handle);

        for (size_t i = 0; i < sizeof(ackBuf) - 2; i++) {
            if (ackBuf[i] == PREAMBLE && ackBuf[i+1] == STARTCODE1 && ackBuf[i+2] == STARTCODE2) {
                ackPtr = &ackBuf[i];
                break;
            }
        }
    }
    else if (std::holds_alternative<PN532I2CConfig>(config.interface)) {
        esp_err_t err = i2c_master_receive(this->i2c_dev_handle, ackBuf, sizeof(ACK) + 1, 10);
        if (err != ESP_OK) {
            return false;
        }
        ackPtr++;
    }
    else if (std::holds_alternative<PN532UARTConfig>(config.interface)) {
        auto& uart_cfg = std::get<PN532UARTConfig>(config.interface);

        if (uart_read_bytes(uart_cfg.port, ackBuf, sizeof(ACK), 10) <= 0) return false;
    } 

    return memcmp(ackPtr, ACK, sizeof(ACK)) == 0;
}

void PN532::hard_reset() {
    if (config.rst == GPIO_NUM_NC) return;

    gpio_set_level(config.rst, LOW);
    vTaskDelay(pdMS_TO_TICKS(1));
    gpio_set_level(config.rst, HIGH);
    vTaskDelay(pdMS_TO_TICKS(50));
}