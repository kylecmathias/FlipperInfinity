#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_LOGO = 1,
    SCREEN_ID_HOME_MENU = 2,
    SCREEN_ID_WIFI_MENU = 3,
    SCREEN_ID_BLUETOOTH_MENU = 4,
    SCREEN_ID_NFC_MENU = 5,
    SCREEN_ID_RFID_MENU = 6,
    SCREEN_ID_IR_MENU = 7,
    SCREEN_ID_RF_MENU = 8,
    _SCREEN_ID_LAST = 8
};

typedef struct _objects_t {
    lv_obj_t *logo;
    lv_obj_t *home_menu;
    lv_obj_t *wifi_menu;
    lv_obj_t *bluetooth_menu;
    lv_obj_t *nfc_menu;
    lv_obj_t *rfid_menu;
    lv_obj_t *ir_menu;
    lv_obj_t *rf_menu;
    lv_obj_t *home_menu_status_bar;
    lv_obj_t *home_menu_status_bar__status_bar_container;
    lv_obj_t *home_menu_status_bar__wifi_status;
    lv_obj_t *home_menu_status_bar__bluetooth_status;
    lv_obj_t *home_menu_status_bar__battery_bar;
    lv_obj_t *home_menu_status_bar__battery_percentage;
    lv_obj_t *wifi_menu_button;
    lv_obj_t *bluetooth_menu_button;
    lv_obj_t *nfc_menu_button;
    lv_obj_t *rfid_menu_button;
    lv_obj_t *ir_menu_button;
    lv_obj_t *rf_menu_button;
    lv_obj_t *wifi_menu_status_bar;
    lv_obj_t *wifi_menu_status_bar__status_bar_container;
    lv_obj_t *wifi_menu_status_bar__wifi_status;
    lv_obj_t *wifi_menu_status_bar__bluetooth_status;
    lv_obj_t *wifi_menu_status_bar__battery_bar;
    lv_obj_t *wifi_menu_status_bar__battery_percentage;
    lv_obj_t *back_button_main;
    lv_obj_t *wifi_option_dropdown;
    lv_obj_t *connect_to_anetwork_container;
    lv_obj_t *scan_networks_button;
    lv_obj_t *enable_wifi_button;
    lv_obj_t *wifissid_text_area;
    lv_obj_t *wifi_password_text_area;
    lv_obj_t *wifi_protocol_dropdown;
    lv_obj_t *discovered_ssids;
    lv_obj_t *packet_logging_container;
    lv_obj_t *toggle_packet_logging_switch;
    lv_obj_t *packet_logging_ssid;
    lv_obj_t *packet_logging_chart;
    lv_obj_t *device_inventory_container;
    lv_obj_t *start_device_inventory_switch;
    lv_obj_t *device_inventory_toggle_manufacturer;
    lv_obj_t *send_mac_to_deauth_button;
    lv_obj_t *inventoried_devices_roller;
    lv_obj_t *rssi_bar;
    lv_obj_t *mac_spoofer_container;
    lv_obj_t *mac_to_spoof_textarea;
    lv_obj_t *toggle_mac_spoofing_switch;
    lv_obj_t *current_mac_label;
    lv_obj_t *randomize_spoof_mac_button;
    lv_obj_t *toggle_macs_to_spoof_switch;
    lv_obj_t *pick_mac_to_spoof_roller;
    lv_obj_t *toggle_mac_rotation_switch;
    lv_obj_t *mac_rotation_time_textarea;
    lv_obj_t *deauthenticator_container;
    lv_obj_t *deauthenticate_mac_textarea;
    lv_obj_t *deauth_all_checkbox;
    lv_obj_t *deauth_frequency_arc;
    lv_obj_t *deauthenticate_button;
    lv_obj_t *deauthenticate_button_label;
    lv_obj_t *keyboard_wifi;
    lv_obj_t *bluetooth_menu_status_bar;
    lv_obj_t *bluetooth_menu_status_bar__status_bar_container;
    lv_obj_t *bluetooth_menu_status_bar__wifi_status;
    lv_obj_t *bluetooth_menu_status_bar__bluetooth_status;
    lv_obj_t *bluetooth_menu_status_bar__battery_bar;
    lv_obj_t *bluetooth_menu_status_bar__battery_percentage;
    lv_obj_t *back_button_main_1;
    lv_obj_t *bluetooth_option_dropdown;
    lv_obj_t *bluetooth_scanner_container;
    lv_obj_t *scan_bluetooth_button;
    lv_obj_t *enable_bluetooth_button;
    lv_obj_t *scan_bluetooth_filter_slider;
    lv_obj_t *ble_scan_range_label;
    lv_obj_t *discovered_ble_devices;
    lv_obj_t *discovered_ble_devices_info;
    lv_obj_t *bluetooth_scanner_rssi;
    lv_obj_t *pairing_msg_container;
    lv_obj_t *close_pairing_container;
    lv_obj_t *accept_pairing_button;
    lv_obj_t *pairing_msg_pin;
    lv_obj_t *bluetooth_spoofer_container;
    lv_obj_t *bt_spoof_mac_textarea;
    lv_obj_t *toggle_bluetooth_mac_spoofing_switch;
    lv_obj_t *bt_current_mac_label;
    lv_obj_t *randomize_bt_spoof_mac_button;
    lv_obj_t *pick_bluetooth_mac_to_spoof_roller;
    lv_obj_t *bluetooth_wifi_bridging_container;
    lv_obj_t *bluetooth_jammer_container;
    lv_obj_t *keyboard_bluetooth;
    lv_obj_t *nfc_menu_status_bar;
    lv_obj_t *nfc_menu_status_bar__status_bar_container;
    lv_obj_t *nfc_menu_status_bar__wifi_status;
    lv_obj_t *nfc_menu_status_bar__bluetooth_status;
    lv_obj_t *nfc_menu_status_bar__battery_bar;
    lv_obj_t *nfc_menu_status_bar__battery_percentage;
    lv_obj_t *back_button_main_2;
    lv_obj_t *rfid_menu_status_bar;
    lv_obj_t *rfid_menu_status_bar__status_bar_container;
    lv_obj_t *rfid_menu_status_bar__wifi_status;
    lv_obj_t *rfid_menu_status_bar__bluetooth_status;
    lv_obj_t *rfid_menu_status_bar__battery_bar;
    lv_obj_t *rfid_menu_status_bar__battery_percentage;
    lv_obj_t *back_button_main_3;
    lv_obj_t *ir_menu_status_bar;
    lv_obj_t *ir_menu_status_bar__status_bar_container;
    lv_obj_t *ir_menu_status_bar__wifi_status;
    lv_obj_t *ir_menu_status_bar__bluetooth_status;
    lv_obj_t *ir_menu_status_bar__battery_bar;
    lv_obj_t *ir_menu_status_bar__battery_percentage;
    lv_obj_t *back_button_main_4;
    lv_obj_t *rf_menu_status_bar;
    lv_obj_t *rf_menu_status_bar__status_bar_container;
    lv_obj_t *rf_menu_status_bar__wifi_status;
    lv_obj_t *rf_menu_status_bar__bluetooth_status;
    lv_obj_t *rf_menu_status_bar__battery_bar;
    lv_obj_t *rf_menu_status_bar__battery_percentage;
    lv_obj_t *back_button_main_5;
} objects_t;

extern objects_t objects;

void create_screen_logo();
void tick_screen_logo();

void create_screen_home_menu();
void tick_screen_home_menu();

void create_screen_wifi_menu();
void tick_screen_wifi_menu();

void create_screen_bluetooth_menu();
void tick_screen_bluetooth_menu();

void create_screen_nfc_menu();
void tick_screen_nfc_menu();

void create_screen_rfid_menu();
void tick_screen_rfid_menu();

void create_screen_ir_menu();
void tick_screen_ir_menu();

void create_screen_rf_menu();
void tick_screen_rf_menu();

void create_user_widget_status_bar(lv_obj_t *parent_obj, int startWidgetIndex);
void tick_user_widget_status_bar(int startWidgetIndex);

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/