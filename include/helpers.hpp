#pragma once

#include <freertos/semphr.h>

constexpr int min(int a, int b) {
    return a < b ? a : b;
}

constexpr int max(int a, int b) {
    return a > b ? a : b;
}

constexpr int clamp(int num, int min, int max) {
    return num > max ? max : num < min ? min : num;
}

constexpr uint32_t time_ms() {
    return static_cast<uint32_t>(esp_timer_get_time() / 1000);
}

constexpr void delay_us(uint32_t us) {
    esp_rom_delay_us(us);
}

constexpr void delay_ms(uint32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

constexpr uint32_t map(uint32_t x, uint32_t in_min, uint32_t in_max, uint32_t out_min, uint32_t out_max) {
    return static_cast<uint32_t>((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}

class SemLock {
    private:        
        SemaphoreHandle_t mtx_handle;
        std::atomic<bool> is_locked = false;
    public:
        SemLock(SemaphoreHandle_t handle, TickType_t block_time) : mtx_handle(handle) {
            if (mtx_handle != nullptr && xSemaphoreTake(mtx_handle, block_time) == pdTRUE) {
                is_locked.store(true, std::memory_order_release);
            }
        }
        ~SemLock() {
            if (is_locked.load(std::memory_order_acquire)) {
                xSemaphoreGive(mtx_handle);
            }
        }
        SemLock(SemLock&&) = delete;
        SemLock& operator=(SemLock&&) = delete;
        SemLock(const SemLock&) = delete;
        SemLock& operator=(const SemLock&) = delete;

        bool locked() const { return is_locked.load(std::memory_order_acquire); }
};