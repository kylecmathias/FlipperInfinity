#include "ir_menu.hpp"

static rmt_channel_handle_t ir_tx_channel = nullptr;
static rmt_channel_handle_t ir_rx_channel = nullptr;
static rmt_encoder_handle_t ir_encoder = nullptr;
static std::atomic<bool> transmitting = false;
static std::atomic<bool> receiving = false;
static QueueHandle_t ir_rx_queue = nullptr;
static rmt_symbol_word_t ir_rx_buffer[IR_SYMBOL_BUFFER_SIZE];
static std::atomic<size_t> current_rx_symbol_count = 0;
static rmt_symbol_word_t ir_tx_buffer[IR_SYMBOL_BUFFER_SIZE];
static std::atomic<size_t> current_tx_symbol_count = 0;

//helpers
void deinit_ir_rmt() {
    if (ir_tx_channel != nullptr) {
        rmt_disable(ir_tx_channel);
        rmt_del_channel(ir_tx_channel);
        ir_tx_channel = nullptr;
    }
    
    if (ir_rx_channel != nullptr) {
        rmt_disable(ir_rx_channel);
        rmt_del_channel(ir_rx_channel);
        ir_rx_channel = nullptr;
    }
}
void init_ir_menu() {
    if (ir_rx_queue == nullptr) {
        ir_rx_queue = xQueueCreate(10, sizeof(size_t));
    }
}