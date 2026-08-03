#pragma once

#include "header.hpp"
#include "ui_bridge.hpp"

inline constexpr uint8_t DEFAULT_KEY[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
inline constexpr uint8_t KEY_A = 0x60;
inline constexpr uint8_t KEY_B = 0x61;
inline constexpr size_t NFC_CHUNK_LEN = 16;
inline constexpr size_t SAVE_QUEUE_SIZE = 16;
inline constexpr size_t BUFFER_SIZE = 32;
inline constexpr size_t PREVIEW_BYTES = 64;
inline constexpr uint8_t PN532_RETRIES = 0x14;
inline constexpr uint32_t PN532_POLL_INT = 500;
inline constexpr size_t PAYLOAD_BUFFER_SIZE = 8192;

namespace TagPreviewStrings {
    static constexpr std::string_view uid = "UID: "; //+30 characters uid
    static constexpr std::string_view type = "Type: "; //+32 characters type
    static constexpr std::string_view blocks = "Blocks: "; //+4 characters num blocks
    static constexpr std::string_view size = "Size: "; //+5 characters size
    static constexpr std::string_view size_suffix = " bytes";
    static constexpr std::string_view separator = "-------------------------"; 
    static constexpr std::string_view preview = "Payload Preview:";

    static constexpr size_t uid_len = uid.size() + 30;
    static constexpr size_t type_len = type.size() + 32;
    static constexpr size_t blocks_len = blocks.size() + 4;
    static constexpr size_t size_len = size.size() + 5 + size_suffix.size();
    static constexpr size_t separator_len = separator.size();
    static constexpr size_t preview_len = preview.size();

    static constexpr size_t num_newlines = 6;

    static constexpr size_t total_reserve_size = uid_len + type_len + blocks_len + size_len + separator_len + preview_len + num_newlines + (PREVIEW_BYTES * 3);
};

struct nfc_save_chunk {
    pn532_uid_t uid;
    uint16_t blocks;
    uint16_t block_size;
    char filename[FILENAME_LEN];
};

inline int32_t nfc_option;

inline bool sak_is_legacy_mifare(uint8_t sak) { return (sak & 0x20) == 0x00 && (sak & 0x08) == 0x08; }
inline bool sak_is_iso14443_4(uint8_t sak) { return (sak & 0x20) == 0x20; }
inline bool sak_is_ultralight(uint8_t sak) { return sak == 0x00; }
inline int8_t mifare_family(uint8_t byte2) {
    switch (byte2 & 0x0F) { 
        case 0x2: return 1;  //mifare plus
        case 0x1: return 0;  //desfire/duox
        case 0x3: return 0;  //ultralight
        case 0x4: return 0;  //ntag
        case 0x7: return 0;  //ntag i2c
        case 0x8: return 0;  //desfire light
        default:  return -1;
    }
}
inline std::string_view get_tag_type(uint8_t sak, uint16_t blocks) {
    switch (sak) {
        case 0x08: return "MIFARE Classic 1K";
        case 0x18: return "MIFARE Classic 4K";
        case 0x00: 
            if (blocks == 135) return "NTAG215 (Amiibo)";
            if (blocks == 231) return "NTAG216";
            return "MIFARE Ultralight / NTAG";
        case 0x20: return "MIFARE DESFire";
        default:   return "Unknown Tag";
    }
}

void init_nfc_menu(pn532_t* dev);