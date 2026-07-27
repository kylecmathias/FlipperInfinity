#pragma once

#include "header.hpp"
#include "ui_bridge.hpp"

enum class BLEAddrType : uint8_t {
    PUBLIC = 0,
    RANDOM = 1,
    RPA_PUBLIC = 2,
    RPA_RANDOM = 3
};

enum class BLEDeviceType : uint8_t {
    UNKNOWN = 0,
    APPLE,
    SAMSUNG,
    MICROSLOP,
    GOOGLE,
    FLIPPER,
    GENERIC_LE
};

enum class BLEPayloadType {
    IBEACON,
    GOOGLE_FAST_PAIR,
    SAMSUNG_PROXIMITY,
    FLIPPER_PERIPHERAL,
    CLONED_DEVICE
};

struct BLEPreset {
    const char* name;
    const uint8_t* payload;
    size_t payload_len;
};

struct BLEInventoryDevice {
    uint8_t mac[6];
    BLEAddrType addr_type;

    int8_t last_rssi;
    bool has_tx_power;
    int8_t tx_power;

    bool has_name;
    char name[HOSTNAME_LEN];
    
    BLEDeviceType device_type;
    uint16_t company_id;
    uint16_t appearance;

    uint8_t uuid_count;
    uint16_t uuids[MAX_UUIDS16];

    bool is_connectable;
    uint32_t last_seen_ms;
    uint16_t packet_count;
    bool active;
};

namespace BLEScanStrings {
    static constexpr std::string_view name = "Name: "; //+32 characters name
    static constexpr std::string_view tx_power = "TX Power: "; //+4 characters signed dbm
    static constexpr std::string_view signal_suffix = " dBm"; //suffix to both rssi and tx power
    static constexpr std::string_view addr_type = "Addr: "; //+10 characters {PUBLIC, RANDOM, RPA_PUBLIC, RPA_RANDOM}
    static constexpr std::string_view connectable = "Connectable: "; //+3 characters connectable {Yes, No}
    static constexpr std::string_view company_id = "Company: 0x"; //+4 characters hex id +2 characters { (} +32 characters company name +1 character {)}
    static constexpr std::string_view appearance = "Appearance: 0x"; //+4 characters hex appearance
    static constexpr std::string_view uuids = "UUIDs: "; //+31 characters hex in the format 0xXXXX, 
    static constexpr std::string_view packets = "Packets: "; //5 characters decimal up to uint16_t max

    static constexpr size_t name_len = name.size() + 32;
    static constexpr size_t tx_power_len = tx_power.size() + 4;
    static constexpr size_t signal_suffix_len = signal_suffix.size();
    static constexpr size_t addr_type_len = addr_type.size() + 10;
    static constexpr size_t connectable_len = connectable.size() + 3;
    static constexpr size_t company_id_len = company_id.size() + 4 + 2 + 32 + 1;
    static constexpr size_t appearance_len = appearance.size() + 4;
    static constexpr size_t uuids_len = uuids.size() + 31;
    static constexpr size_t packets_len = packets.size() + 5;

    static constexpr size_t num_newlines = 9; //for the \n after each field (except signal_suffix)

    static constexpr size_t total_reserve_size = name_len + tx_power_len + signal_suffix_len + addr_type_len + connectable_len + company_id_len + appearance_len + uuids_len + packets_len + num_newlines;
};

struct BLECompanyEntry {
    uint16_t company_id;
    const char* manufacturer;
};

inline int32_t bluetooth_option;

inline BLEDeviceType get_device_type(uint16_t company_id) {
    switch(company_id) {
        case 0x004C: 
            return BLEDeviceType::APPLE;
        case 0x0075:
            return BLEDeviceType::SAMSUNG;
        case 0x0006:
            return BLEDeviceType::MICROSLOP;
        case 0x00E0:
            return BLEDeviceType::GOOGLE;
        default:
            return BLEDeviceType::GENERIC_LE;
    }
}

void ble_host_task(void *param);
void init_bluetooth_menu();