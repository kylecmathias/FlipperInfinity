#include "bluetooth_menu.hpp"

static lv_timer_t* scan_timer;
static BLEInventoryDevice ble_inventory[MAX_INVENTORY_DEVICES] = {};
static std::atomic<size_t> ble_inventory_count = 0;
static std::atomic<bool> is_scanning = false;
static std::atomic<int8_t> scan_range = -70;
static SemaphoreHandle_t mtx_handle = nullptr;
static std::atomic<bool> ble_connected{false};
static std::atomic<bool> manual_ble_disconnect{false};
static uint16_t current_conn_handle = BLE_HS_CONN_HANDLE_NONE;
static uint16_t pending_auth_handle = BLE_HS_CONN_HANDLE_NONE;
static uint8_t pending_auth_action = 0;
static bool updating_mac = false;
static std::atomic<bool> spoofer_active{false};

static const BLECompanyEntry ble_companies[] = {
    {0x0001, "Ericsson Technology"},
    {0x0002, "Intel"},
    {0x0003, "IBM"},
    {0x0004, "Toshiba"},
    {0x0006, "Microslop"},
    {0x000A, "CSR (Qualcomm)"},
    {0x000D, "Texas Instruments"},
    {0x000F, "Broadcom"},
    {0x001D, "Qualcomm"},
    {0x0030, "STMicroelectronics"},
    {0x004C, "Apple"},
    {0x0055, "Plantronics Inc."},
    {0x0059, "Nordic Semiconductor"},
    {0x006B, "Philips"},
    {0x0075, "Samsung"},
    {0x0078, "Nike"},
    {0x0087, "Garmin"},
    {0x008A, "Jawbone"},
    {0x0090, "Logitech"},
    {0x009E, "Bose"},
    {0x00A3, "Motorola"},
    {0x00AB, "Nokia"},
    {0x00B5, "Swatch Group"},
    {0x00C4, "LG Electronics"},
    {0x00C7, "Vizio"},
    {0x00CC, "Beats Electronics"},
    {0x00D2, "Dialog Semiconductor"},
    {0x00DF, "Wahoo Fitness"},
    {0x00E0, "Google"},
    {0x00F0, "PayPal"},
    {0x0117, "Sony"},
    {0x012D, "Sony Ericsson"},
    {0x0136, "GoPro"},
    {0x0157, "Anker"},
    {0x015D, "Xiaomi"},
    {0x015E, "UniKey Technologies"},
    {0x017F, "Sennheiser"},
    {0x0195, "Fitbit"},
    {0x01AB, "Meta / Facebook"},
    {0x01B5, "Polycom"},
    {0x020C, "Oura Health"},
    {0x0211, "Lenovo"},
    {0x022D, "Tesla"},
    {0x024D, "iRobot"},
    {0x027D, "Huawei"},
    {0x02E5, "Espressif Systems"},
    {0x02F2, "Nintendo"},
    {0x0308, "DJI"},
    {0x038F, "Lumi United Technology"},
    {0x0499, "Ruckus Networks"},
    {0x049D, "Peloton"},
    {0x056F, "Sonos"}
};
static size_t company_database_count = sizeof(ble_companies) / sizeof(ble_companies[0]);

static const uint8_t payload_ibeacon[] = {
    0x02, 0x01, 0x06, 
    0x1A, 0xFF, 0x4C, 0x00, 0x02, 0x15, 
    0xE2, 0xC5, 0x6D, 0xB5, 0xDF, 0xFB, 0x48, 0xD2, 
    0xB0, 0x60, 0xD0, 0xF5, 0xA7, 0x10, 0x96, 0xE0, 
    0x00, 0x01, 0x00, 0x01, 0xC5
};
static const uint8_t payload_google[] = {
    0x02, 0x01, 0x06, 
    0x03, 0x03, 0x2C, 0xFE, 
    0x06, 0x16, 0x2C, 0xFE, 0x00, 0xB7, 0x27
};
static const uint8_t payload_samsung[] = {
    0x02, 0x01, 0x06, 
    0x0E, 0xFF, 0x75, 0x00, 0x01, 0x00, 0x02, 0x00, 
    0x01, 0x01, 0xFF, 0x00, 0x00, 0x43
};
static const uint8_t payload_flipper[] = {
    0x02, 0x01, 0x06, 
    0x06, 0x09, 'I', 'N', 'F', 'I', 'N', 
    0x05, 0xFF, 0xBA, 0x0F, 0x81, 0x30
};
static const BLEPreset ble_presets[] = {
    {"Apple iBeacon", payload_ibeacon, sizeof(payload_ibeacon)},
    {"Google Fast Pair", payload_google, sizeof(payload_google)},
    {"Samsung Proximity", payload_samsung, sizeof(payload_samsung)},
    {"Flipper Zero", payload_flipper, sizeof(payload_flipper)}
};
static const char* preset_default_macs[] = {
    "A2:00:00:00:00:4C", 
    "A2:00:00:00:FE:2C", 
    "A2:00:00:00:00:75", 
    "A2:00:00:00:0F:BA"  
};
static const size_t ble_preset_count = sizeof(ble_presets) / sizeof(ble_presets[0]);
static_assert(ble_preset_count == (sizeof(preset_default_macs) / sizeof(preset_default_macs[0])));


//helpers
void ble_host_task(void *param) {
    nimble_port_run();
    nimble_port_freertos_deinit();
}
const char* resolve_company_from_id(uint16_t id) {
    int left = 0;
    int right = company_database_count - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;

        if (ble_companies[mid].company_id == id) {
            return ble_companies[mid].manufacturer;
        }

        if (ble_companies[mid].company_id < id) {
            left = mid + 1;
        }
        else {
            right = mid - 1;
        }
    }
    return "Unknown";
}
void fill_ble_scan_details_roller(size_t index) {
    if (index >= ble_inventory_count) return;

    lvgl_port_lock(0);
    uint16_t selected_index = lv_roller_get_selected(objects.discovered_ble_devices_info);
    lvgl_port_unlock();

    BLEInventoryDevice device;
    {
        SemLock lock(mtx_handle, portMAX_DELAY);
        if (lock.locked()) {
            device = ble_inventory[index];
        }
        else return;
    }
    
    std::string roller_str = "";
    roller_str.reserve(BLEScanStrings::total_reserve_size);
    char line_buf[LINE_BUFFER_LEN];

    snprintf(line_buf, sizeof(line_buf), "%s%s\n", BLEScanStrings::name.data(), device.has_name ? device.name : "[Hidden]");
    roller_str += line_buf;

    if (device.has_tx_power) {
        snprintf(line_buf, sizeof(line_buf), "%s%d%s\n", BLEScanStrings::tx_power.data(), device.tx_power, BLEScanStrings::signal_suffix.data());
        roller_str += line_buf;
    }

    const char* addr_type_str = (device.addr_type == BLEAddrType::PUBLIC) ? "PUBLIC" : (device.addr_type == BLEAddrType::RANDOM) ? "RANDOM" : (device.addr_type == BLEAddrType::RPA_PUBLIC) ? "RPA_PUB" : "RPA_RND";
    snprintf(line_buf, sizeof(line_buf), "%s%s\n", BLEScanStrings::addr_type.data(), addr_type_str);
    roller_str += line_buf;

    snprintf(line_buf, sizeof(line_buf), "%s%s\n", BLEScanStrings::connectable.data(), device.is_connectable ? "Yes" : "No");
    roller_str += line_buf;

    if (device.company_id != 0xFFFF) {
        snprintf(line_buf, sizeof(line_buf), "%s%04X (%s)\n", BLEScanStrings::company_id.data(), device.company_id, resolve_company_from_id(device.company_id));
        roller_str += line_buf;
    }

    if (device.appearance != 0) {
        snprintf(line_buf, sizeof(line_buf), "%s%04X\n", BLEScanStrings::appearance.data(), device.appearance);
        roller_str += line_buf;
    }

    if (device.uuid_count > 0) {
        roller_str += BLEScanStrings::uuids;
        for (uint8_t i = 0; i < device.uuid_count; i++) {
            snprintf(line_buf, sizeof(line_buf), "0x%04X%s", device.uuids[i], (i < device.uuid_count - 1) ? ", " : "\n");
            roller_str += line_buf;
        }
    }

    snprintf(line_buf, sizeof(line_buf), "%s%u", BLEScanStrings::packets.data(), device.packet_count);
    roller_str += line_buf;

    int16_t raw_rssi = clamp(device.last_rssi, RSSI_MIN, RSSI_MAX);
    uint8_t percentage = static_cast<uint8_t>(raw_rssi - (RSSI_MIN)) * 100 / (RSSI_MAX - (RSSI_MIN));
    lv_color_t bar_color;

    if (percentage >= 60) {
        uint8_t mix_ratio = (percentage - 60) * 255 / 40; 
        bar_color = lv_color_mix(lv_palette_main(LV_PALETTE_GREEN), lv_palette_main(LV_PALETTE_YELLOW), mix_ratio);
    } 
    else if (percentage >= 30) {
        uint8_t mix_ratio = (percentage - 30) * 255 / 30; 
        bar_color = lv_color_mix(lv_palette_main(LV_PALETTE_YELLOW), lv_palette_main(LV_PALETTE_RED), mix_ratio);
    } 
    else {
        bar_color = lv_palette_main(LV_PALETTE_RED);
    }

    lvgl_port_lock(0);
    lv_bar_set_value(objects.bluetooth_scanner_rssi, map(device.last_rssi, -100, -20, 0, 100), LV_ANIM_OFF);
    lv_obj_set_style_bg_color(objects.bluetooth_scanner_rssi, bar_color, LV_PART_INDICATOR);
    lv_roller_set_options(objects.discovered_ble_devices_info, roller_str.c_str(), LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(objects.discovered_ble_devices_info, selected_index, LV_ANIM_OFF);
    lvgl_port_unlock();
}
void fill_ble_scan_roller_timer_cb(lv_timer_t * timer) {
    uint16_t current_count = ble_inventory_count.load(std::memory_order_acquire);
    if (current_count == 0) return;

    lvgl_port_lock(0);
    uint16_t current_selected_index = lv_roller_get_selected(objects.discovered_ble_devices);
    lvgl_port_unlock();
    
    uint8_t current_mac[6] = {};
    std::string roller_str = "";
    roller_str.reserve(current_count * 18);
    char mac_str[MAC_STR_LEN] = {};
    uint8_t mac[6];
    size_t new_selected_index = 0;

    {
        SemLock lock(mtx_handle, portMAX_DELAY);
        if (lock.locked()) {
            if (current_selected_index < current_count) {
                memcpy(current_mac, ble_inventory[current_selected_index].mac, 6);
            }

            for (size_t i = 0; i < current_count; i++) {
                memcpy(mac, ble_inventory[i].mac, 6);
                snprintf(mac_str, MAC_STR_LEN, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                roller_str += mac_str;

                if (i < current_count - 1) {
                    roller_str += '\n';
                }
            }

            if (current_selected_index < current_count) {
                for (size_t i = 0; i < current_count; i++) {
                    if (memcmp(current_mac, ble_inventory[i].mac, 6) == 0) {
                        new_selected_index = i;
                        break;
                    }
                }
            }
        }
    }

    lvgl_port_lock(0);
    lv_roller_set_options(objects.discovered_ble_devices, roller_str.c_str(), LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(objects.discovered_ble_devices, new_selected_index, LV_ANIM_OFF);
    lvgl_port_unlock();
    
    fill_ble_scan_details_roller(new_selected_index);
}
int ble_scan_cb(ble_gap_event* event, void *arg) {
    if (event->type == BLE_GAP_EVENT_DISC) {
        ble_gap_disc_desc* disc = &event->disc;

        int8_t range = scan_range.load(std::memory_order_acquire);
        if (disc->rssi < range) return 0;

        struct ble_hs_adv_fields fields;
        int rc = ble_hs_adv_parse_fields(&fields, disc->data, disc->length_data);

        size_t current_inventory_count = ble_inventory_count.load(std::memory_order_acquire);

        {
            SemLock lock(mtx_handle, 0);
            if (lock.locked()) {
                if (rc == 0) {
                    for (size_t i = 0; i < current_inventory_count; i++) {
                        if (memcmp(ble_inventory[i].mac, disc->addr.val, 6) == 0) {
                            ble_inventory[i].addr_type = disc->addr.type == BLE_ADDR_PUBLIC ? BLEAddrType::PUBLIC : disc->addr.type == BLE_ADDR_RANDOM ? BLEAddrType::RANDOM : disc->addr.type == BLE_ADDR_PUBLIC_ID ? BLEAddrType::RPA_PUBLIC : BLEAddrType::RPA_RANDOM;
                            ble_inventory[i].last_rssi = disc->rssi;
                            if (fields.tx_pwr_lvl_is_present) {
                                ble_inventory[i].has_tx_power = true;
                                ble_inventory[i].tx_power = fields.tx_pwr_lvl;
                            }
                            if (fields.name != nullptr && !ble_inventory[i].has_name) {
                                ble_inventory[i].has_name = true;
                                snprintf(ble_inventory[i].name, sizeof(ble_inventory[i].name), "%.*s", fields.name_len, fields.name);
                            }
                            if (fields.uuids16 != nullptr && fields.num_uuids16 > 0) {
                                ble_inventory[i].uuid_count = 0;
                                for (size_t j = 0; j < fields.num_uuids16 && j < MAX_UUIDS16; j++) {
                                    ble_inventory[i].uuids[j] = fields.uuids16[j].value;
                                    ble_inventory[i].uuid_count++;
                                }
                            }
                            if (fields.mfg_data != nullptr && fields.mfg_data_len >= 2) {
                                uint16_t company_id = fields.mfg_data[0] | (fields.mfg_data[1] << 8);
                                ble_inventory[i].company_id = company_id;
                                ble_inventory[i].device_type = get_device_type(company_id);
                            }
                            if (fields.appearance_is_present) {
                                ble_inventory[i].appearance = fields.appearance;
                            }
                            ble_inventory[i].is_connectable = disc->event_type == BLE_HCI_ADV_TYPE_ADV_IND ? true : false;
                            ble_inventory[i].last_seen_ms = time_ms();
                            ble_inventory[i].packet_count++;
                            ble_inventory[i].active = true;

                            return 0;
                        }
                    }

                    if (current_inventory_count >= MAX_INVENTORY_DEVICES) {
                        return 0;
                    }

                    memcpy(ble_inventory[current_inventory_count].mac, disc->addr.val, 6);
                    ble_inventory[current_inventory_count].addr_type = disc->addr.type == BLE_ADDR_PUBLIC ? BLEAddrType::PUBLIC : disc->addr.type == BLE_ADDR_RANDOM ? BLEAddrType::RANDOM : disc->addr.type == BLE_ADDR_PUBLIC_ID ? BLEAddrType::RPA_PUBLIC : BLEAddrType::RPA_RANDOM;
                    ble_inventory[current_inventory_count].last_rssi = disc->rssi;
                    if (fields.tx_pwr_lvl_is_present) {
                        ble_inventory[current_inventory_count].has_tx_power = true;
                        ble_inventory[current_inventory_count].tx_power = fields.tx_pwr_lvl;
                    }
                    if (fields.name != nullptr) {
                        ble_inventory[current_inventory_count].has_name = true;
                        snprintf(ble_inventory[current_inventory_count].name, sizeof(ble_inventory[current_inventory_count].name), "%.*s", fields.name_len, fields.name);
                    }
                    if (fields.uuids16 != nullptr && fields.num_uuids16 > 0) {
                        ble_inventory[current_inventory_count].uuid_count = 0;
                        for (size_t j = 0; j < fields.num_uuids16 && j < MAX_UUIDS16; j++) {
                            ble_inventory[current_inventory_count].uuids[j] = fields.uuids16[j].value;
                            ble_inventory[current_inventory_count].uuid_count++;
                        }
                    }
                    if (fields.mfg_data != nullptr && fields.mfg_data_len >= 2) {
                        uint16_t company_id = fields.mfg_data[0] | (fields.mfg_data[1] << 8);
                        ble_inventory[current_inventory_count].company_id = company_id;
                        ble_inventory[current_inventory_count].device_type = get_device_type(company_id);
                    }
                    if (fields.appearance_is_present) {
                        ble_inventory[current_inventory_count].appearance = fields.appearance;
                    }
                    ble_inventory[current_inventory_count].is_connectable = disc->event_type == BLE_HCI_ADV_TYPE_ADV_IND ? true : false;
                    ble_inventory[current_inventory_count].last_seen_ms = time_ms();
                    ble_inventory[current_inventory_count].packet_count = 1;
                    ble_inventory[current_inventory_count].active = true;
                    ble_inventory_count.store(current_inventory_count + 1, std::memory_order_release);
                } 
            }
        } 
    }

    return 0;
}
int ble_connection_cb(struct ble_gap_event *event, void *arg) {
    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            if (event->connect.status == 0) {
                current_conn_handle = event->connect.conn_handle;
                ble_connected.store(true, std::memory_order_release);
                
                lvgl_port_lock(0);
                lv_obj_set_style_bg_color(objects.enable_bluetooth_button, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);
                lvgl_port_unlock();
            } else {
                ble_connected.store(false, std::memory_order_release);
                current_conn_handle = BLE_HS_CONN_HANDLE_NONE;
                
                lvgl_port_lock(0);
                lv_obj_set_style_bg_color(objects.enable_bluetooth_button, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
                lvgl_port_unlock();
            }
            break;

        case BLE_GAP_EVENT_DISCONNECT:
            ble_connected.store(false, std::memory_order_release);
            current_conn_handle = BLE_HS_CONN_HANDLE_NONE;
            
            lvgl_port_lock(0);
            if (manual_ble_disconnect.load(std::memory_order_acquire)) {
                lv_obj_set_style_bg_color(objects.enable_bluetooth_button, lv_color_hex(0x747474), LV_PART_MAIN);
                manual_ble_disconnect.store(false, std::memory_order_release);
            } else {
                lv_obj_set_style_bg_color(objects.enable_bluetooth_button, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
            }
            lvgl_port_unlock();
            break;

        case BLE_GAP_EVENT_PASSKEY_ACTION:
            pending_auth_handle = event->passkey.conn_handle;
            pending_auth_action = event->passkey.params.action;

            if (event->passkey.params.action == BLE_SM_IOACT_DISP) {
                uint32_t random_pin = esp_random() % 1000000;

                char pin_str[8];
                snprintf(pin_str, 8, "%06lu", random_pin); 

                lvgl_port_lock(0);
                lv_obj_clear_flag(objects.pairing_msg_container, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(objects.accept_pairing_button, LV_OBJ_FLAG_HIDDEN); 
                lv_obj_clear_flag(objects.pairing_msg_pin, LV_OBJ_FLAG_CLICK_FOCUSABLE);
                lv_textarea_set_text(objects.pairing_msg_pin, pin_str);
                lvgl_port_unlock();

                struct ble_sm_io pk;
                pk.action = event->passkey.params.action;
                pk.passkey = random_pin;
                ble_sm_inject_io(event->passkey.conn_handle, &pk);
            } 
            else if (event->passkey.params.action == BLE_SM_IOACT_NUMCMP) {
                char pin_str[8];
                snprintf(pin_str, 8, "%06lu", event->passkey.params.numcmp);

                lvgl_port_lock(0);
                lv_obj_clear_flag(objects.pairing_msg_container, LV_OBJ_FLAG_HIDDEN);
                lv_obj_clear_flag(objects.accept_pairing_button, LV_OBJ_FLAG_HIDDEN);
                lv_obj_clear_flag(objects.pairing_msg_pin, LV_OBJ_FLAG_CLICK_FOCUSABLE);
                lv_textarea_set_text(objects.pairing_msg_pin, pin_str); 
                lvgl_port_unlock();
            }
            else if (event->passkey.params.action == BLE_SM_IOACT_INPUT) {
                lvgl_port_lock(0);
                lv_obj_clear_flag(objects.pairing_msg_container, LV_OBJ_FLAG_HIDDEN);
                lv_obj_clear_flag(objects.accept_pairing_button, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(objects.pairing_msg_pin, LV_OBJ_FLAG_CLICK_FOCUSABLE);
                lv_textarea_set_text(objects.pairing_msg_pin, ""); 
                lvgl_port_unlock();
            }
            break;
    }
    return 0;
}
int ble_spoofer_gap_event(struct ble_gap_event *event, void *arg) {
    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            break;
        case BLE_GAP_EVENT_DISCONNECT:
            break;
    }
    return 0;
}
void start_ble_advertising(const uint8_t* raw_payload, uint8_t payload_len) {
    if (ble_gap_adv_active()) {
        ble_gap_adv_stop();
    }

    int rc = ble_gap_adv_set_data(raw_payload, payload_len);
    if (rc != 0) return;

    struct ble_gap_adv_params adv_params = {};

    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND; 
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN; 
    adv_params.itvl_min = static_cast<uint16_t>(BLE_AD_INTERVAL_MIN_MS / 0.625); 
    adv_params.itvl_max = static_cast<uint16_t>(BLE_AD_INTERVAL_MAX_MS / 0.625);  

    ble_gap_adv_start(BLE_OWN_ADDR_RANDOM, NULL, BLE_HS_FOREVER, &adv_params, ble_spoofer_gap_event, NULL);
}
void init_bluetooth_menu() {
    mtx_handle = xSemaphoreCreateMutex();

    uint8_t mac[6];
    char mac_str[MAC_STR_LEN];
    ble_hs_id_copy_addr(BLE_ADDR_RANDOM, mac, NULL);
    snprintf(mac_str, MAC_STR_LEN, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    lv_label_set_text(objects.bt_current_mac_label, mac_str);

    std::string roller_str = "";
    roller_str.reserve((MANUFACTURER_STR_LEN * ble_preset_count) + ble_preset_count - 1);
    for (size_t i = 0; i < ble_preset_count; i++) {
        roller_str += ble_presets[i].name;

        if (i < ble_preset_count - 1) {
            roller_str += '\n';
        }
    }
    lv_roller_set_options(objects.pick_bluetooth_mac_to_spoof_roller, roller_str.c_str(), LV_ROLLER_MODE_NORMAL);
}


//variables
int32_t get_var_bluetooth_option() { return bluetooth_option; }
void set_var_bluetooth_option(int32_t value) { bluetooth_option = value; }


//actions
void action_change_bluetooth_option(lv_event_t * e) {
    lv_obj_t* bluetooth_options[] = {
        objects.bluetooth_scanner_container,
        objects.bluetooth_spoofer_container,
        objects.bluetooth_jammer_container
    };
    size_t num_bluetooth_options = sizeof(bluetooth_options) / sizeof(lv_obj_t*);

    uint16_t option = lv_dropdown_get_selected(objects.bluetooth_option_dropdown);

    for (size_t i = 0; i < num_bluetooth_options; i++) {
        if (i == option) {
            lv_obj_clear_flag(bluetooth_options[i], LV_OBJ_FLAG_HIDDEN);
        }
        else {
            lv_obj_add_flag(bluetooth_options[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
}
void action_scan_bluetooth(lv_event_t * e) {
    if (!is_scanning.load(std::memory_order_acquire)) {
        ble_gap_disc_params disc_params = {};

        is_scanning.store(true, std::memory_order_release);
        ble_inventory_count.store(0, std::memory_order_release);

        lvgl_port_lock(0);
        lv_roller_set_options(objects.discovered_ble_devices, "", LV_ANIM_OFF);
        lv_roller_set_options(objects.discovered_ble_devices_info, "", LV_ANIM_OFF);
        lvgl_port_unlock();

        ble_gap_disc(BLE_OWN_ADDR_PUBLIC, BLE_HS_FOREVER, &disc_params, ble_scan_cb, nullptr);
        scan_timer = lv_timer_create(fill_ble_scan_roller_timer_cb, 500, nullptr);

        lv_obj_set_style_bg_color(objects.scan_bluetooth_button, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);
    }
    else {
        ble_gap_disc_cancel();
        lv_timer_del(scan_timer);
        is_scanning.store(false, std::memory_order_release);
        lv_obj_set_style_bg_color(objects.scan_bluetooth_button, lv_color_hex(0x2196F3), LV_PART_MAIN);
    }
}
void action_select_bluetooth_device(lv_event_t * e) {
    size_t index = lv_roller_get_selected(objects.discovered_ble_devices);
    fill_ble_scan_details_roller(index);
}
void action_adjust_bluetooth_scan_range(lv_event_t * e) {
    int8_t range = static_cast<int8_t>(lv_slider_get_value(objects.scan_bluetooth_filter_slider));
    scan_range.store(range, std::memory_order_release);

    char range_str[8];
    snprintf(range_str, 8, "%hhddBm", range);
    lv_label_set_text(objects.ble_scan_range_label, range_str);
}
void action_toggle_bluetooth(lv_event_t * e) {
    if (ble_connected.load(std::memory_order_acquire)) {
        manual_ble_disconnect.store(true, std::memory_order_release);
        ble_gap_terminate(current_conn_handle, BLE_ERR_REM_USER_CONN_TERM);
        return; 
    }

    lvgl_port_lock(0);
    uint16_t index = lv_roller_get_selected(objects.discovered_ble_devices);
    lv_obj_set_style_bg_color(objects.enable_bluetooth_button, lv_palette_main(LV_PALETTE_YELLOW), LV_PART_MAIN);
    lvgl_port_unlock();

    BLEInventoryDevice target_device;
    {
        SemLock lock(mtx_handle, portMAX_DELAY);
        if (lock.locked() && index < ble_inventory_count.load(std::memory_order_acquire)) {
            target_device = ble_inventory[index];
        } else {
            lvgl_port_lock(0);
            lv_obj_set_style_bg_color(objects.enable_bluetooth_button, lv_color_hex(0x747474), LV_PART_MAIN);
            lvgl_port_unlock();
            return;
        }
    }

    if (is_scanning.load(std::memory_order_acquire)) {
        ble_gap_disc_cancel();
        lv_timer_del(scan_timer);
        is_scanning.store(false, std::memory_order_release);
        
        lvgl_port_lock(0);
        lv_obj_set_style_bg_color(objects.scan_bluetooth_button, lv_color_hex(0x2196F3), LV_PART_MAIN);
        lvgl_port_unlock();
    }

    ble_addr_t peer_addr;
    peer_addr.type = (target_device.addr_type == BLEAddrType::PUBLIC) ? BLE_ADDR_PUBLIC : (target_device.addr_type == BLEAddrType::RANDOM) ? BLE_ADDR_RANDOM : (target_device.addr_type == BLEAddrType::RPA_PUBLIC) ? BLE_ADDR_PUBLIC_ID : BLE_ADDR_RANDOM_ID;
    memcpy(peer_addr.val, target_device.mac, 6);

    int rc = ble_gap_connect(BLE_OWN_ADDR_PUBLIC, &peer_addr, 10000, NULL, ble_connection_cb, NULL);
    
    if (rc != 0) {
        lvgl_port_lock(0);
        lv_obj_set_style_bg_color(objects.enable_bluetooth_button, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
        lvgl_port_unlock();
    }
}
void action_close_pairing_msg_box(lv_event_t * e) {
    if (pending_auth_handle != BLE_HS_CONN_HANDLE_NONE) {
        if (pending_auth_action == BLE_SM_IOACT_NUMCMP) {
            struct ble_sm_io pk;
            pk.action = pending_auth_action;
            pk.numcmp_accept = 0; 
            ble_sm_inject_io(pending_auth_handle, &pk);
        } else {
            ble_gap_terminate(pending_auth_handle, BLE_ERR_REM_USER_CONN_TERM);
        }
        pending_auth_handle = BLE_HS_CONN_HANDLE_NONE;
    }
    
    lv_obj_add_flag(objects.pairing_msg_container, LV_OBJ_FLAG_HIDDEN);
}
void action_accept_pairing(lv_event_t * e) {
    if (pending_auth_handle == BLE_HS_CONN_HANDLE_NONE) return;

    struct ble_sm_io pk;
    pk.action = pending_auth_action;

    if (pending_auth_action == BLE_SM_IOACT_NUMCMP) {
        pk.numcmp_accept = 1; 
    } 
    else if (pending_auth_action == BLE_SM_IOACT_INPUT) {
        const char* pin_text = lv_textarea_get_text(objects.pairing_msg_pin);
        uint32_t pin = 0;
        
        if (pin_text != nullptr && strlen(pin_text) > 0) {
            sscanf(pin_text, "%lu", &pin);
        }
        pk.passkey = pin;
    }

    ble_sm_inject_io(pending_auth_handle, &pk);

    lv_obj_add_flag(objects.pairing_msg_container, LV_OBJ_FLAG_HIDDEN);
    pending_auth_handle = BLE_HS_CONN_HANDLE_NONE;
}
void action_enforce_btmac_format(lv_event_t * e) {
    if (updating_mac) return;
    updating_mac = true;
    const char* mac_text = lv_textarea_get_text(objects.bt_spoof_mac_textarea);
    
    char mac[18] = {};
    int mac_idx = 0;
    int hex_count = 0;

    for (size_t i = 0; mac_text[i] != '\0' && hex_count < 12; i++) {
        char c = mac_text[i];

        if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
            if (hex_count > 0 && hex_count % 2 == 0) {
                mac[mac_idx++] = ':';
            }
            
            if (c >= 'a' && c <= 'f') {
                mac[mac_idx++] = c - 32;
            } else {
                mac[mac_idx++] = c;
            }
            
            hex_count++;
        }
    }
    mac[mac_idx] = '\0';

    if (strcmp(mac_text, mac) != 0) {
        uint32_t cursor_pos = lv_textarea_get_cursor_pos(objects.bt_spoof_mac_textarea);

        lv_textarea_set_text(objects.bt_spoof_mac_textarea, mac);

        if (mac_idx > 0 && cursor_pos > 0 && mac[cursor_pos - 1] == ':') {
            cursor_pos++;
        }

        lv_textarea_set_cursor_pos(objects.bt_spoof_mac_textarea, cursor_pos);
    }

    updating_mac = false;
}
void action_toggle_bluetooth_mac_spoofing(lv_event_t * e) {
    bool toggle_spoofing = lv_obj_has_state(objects.toggle_bluetooth_mac_spoofing_switch, LV_STATE_CHECKED);
    if (toggle_spoofing) {
        const char* mac_str = lv_textarea_get_text(objects.bt_spoof_mac_textarea);
        uint8_t spoof_mac[6];
        
        if (strlen(mac_str) != 17 || sscanf(mac_str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &spoof_mac[0], &spoof_mac[1], &spoof_mac[2], &spoof_mac[3], &spoof_mac[4], &spoof_mac[5]) != 6) {
            lv_obj_clear_state(objects.toggle_bluetooth_mac_spoofing_switch, LV_STATE_CHECKED);
            return;
        }

        spoof_mac[0] = (spoof_mac[0] & 0xFC) | 0x02; 
        
        int rc = ble_hs_id_set_rnd(spoof_mac);
        if (rc != 0) {
            lv_obj_clear_state(objects.toggle_bluetooth_mac_spoofing_switch, LV_STATE_CHECKED);
            return;
        }

        uint16_t index = lv_roller_get_selected(objects.pick_bluetooth_mac_to_spoof_roller);
        if (index >= ble_preset_count) {
            lv_obj_clear_state(objects.toggle_bluetooth_mac_spoofing_switch, LV_STATE_CHECKED);
            return;
        }

        if (is_scanning.load(std::memory_order_acquire)) {
            ble_gap_disc_cancel();
            lv_timer_del(scan_timer);
            is_scanning.store(false, std::memory_order_release);
            lv_obj_set_style_bg_color(objects.scan_bluetooth_button, lv_color_hex(0x2196F3), LV_PART_MAIN);
        }

        start_ble_advertising(ble_presets[index].payload, ble_presets[index].payload_len);
        spoofer_active.store(true, std::memory_order_release);
    }
    else {
        if (ble_gap_adv_active()) {
            ble_gap_adv_stop();
        }
        spoofer_active.store(false, std::memory_order_release);
    }

    uint8_t mac[6];
    char mac_str[MAC_STR_LEN];
    ble_hs_id_copy_addr(BLE_ADDR_RANDOM, mac, NULL);

    snprintf(mac_str, MAC_STR_LEN, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    lv_label_set_text(objects.bt_current_mac_label, mac_str);
}
void action_randomize_bluetooth_mac(lv_event_t * e) {
    uint8_t mac[6];

    for (int i = 0; i < 6; i++) {
        mac[i] = esp_random() & 0xFF;
    }

    char mac_str[MAC_STR_LEN] = {};
    snprintf(mac_str, MAC_STR_LEN, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    lv_textarea_set_text(objects.bt_spoof_mac_textarea, mac_str);
}
void action_select_bt_preset_mac(lv_event_t * e) {
    uint16_t index = lv_roller_get_selected(objects.pick_bluetooth_mac_to_spoof_roller);
    
    if (index < ble_preset_count) {
        updating_mac = true; 
        lv_textarea_set_text(objects.bt_spoof_mac_textarea, preset_default_macs[index]);
        updating_mac = false;
    }
}
