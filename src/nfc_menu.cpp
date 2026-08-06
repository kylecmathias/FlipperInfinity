#include "nfc_menu.hpp"


//statics
static std::atomic<bool> read_scanning = false;
// static pn532_t* pn532 = nullptr;
static std::optional<PN532> pn532;
static lv_timer_t* read_timer = nullptr;
static QueueHandle_t nfc_save_queue = nullptr;
static PN532UID current_scanned_uid;
static std::atomic<uint16_t> current_blocks = 0;
static std::atomic<uint16_t> current_block_size = 0;
static std::atomic<bool> current_is_mifare = false;
static uint8_t* tag_psram_buffer = nullptr;
static size_t last_read_total_size = 0;
static std::string cached_preview_text = "";
static std::atomic<bool> nfc_busy = false;
static std::atomic<bool> tag_ready_to_save = false;



//helpers
void nfc_read_task(void* pvParameters) {
    bool is_mifare = current_is_mifare.load(std::memory_order_acquire);
    uint16_t blocks = current_blocks.load(std::memory_order_acquire);
    uint16_t block_size = current_block_size.load(std::memory_order_acquire);
    size_t total_size = static_cast<size_t>(blocks) * block_size;

    if (tag_psram_buffer == nullptr) printf("DEBUG FAIL: tag_psram_buffer is NULL!\n");
    if (total_size == 0) printf("DEBUG FAIL: total_size is 0! (Detection failed)\n");
    if (total_size > PAYLOAD_BUFFER_SIZE) printf("DEBUG FAIL: total_size (%u) exceeds PSRAM buffer (%u)\n", total_size, PAYLOAD_BUFFER_SIZE);

    bool success = (tag_psram_buffer != nullptr) && (total_size > 0) && (total_size <= PAYLOAD_BUFFER_SIZE);
    bool is_protected = false;

    if (success) {
        std::string preview;
        preview.reserve(TagPreviewStrings::total_reserve_size + 32);
        preview += TagPreviewStrings::uid.data();
        for (size_t i = 0; i < current_scanned_uid.uid_length; i++) {
            char hex[4];
            snprintf(hex, sizeof(hex), "%02X", current_scanned_uid.uid[i]);
            preview += hex;
            if (i < current_scanned_uid.uid_length - 1) preview += " ";
        }
        preview += "\n";
        preview += TagPreviewStrings::type.data();
        preview += get_tag_type(current_scanned_uid.sak, blocks);
        preview += "\n";
        preview += TagPreviewStrings::blocks.data() + std::to_string(blocks);
        preview += '\n';
        preview += TagPreviewStrings::size.data() + std::to_string(total_size) + TagPreviewStrings::size_suffix;
        preview += '\n';
        preview += TagPreviewStrings::separator.data();
        preview += '\n';
        preview += TagPreviewStrings::preview.data();
        preview += '\n';

        size_t preview_bytes = 0;
        for (size_t i = 0; i < blocks;) {
            size_t current_offset = i * block_size;
            uint8_t* dest = tag_psram_buffer + current_offset;

            uint8_t read_buffer[NFC_CHUNK_LEN] = {0};
            size_t read_len = (block_size == 4) ? 4 : 1;

            bool read_ok = false;
            int16_t auth_status = 1;
            uint8_t retries = 5;
            while (retries > 0) {
                auth_status = 1;
                
                if (is_mifare) {
                    auth_status = pn532->authenticate(PN532Key::KEY_A, DEFAULT_KEY, &current_scanned_uid, i);
                }
                
                if (auth_status == 1) {
                    read_ok = pn532->block_read(i, read_buffer, NFC_CHUNK_LEN);
                    if (read_ok) {
                        break; 
                    }
                }
                else if (auth_status == 0) {
                    printf("DEBUG: Auth rejected at block %zu. Tag is protected.\n", i);
                    is_protected = true;
                    success = false;
                    break; 
                }
                
                retries--;
                printf("WARN: Read/Auth failed at block %zu, retries left: %d\n", i, retries);
                
                vTaskDelay(pdMS_TO_TICKS(10)); 
            }
            
            if (!read_ok) { 
                printf("DEBUG: read failed at block %zu, t=%lu ms\n", i, time_ms());
                if (block_size == 4 && i >= blocks - 4) {
                    success = true; 
                    break;
                }
                memset(dest, 0, block_size);
                success = false;
                break; 
            }
            printf("Successful read at block: %zu, t=%lu ms\n", i, time_ms());

            size_t copy_bytes = read_len * block_size;
            size_t remaining_bytes = total_size - (i * block_size);
            if (copy_bytes > remaining_bytes) {
                copy_bytes = remaining_bytes;
            }

            memcpy(dest, read_buffer, copy_bytes);

            for (size_t b = 0; b < copy_bytes && preview_bytes < PREVIEW_BYTES; b++) {
                char hex[4];
                snprintf(hex, sizeof(hex), "%02X", dest[b]);
                preview += hex;
                preview_bytes++;
                preview += (preview_bytes % 8 == 0) ? '\n' : ' ';
            }

            i += read_len;

            vTaskDelay(pdMS_TO_TICKS(2));
        
        }
        if (total_size > PREVIEW_BYTES) preview += "...\n";

        cached_preview_text = std::move(preview);
        last_read_total_size = total_size;
    }

    lvgl_port_lock(0);
    if (!success && is_mifare && is_protected) {
        lv_textarea_set_text(objects.nfc_read_results_textarea, "Protected Mifare tag detected");
        lv_obj_set_style_bg_color(objects.toggle_reading_nfc, lv_color_hex(0x747474), LV_PART_MAIN);
    } 
    else {
        lv_textarea_set_text(objects.nfc_read_results_textarea, success ? cached_preview_text.c_str() : "Read failed");
        lv_obj_set_style_bg_color(objects.toggle_reading_nfc, success ? lv_color_hex(0x747474) : lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
    }
    lv_obj_clear_state(objects.toggle_reading_nfc, LV_STATE_DISABLED);
    lvgl_port_unlock();

    tag_ready_to_save.store(success, std::memory_order_release);
    nfc_busy.store(false, std::memory_order_release);

    if (success) {
        lv_event_send(objects.nfc_read_filename_textarea, LV_EVENT_VALUE_CHANGED, nullptr);
    }
    // vTaskDelete(wdt_task);
    vTaskDelete(NULL);
}
void file_write_task(void *pvParameters) {
    nfc_save_chunk chunk;
    while (true) {
        if (xQueueReceive(nfc_save_queue, &chunk, portMAX_DELAY) == pdTRUE) {
            char filepath[FILEPATH_LEN];
            snprintf(filepath, sizeof(filepath), "%s/%s.bin", LITTLEFS_MOUNT, chunk.filename);

            FILE* f = fopen(filepath, "wb");
            bool success = (f != nullptr);
            if (success) {
                fprintf(f, "UID:");
                for (size_t i = 0; i < chunk.uid.uid_length; i++) fprintf(f, "%02X", chunk.uid.uid[i]);
                fprintf(f, "\nBlocks:%d\nBlockSize:%d\nDATA\n", chunk.blocks, chunk.block_size);
                fwrite(tag_psram_buffer, 1, last_read_total_size, f); 
                fclose(f);
            }

            lvgl_port_lock(0);
            lv_obj_set_style_bg_color(objects.nfc_read_save_tag, success ? lv_palette_main(LV_PALETTE_GREEN) : lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
            lv_obj_clear_state(objects.toggle_reading_nfc, LV_STATE_DISABLED);
            lvgl_port_unlock();

            nfc_busy.store(false, std::memory_order_release);
        }
    }
}
void nfc_read_timer_cb(lv_timer_t * timer) {
    if (!pn532.has_value()) {
        printf("CRITICAL: PN532 pointer is NULL\n");
        read_scanning.store(false, std::memory_order_release);
        lv_timer_del(read_timer);
        read_timer = nullptr;
        lv_obj_set_style_bg_color(objects.toggle_reading_nfc, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
        lv_obj_add_state(objects.toggle_reading_nfc, LV_STATE_DISABLED);
        esp_restart();
        return;
    }

    PN532UID uids[2];
    size_t num_read;
    bool success = pn532->read_targets(uids, sizeof(uids) / sizeof(PN532UID), &num_read);

    if (!success || num_read == 0) return;

    read_scanning.store(false, std::memory_order_release);
    lv_timer_del(read_timer);
    read_timer = nullptr;

    lv_obj_set_style_bg_color(objects.toggle_reading_nfc, lv_color_hex(0x747474), LV_PART_MAIN);

    current_scanned_uid = uids[0]; 
    uint16_t blocks, block_size;
    pn532->detect_attributes(&current_scanned_uid, &blocks, &block_size);

    if (block_size == 4 && blocks == 16) {
        blocks = 135; 
    }
    bool is_mifare = false;
    if (block_size == 16 && sak_is_legacy_mifare(current_scanned_uid.sak)) {
        is_mifare = true;
    }
    else if (sak_is_iso14443_4(current_scanned_uid.sak)) {
        uint8_t version_cmd[] = {0x90, 0x60, 0x00, 0x00, 0x00};
        uint8_t response_buffer[64];
        int16_t response_size = 0;
        uint8_t version = 0x00;
        if (pn532->transceive(version_cmd, sizeof(version_cmd), response_buffer, &response_size)){
            if (response_size > 4) version = response_buffer[4];
            int8_t family = mifare_family(version);
            if (family == 1) { 
                is_mifare = true;
            }
            else if (family == -1) {
                if (pn532->authenticate(PN532Key::KEY_A, DEFAULT_KEY, &current_scanned_uid, 0)) {
                    is_mifare = true;
                }
            }
        }
    }

    current_is_mifare.store(is_mifare, std::memory_order_release);
    current_blocks.store(blocks, std::memory_order_release);
    current_block_size.store(block_size, std::memory_order_release);

    nfc_busy.store(true, std::memory_order_release);
    lv_obj_add_state(objects.toggle_reading_nfc, LV_STATE_DISABLED); 
    lv_obj_add_state(objects.nfc_read_save_tag, LV_STATE_DISABLED); 
    xTaskCreatePinnedToCore(nfc_read_task, "nfc_read", 4096, nullptr, 4, nullptr, 0); 
}
void init_nfc() {
    PN532Config cfg = {};

    PN532SPIConfig spi_cfg = {};
    spi_cfg.host = PN532_HOST;
    spi_cfg.mosi = GPIO_PINS::MOSI;
    spi_cfg.miso = GPIO_PINS::MISO;
    spi_cfg.sck  = GPIO_PINS::SCK;
    spi_cfg.cs   = GPIO_PINS::PN532_SS;

    cfg.interface = spi_cfg;
    cfg.rst = GPIO_PINS::PN532_RST;
    cfg.irq = GPIO_PINS::PN532_IRQ;

    pn532.emplace(cfg);

    pn532->hard_reset();
    pn532->sam_config();
    pn532->set_passive_activation_retries(0x01);
}
void init_nfc_menu() {
    if (!pn532.has_value()){
        printf("CRITICAL: PN532 not initialized");
        return;
    }

    if (tag_psram_buffer == nullptr) {
        tag_psram_buffer = static_cast<uint8_t*>(heap_caps_malloc(PAYLOAD_BUFFER_SIZE, MALLOC_CAP_SPIRAM));
        if (tag_psram_buffer == nullptr) printf("CRITICAL: failed to allocate tag psram buffer\n");
    }

    lv_obj_add_state(objects.nfc_read_save_tag, LV_STATE_DISABLED);

    nfc_save_queue = xQueueCreate(SAVE_QUEUE_SIZE, sizeof(nfc_save_chunk));
    xTaskCreate(file_write_task, "NFC Writer", 4096, nullptr, 3, nullptr);
}


//variables
int32_t get_var_nfc_option() { return nfc_option; }
void set_var_nfc_option(int32_t value) { nfc_option = value; }


//actions
void action_change_nfc_option(lv_event_t * e) {
    lv_obj_t* nfc_options[] = {
        objects.nfc_read_container
    };
    size_t num_nfc_options = sizeof(nfc_options) / sizeof(lv_obj_t*);

    uint16_t option = lv_dropdown_get_selected(objects.wifi_option_dropdown);

    for (size_t i = 0; i < num_nfc_options; i++) {
        if (i == option) {
            lv_obj_clear_flag(nfc_options[i], LV_OBJ_FLAG_HIDDEN);
        }
        else {
            lv_obj_add_flag(nfc_options[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
}
void action_toggle_reading_nfc(lv_event_t * e) {
    if (!read_scanning.load(std::memory_order_acquire) && read_timer == nullptr) {
        read_scanning.store(true, std::memory_order_release);
        read_timer = lv_timer_create(nfc_read_timer_cb, PN532_POLL_INT, nullptr);

        lvgl_port_lock(0);
        lv_obj_set_style_bg_color(objects.toggle_reading_nfc, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);
        lvgl_port_unlock();
    }
    else {
        read_scanning.store(false, std::memory_order_release);
        if (read_timer != nullptr) {
            lv_timer_del(read_timer);
            read_timer = nullptr;
        }

        lvgl_port_lock(0);
        lv_obj_set_style_bg_color(objects.toggle_reading_nfc, lv_color_hex(0x747474), LV_PART_MAIN);
        lvgl_port_unlock();
    }
    lvgl_port_lock(0);
    lv_obj_set_style_bg_color(objects.nfc_read_save_tag, lv_color_hex(0x2196F3), LV_PART_MAIN);
    lvgl_port_unlock();
}
void action_nfc_read_save(lv_event_t * e) {
    const char* filename = lv_textarea_get_text(objects.nfc_read_filename_textarea);
    if (strlen(filename) == 0 || !tag_ready_to_save.load(std::memory_order_acquire)) return;

    nfc_save_chunk chunk;
    chunk.uid = current_scanned_uid;
    chunk.blocks = current_blocks.load(std::memory_order_acquire);
    chunk.block_size = current_block_size.load(std::memory_order_acquire);
    strlcpy(chunk.filename, filename, sizeof(chunk.filename));

    lv_obj_add_state(objects.toggle_reading_nfc, LV_STATE_DISABLED);

    if (xQueueSend(nfc_save_queue, &chunk, 0) == pdTRUE) {
        nfc_busy.store(true, std::memory_order_release);
        lv_obj_add_state(objects.nfc_read_save_tag, LV_STATE_DISABLED);
        lv_obj_add_state(objects.toggle_reading_nfc, LV_STATE_DISABLED); 
    }
}
void action_check_nfc_read_filename(lv_event_t * e) {
    const char* filename = lv_textarea_get_text(objects.nfc_read_filename_textarea);
    if (strlen(filename) > 0 && current_blocks > 0) {
        lv_obj_clear_state(objects.nfc_read_save_tag, LV_STATE_DISABLED);
    }
    else {
        lv_obj_add_state(objects.nfc_read_save_tag, LV_STATE_DISABLED);
    }
}