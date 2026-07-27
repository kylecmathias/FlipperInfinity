#pragma once

#include "header.hpp"
#include "ui_bridge.hpp"

enum class ScanContext {
    CONNECT_TO_A_NETWORK,
    FIND_LOGGER_TARGET
};

enum class DeviceType : uint8_t {
    UNKNOWN = 0,
    ACCESS_POINT,
    CLIENT,
    BOTH
};

struct InventoryDevice {
    uint8_t mac[6];
    int8_t last_rssi;
    char manufacturer[32]; 
    bool has_manufacturer;

    uint8_t ap_mac[6];
    bool has_ap;
    DeviceType device_type;

    uint8_t channel;
    bool has_channel;

    uint32_t last_seen_ms;         
    uint16_t packet_count;         
    bool active;
};

struct OUIEntry {
    uint32_t oui_prefix; 
    char manufacturer[32]; 
};

inline size_t num_wifi_protocols = 5;

inline int32_t wifi_option;
inline int32_t network_found_ssid;
inline wifi_auth_mode_t scanned_protocols[MAX_SCAN_NETWORKS]; 
inline uint16_t total_scanned_networks = 0;
inline int32_t device_inventory_signal_strength;

inline constexpr uint8_t broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
inline constexpr uint8_t empty[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

inline bool is_broadcast_mac(const uint8_t* mac) {
    return memcmp(mac, broadcast, 6) == 0;
}

inline bool is_empty_mac(const uint8_t* mac) {
    return memcmp(mac, empty, 6) == 0;
}

inline bool is_known_ap(const uint8_t* mac, uint16_t inventory_count, InventoryDevice inventory_list[]) {
    for (uint16_t i = 0; i < inventory_count; i++) {
        if (memcmp(inventory_list[i].mac, mac, 6) == 0) {
            return inventory_list[i].device_type == DeviceType::ACCESS_POINT || inventory_list[i].device_type == DeviceType::BOTH;
        }
    }
    return false;
}

void wifi_scan_done_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
void wifi_connection_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

void init_psram_oui_database();
void init_wifi_menu();
