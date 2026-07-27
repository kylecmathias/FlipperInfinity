#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

objects_t objects;

//
// Event handlers
//

lv_obj_t *tick_value_change_obj;

static void event_handler_cb_wifi_menu_wifi_option_dropdown(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target(e);
        if (tick_value_change_obj != ta) {
            int32_t value = lv_dropdown_get_selected(ta);
            set_var_wifi_option(value);
        }
    }
}

static void event_handler_cb_wifi_menu_discovered_ssids(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target(e);
        if (tick_value_change_obj != ta) {
            int32_t value = lv_roller_get_selected(ta);
            set_var_network_found_ssid(value);
        }
    }
}

static void event_handler_cb_bluetooth_menu_bluetooth_option_dropdown(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target(e);
        if (tick_value_change_obj != ta) {
            int32_t value = lv_dropdown_get_selected(ta);
            set_var_bluetooth_option(value);
        }
    }
}

static void event_handler_cb_bluetooth_menu_discovered_ble_devices(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target(e);
        if (tick_value_change_obj != ta) {
            int32_t value = lv_roller_get_selected(ta);
            set_var_network_found_ssid(value);
        }
    }
}

//
// Screens
//

void create_screen_logo() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.logo = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 240, 320);
    lv_obj_add_event_cb(obj, action_change_screen_delay, LV_EVENT_SCREEN_LOADED, (void *)0);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 20, 66);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_logo);
        }
    }
    
    tick_screen_logo();
}

void tick_screen_logo() {
}

void create_screen_home_menu() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.home_menu = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 240, 320);
    {
        lv_obj_t *parent_obj = obj;
        {
            // HomeMenuStatusBar
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.home_menu_status_bar = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 240, 320);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            create_user_widget_status_bar(obj, 9);
        }
        {
            // WifiMenuButton
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.wifi_menu_button = obj;
            lv_obj_set_pos(obj, 10, 24);
            lv_obj_set_size(obj, 70, 70);
            lv_obj_add_event_cb(obj, action_change_screen, LV_EVENT_CLICKED, (void *)3);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_img_create(parent_obj);
                    lv_obj_set_pos(obj, -13, -8);
                    lv_obj_set_size(obj, 70, 70);
                    lv_img_set_src(obj, &img_wifi);
                    lv_img_set_zoom(obj, 80);
                    lv_img_set_size_mode(obj, LV_IMG_SIZE_MODE_REAL);
                }
            }
        }
        {
            // BluetoothMenuButton
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.bluetooth_menu_button = obj;
            lv_obj_set_pos(obj, 85, 24);
            lv_obj_set_size(obj, 70, 70);
            lv_obj_add_event_cb(obj, action_change_screen, LV_EVENT_CLICKED, (void *)4);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_img_create(parent_obj);
                    lv_obj_set_pos(obj, -13, -8);
                    lv_obj_set_size(obj, 70, 70);
                    lv_img_set_src(obj, &img_bluetooth);
                    lv_img_set_zoom(obj, 80);
                    lv_img_set_size_mode(obj, LV_IMG_SIZE_MODE_REAL);
                }
            }
        }
        {
            // NFCMenuButton
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.nfc_menu_button = obj;
            lv_obj_set_pos(obj, 160, 24);
            lv_obj_set_size(obj, 70, 70);
            lv_obj_add_event_cb(obj, action_change_screen, LV_EVENT_CLICKED, (void *)5);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_img_create(parent_obj);
                    lv_obj_set_pos(obj, -13, -8);
                    lv_obj_set_size(obj, 70, 70);
                    lv_img_set_src(obj, &img_nfc);
                    lv_img_set_zoom(obj, 80);
                    lv_img_set_size_mode(obj, LV_IMG_SIZE_MODE_REAL);
                }
            }
        }
        {
            // RFIDMenuButton
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.rfid_menu_button = obj;
            lv_obj_set_pos(obj, 10, 102);
            lv_obj_set_size(obj, 70, 70);
            lv_obj_add_event_cb(obj, action_change_screen, LV_EVENT_CLICKED, (void *)6);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_img_create(parent_obj);
                    lv_obj_set_pos(obj, -13, -8);
                    lv_obj_set_size(obj, 70, 70);
                    lv_img_set_src(obj, &img_rfid);
                    lv_img_set_zoom(obj, 80);
                    lv_img_set_size_mode(obj, LV_IMG_SIZE_MODE_REAL);
                }
            }
        }
        {
            // IRMenuButton
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.ir_menu_button = obj;
            lv_obj_set_pos(obj, 85, 102);
            lv_obj_set_size(obj, 70, 70);
            lv_obj_add_event_cb(obj, action_change_screen, LV_EVENT_CLICKED, (void *)7);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_img_create(parent_obj);
                    lv_obj_set_pos(obj, -13, -8);
                    lv_obj_set_size(obj, 70, 70);
                    lv_img_set_src(obj, &img_ir);
                    lv_img_set_zoom(obj, 80);
                    lv_img_set_size_mode(obj, LV_IMG_SIZE_MODE_REAL);
                }
            }
        }
        {
            // RFMenuButton
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.rf_menu_button = obj;
            lv_obj_set_pos(obj, 160, 102);
            lv_obj_set_size(obj, 70, 70);
            lv_obj_add_event_cb(obj, action_change_screen, LV_EVENT_CLICKED, (void *)8);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_img_create(parent_obj);
                    lv_obj_set_pos(obj, -13, -8);
                    lv_obj_set_size(obj, 70, 70);
                    lv_img_set_src(obj, &img_rf);
                    lv_img_set_zoom(obj, 80);
                    lv_img_set_size_mode(obj, LV_IMG_SIZE_MODE_REAL);
                }
            }
        }
    }
    
    tick_screen_home_menu();
}

void tick_screen_home_menu() {
    tick_user_widget_status_bar(9);
}

void create_screen_wifi_menu() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.wifi_menu = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 240, 320);
    {
        lv_obj_t *parent_obj = obj;
        {
            // WifiMenuStatusBar
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.wifi_menu_status_bar = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 240, 20);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            create_user_widget_status_bar(obj, 21);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 80, 21);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_decor(obj, LV_TEXT_DECOR_UNDERLINE, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "WIFI MENU");
        }
        {
            // BackButtonMain
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.back_button_main = obj;
            lv_obj_set_pos(obj, 4, 20);
            lv_obj_set_size(obj, 31, 18);
            lv_obj_add_event_cb(obj, action_change_screen, LV_EVENT_CLICKED, (void *)2);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x545454), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // WifiOptionDropdown
            lv_obj_t *obj = lv_dropdown_create(parent_obj);
            objects.wifi_option_dropdown = obj;
            lv_obj_set_pos(obj, 5, 42);
            lv_obj_set_size(obj, 231, LV_SIZE_CONTENT);
            lv_dropdown_set_options_static(obj, "Connect to a Network\nPacket Logging\nDevice Inventory\nMAC Spoofer\nDeauthenticator");
            lv_obj_add_event_cb(obj, action_change_wifi_option, LV_EVENT_VALUE_CHANGED, (void *)0);
            lv_obj_add_event_cb(obj, event_handler_cb_wifi_menu_wifi_option_dropdown, LV_EVENT_ALL, 0);
        }
        {
            // ConnectToANetworkContainer
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.connect_to_anetwork_container = obj;
            lv_obj_set_pos(obj, 0, 78);
            lv_obj_set_size(obj, 240, 242);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // ScanNetworksButton
                    lv_obj_t *obj = lv_btn_create(parent_obj);
                    objects.scan_networks_button = obj;
                    lv_obj_set_pos(obj, 5, 5);
                    lv_obj_set_size(obj, 193, 36);
                    lv_obj_add_event_cb(obj, action_scan_networks, LV_EVENT_CLICKED, (void *)0);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "Scan Networks");
                        }
                    }
                }
                {
                    // EnableWifiButton
                    lv_obj_t *obj = lv_btn_create(parent_obj);
                    objects.enable_wifi_button = obj;
                    lv_obj_set_pos(obj, 202, 5);
                    lv_obj_set_size(obj, 34, 36);
                    lv_obj_add_event_cb(obj, action_toggle_wifi, LV_EVENT_CLICKED, (void *)0);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x747474), LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    // WIFISSIDTextArea
                    lv_obj_t *obj = lv_textarea_create(parent_obj);
                    objects.wifissid_text_area = obj;
                    lv_obj_set_pos(obj, 5, 46);
                    lv_obj_set_size(obj, 116, 36);
                    lv_textarea_set_accepted_chars(obj, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()_+-=[]{}|;:'\",.<>/?~\\ ");
                    lv_textarea_set_max_length(obj, 32);
                    lv_textarea_set_placeholder_text(obj, "SSID");
                    lv_textarea_set_one_line(obj, true);
                    lv_textarea_set_password_mode(obj, false);
                    lv_obj_add_event_cb(obj, action_on_text_area_focused, LV_EVENT_CLICKED, (void *)0);
                    lv_obj_add_event_cb(obj, action_hide_keyboard, LV_EVENT_DEFOCUSED, (void *)0);
                    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
                }
                {
                    // WIFIPasswordTextArea
                    lv_obj_t *obj = lv_textarea_create(parent_obj);
                    objects.wifi_password_text_area = obj;
                    lv_obj_set_pos(obj, 5, 85);
                    lv_obj_set_size(obj, 231, 36);
                    lv_textarea_set_accepted_chars(obj, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()_+-=[]{}|;:'\",.<>/?~\\ ");
                    lv_textarea_set_max_length(obj, 63);
                    lv_textarea_set_placeholder_text(obj, "Password");
                    lv_textarea_set_one_line(obj, true);
                    lv_textarea_set_password_mode(obj, false);
                    lv_obj_add_event_cb(obj, action_on_text_area_focused, LV_EVENT_CLICKED, (void *)1);
                    lv_obj_add_event_cb(obj, action_hide_keyboard, LV_EVENT_DEFOCUSED, (void *)0);
                    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
                }
                {
                    // WIFIProtocolDropdown
                    lv_obj_t *obj = lv_dropdown_create(parent_obj);
                    objects.wifi_protocol_dropdown = obj;
                    lv_obj_set_pos(obj, 125, 46);
                    lv_obj_set_size(obj, 110, LV_SIZE_CONTENT);
                    lv_dropdown_set_options_static(obj, "Open\nWPA/WPA2\nWPA3-Personal\nWPA-Enterprise\nWEP");
                    lv_dropdown_set_selected(obj, 0);
                }
                {
                    // DiscoveredSSIDS
                    lv_obj_t *obj = lv_roller_create(parent_obj);
                    objects.discovered_ssids = obj;
                    lv_obj_set_pos(obj, 5, 124);
                    lv_obj_set_size(obj, 231, 112);
                    lv_roller_set_options(obj, "", LV_ROLLER_MODE_NORMAL);
                    lv_obj_add_event_cb(obj, action_select_ssid, LV_EVENT_VALUE_CHANGED, (void *)0);
                    lv_obj_add_event_cb(obj, event_handler_cb_wifi_menu_discovered_ssids, LV_EVENT_ALL, 0);
                }
            }
        }
        {
            // PacketLoggingContainer
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.packet_logging_container = obj;
            lv_obj_set_pos(obj, 0, 78);
            lv_obj_set_size(obj, 240, 242);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // TogglePacketLoggingSwitch
                    lv_obj_t *obj = lv_switch_create(parent_obj);
                    objects.toggle_packet_logging_switch = obj;
                    lv_obj_set_pos(obj, 176, 9);
                    lv_obj_set_size(obj, 50, 25);
                    lv_obj_add_event_cb(obj, action_toggle_packet_logging, LV_EVENT_VALUE_CHANGED, (void *)0);
                }
                {
                    // PacketLoggingSSID
                    lv_obj_t *obj = lv_textarea_create(parent_obj);
                    objects.packet_logging_ssid = obj;
                    lv_obj_set_pos(obj, 5, 4);
                    lv_obj_set_size(obj, 156, 36);
                    lv_textarea_set_accepted_chars(obj, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()_+-=[]{}|;:'\",.<>/?~\\ ");
                    lv_textarea_set_max_length(obj, 32);
                    lv_textarea_set_placeholder_text(obj, "SSID");
                    lv_textarea_set_one_line(obj, true);
                    lv_textarea_set_password_mode(obj, false);
                    lv_obj_add_event_cb(obj, action_on_text_area_focused, LV_EVENT_CLICKED, (void *)0);
                    lv_obj_add_event_cb(obj, action_hide_keyboard, LV_EVENT_DEFOCUSED, (void *)0);
                    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
                }
                {
                    // PacketLoggingChart
                    lv_obj_t *obj = lv_chart_create(parent_obj);
                    objects.packet_logging_chart = obj;
                    lv_obj_set_pos(obj, 35, 44);
                    lv_obj_set_size(obj, 191, 161);
                }
            }
        }
        {
            // DeviceInventoryContainer
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.device_inventory_container = obj;
            lv_obj_set_pos(obj, 0, 78);
            lv_obj_set_size(obj, 240, 242);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // StartDeviceInventorySwitch
                    lv_obj_t *obj = lv_switch_create(parent_obj);
                    objects.start_device_inventory_switch = obj;
                    lv_obj_set_pos(obj, 186, 9);
                    lv_obj_set_size(obj, 50, 25);
                    lv_obj_add_event_cb(obj, action_toggle_device_inventory, LV_EVENT_VALUE_CHANGED, (void *)0);
                }
                {
                    // DeviceInventoryToggleManufacturer
                    lv_obj_t *obj = lv_switch_create(parent_obj);
                    objects.device_inventory_toggle_manufacturer = obj;
                    lv_obj_set_pos(obj, 5, 9);
                    lv_obj_set_size(obj, 50, 25);
                    lv_obj_add_event_cb(obj, action_toggle_device_inventory_manufacturers, LV_EVENT_VALUE_CHANGED, (void *)0);
                }
                {
                    // SendMACToDeauthButton
                    lv_obj_t *obj = lv_btn_create(parent_obj);
                    objects.send_mac_to_deauth_button = obj;
                    lv_obj_set_pos(obj, 70, 9);
                    lv_obj_set_size(obj, 100, 25);
                    lv_obj_add_event_cb(obj, action_send_device_to_deauth, LV_EVENT_CLICKED, (void *)0);
                }
                {
                    // InventoriedDevicesRoller
                    lv_obj_t *obj = lv_roller_create(parent_obj);
                    objects.inventoried_devices_roller = obj;
                    lv_obj_set_pos(obj, 5, 43);
                    lv_obj_set_size(obj, 231, 155);
                    lv_roller_set_options(obj, "", LV_ROLLER_MODE_NORMAL);
                }
                {
                    // RSSIBar
                    lv_obj_t *obj = lv_bar_create(parent_obj);
                    objects.rssi_bar = obj;
                    lv_obj_set_pos(obj, 5, 206);
                    lv_obj_set_size(obj, 231, 27);
                }
            }
        }
        {
            // MacSpooferContainer
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.mac_spoofer_container = obj;
            lv_obj_set_pos(obj, 0, 78);
            lv_obj_set_size(obj, 240, 242);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // MACToSpoofTextarea
                    lv_obj_t *obj = lv_textarea_create(parent_obj);
                    objects.mac_to_spoof_textarea = obj;
                    lv_obj_set_pos(obj, 5, 9);
                    lv_obj_set_size(obj, 144, 36);
                    lv_textarea_set_accepted_chars(obj, "1234567890ABCDEFabcdef:");
                    lv_textarea_set_max_length(obj, 17);
                    lv_textarea_set_placeholder_text(obj, "XX:XX:XX:XX:XX:XX");
                    lv_textarea_set_one_line(obj, true);
                    lv_textarea_set_password_mode(obj, false);
                    lv_obj_add_event_cb(obj, action_enforce_mac_format, LV_EVENT_VALUE_CHANGED, (void *)0);
                    lv_obj_add_event_cb(obj, action_on_text_area_focused, LV_EVENT_CLICKED, (void *)0);
                    lv_obj_add_event_cb(obj, action_hide_keyboard, LV_EVENT_DEFOCUSED, (void *)0);
                    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
                }
                {
                    // ToggleMACSpoofingSwitch
                    lv_obj_t *obj = lv_switch_create(parent_obj);
                    objects.toggle_mac_spoofing_switch = obj;
                    lv_obj_set_pos(obj, 186, 15);
                    lv_obj_set_size(obj, 50, 25);
                    lv_obj_add_event_cb(obj, action_toggle_mac_spoofing, LV_EVENT_VALUE_CHANGED, (void *)0);
                }
                {
                    // CurrentMACLabel
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.current_mac_label = obj;
                    lv_obj_set_pos(obj, 6, 58);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text_static(obj, "XX:XX:XX:XX:XX:XX");
                }
                {
                    // RandomizeSpoofMACButton
                    lv_obj_t *obj = lv_btn_create(parent_obj);
                    objects.randomize_spoof_mac_button = obj;
                    lv_obj_set_pos(obj, 134, 51);
                    lv_obj_set_size(obj, 102, 31);
                    lv_obj_add_event_cb(obj, action_randomize_mac, LV_EVENT_CLICKED, (void *)0);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "Random MAC");
                        }
                    }
                }
                {
                    // ToggleMACSToSpoofSwitch
                    lv_obj_t *obj = lv_switch_create(parent_obj);
                    objects.toggle_macs_to_spoof_switch = obj;
                    lv_obj_set_pos(obj, 186, 94);
                    lv_obj_set_size(obj, 50, 25);
                    lv_obj_add_event_cb(obj, action_toggle_macs_to_spoof, LV_EVENT_VALUE_CHANGED, (void *)0);
                }
                {
                    // PickMACToSpoofRoller
                    lv_obj_t *obj = lv_roller_create(parent_obj);
                    objects.pick_mac_to_spoof_roller = obj;
                    lv_obj_set_pos(obj, 5, 131);
                    lv_obj_set_size(obj, 231, 105);
                    lv_roller_set_options(obj, "", LV_ROLLER_MODE_NORMAL);
                    lv_obj_add_event_cb(obj, action_select_mac_to_spoof, LV_EVENT_VALUE_CHANGED, (void *)0);
                }
                {
                    // ToggleMACRotationSwitch
                    lv_obj_t *obj = lv_switch_create(parent_obj);
                    objects.toggle_mac_rotation_switch = obj;
                    lv_obj_set_pos(obj, 70, 94);
                    lv_obj_set_size(obj, 50, 25);
                    lv_obj_add_event_cb(obj, action_toggle_mac_rotation, LV_EVENT_VALUE_CHANGED, (void *)0);
                }
                {
                    // MACRotationTimeTextarea
                    lv_obj_t *obj = lv_textarea_create(parent_obj);
                    objects.mac_rotation_time_textarea = obj;
                    lv_obj_set_pos(obj, 6, 88);
                    lv_obj_set_size(obj, 56, 36);
                    lv_textarea_set_accepted_chars(obj, "1234567890");
                    lv_textarea_set_max_length(obj, 4);
                    lv_textarea_set_text(obj, "300");
                    lv_textarea_set_placeholder_text(obj, "9999");
                    lv_textarea_set_one_line(obj, true);
                    lv_textarea_set_password_mode(obj, false);
                    lv_obj_add_event_cb(obj, action_on_text_area_focused, LV_EVENT_CLICKED, (void *)0);
                }
            }
        }
        {
            // DeauthenticatorContainer
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.deauthenticator_container = obj;
            lv_obj_set_pos(obj, 0, 78);
            lv_obj_set_size(obj, 240, 242);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // DeauthenticateMACTextarea
                    lv_obj_t *obj = lv_textarea_create(parent_obj);
                    objects.deauthenticate_mac_textarea = obj;
                    lv_obj_set_pos(obj, 5, 9);
                    lv_obj_set_size(obj, 143, 36);
                    lv_textarea_set_accepted_chars(obj, "1234567890ABCDEFabcdef:");
                    lv_textarea_set_max_length(obj, 128);
                    lv_textarea_set_placeholder_text(obj, "XX:XX:XX:XX:XX:XX");
                    lv_textarea_set_one_line(obj, true);
                    lv_textarea_set_password_mode(obj, false);
                    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE);
                }
                {
                    // DeauthAllCheckbox
                    lv_obj_t *obj = lv_checkbox_create(parent_obj);
                    objects.deauth_all_checkbox = obj;
                    lv_obj_set_pos(obj, 154, 17);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_checkbox_set_text_static(obj, "Kick All");
                }
                {
                    // DeauthFrequencyArc
                    lv_obj_t *obj = lv_arc_create(parent_obj);
                    objects.deauth_frequency_arc = obj;
                    lv_obj_set_pos(obj, 20, 53);
                    lv_obj_set_size(obj, 209, 202);
                    lv_arc_set_value(obj, 25);
                }
                {
                    // DeauthenticateButton
                    lv_obj_t *obj = lv_btn_create(parent_obj);
                    objects.deauthenticate_button = obj;
                    lv_obj_set_pos(obj, 51, 83);
                    lv_obj_set_size(obj, 140, 140);
                    lv_obj_add_event_cb(obj, action_toggle_deauthenticator_button, LV_EVENT_PRESSED, (void *)0);
                    lv_obj_set_style_radius(obj, 100, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_clip_corner(obj, false, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff0000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // DeauthenticateButtonLabel
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.deauthenticate_button_label = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "DEAUTHENTICATE");
                        }
                    }
                }
            }
        }
        {
            // KeyboardWifi
            lv_obj_t *obj = lv_keyboard_create(parent_obj);
            objects.keyboard_wifi = obj;
            lv_obj_set_pos(obj, 0, 199);
            lv_obj_set_size(obj, 240, 120);
            lv_obj_add_event_cb(obj, action_hide_keyboard, LV_EVENT_READY, (void *)0);
            lv_obj_add_event_cb(obj, action_hide_keyboard, LV_EVENT_CANCEL, (void *)0);
            lv_obj_set_style_align(obj, LV_ALIGN_DEFAULT, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    
    tick_screen_wifi_menu();
}

void tick_screen_wifi_menu() {
    tick_user_widget_status_bar(21);
    {
        if (!(lv_obj_get_state(objects.wifi_option_dropdown) & LV_STATE_EDITED)) {
            int32_t new_val = get_var_wifi_option();
            int32_t cur_val = lv_dropdown_get_selected(objects.wifi_option_dropdown);
            if (new_val != cur_val) {
                tick_value_change_obj = objects.wifi_option_dropdown;
                lv_dropdown_set_selected(objects.wifi_option_dropdown, new_val);
                tick_value_change_obj = NULL;
            }
        }
    }
    {
        if (!(lv_obj_get_state(objects.discovered_ssids) & LV_STATE_EDITED)) {
            int32_t new_val = get_var_network_found_ssid();
            int32_t cur_val = lv_roller_get_selected(objects.discovered_ssids);
            if (new_val != cur_val) {
                tick_value_change_obj = objects.discovered_ssids;
                lv_roller_set_selected(objects.discovered_ssids, new_val, LV_ANIM_OFF);
                tick_value_change_obj = NULL;
            }
        }
    }
    {
        int32_t new_val = get_var_device_inventory_signal_strength();
        int32_t cur_val = lv_bar_get_value(objects.rssi_bar);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.rssi_bar;
            lv_bar_set_value(objects.rssi_bar, new_val, LV_ANIM_ON);
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = get_var_show_keyboard();
        bool cur_val = lv_obj_has_flag(objects.keyboard_wifi, LV_OBJ_FLAG_HIDDEN);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.keyboard_wifi;
            if (new_val) {
                lv_obj_add_flag(objects.keyboard_wifi, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_clear_flag(objects.keyboard_wifi, LV_OBJ_FLAG_HIDDEN);
            }
            tick_value_change_obj = NULL;
        }
    }
}

void create_screen_bluetooth_menu() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.bluetooth_menu = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 240, 320);
    {
        lv_obj_t *parent_obj = obj;
        {
            // BluetoothMenuStatusBar
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.bluetooth_menu_status_bar = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 240, 320);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            create_user_widget_status_bar(obj, 62);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 51, 21);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_decor(obj, LV_TEXT_DECOR_UNDERLINE, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "BLUETOOTH MENU");
        }
        {
            // BackButtonMain_1
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.back_button_main_1 = obj;
            lv_obj_set_pos(obj, 4, 20);
            lv_obj_set_size(obj, 31, 18);
            lv_obj_add_event_cb(obj, action_change_screen, LV_EVENT_CLICKED, (void *)2);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x545454), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // BluetoothOptionDropdown
            lv_obj_t *obj = lv_dropdown_create(parent_obj);
            objects.bluetooth_option_dropdown = obj;
            lv_obj_set_pos(obj, 5, 42);
            lv_obj_set_size(obj, 231, LV_SIZE_CONTENT);
            lv_dropdown_set_options_static(obj, "BLE Scanner\nBLE Spoofer\nBLE Wifi Bridging\nBLE Jammer");
            lv_obj_add_event_cb(obj, action_change_bluetooth_option, LV_EVENT_VALUE_CHANGED, (void *)0);
            lv_obj_add_event_cb(obj, event_handler_cb_bluetooth_menu_bluetooth_option_dropdown, LV_EVENT_ALL, 0);
        }
        {
            // BluetoothScannerContainer
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.bluetooth_scanner_container = obj;
            lv_obj_set_pos(obj, 0, 78);
            lv_obj_set_size(obj, 240, 242);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // ScanBluetoothButton
                    lv_obj_t *obj = lv_btn_create(parent_obj);
                    objects.scan_bluetooth_button = obj;
                    lv_obj_set_pos(obj, 5, 5);
                    lv_obj_set_size(obj, 193, 36);
                    lv_obj_add_event_cb(obj, action_scan_bluetooth, LV_EVENT_CLICKED, (void *)0);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "Scan Bluetooth");
                        }
                    }
                }
                {
                    // EnableBluetoothButton
                    lv_obj_t *obj = lv_btn_create(parent_obj);
                    objects.enable_bluetooth_button = obj;
                    lv_obj_set_pos(obj, 202, 5);
                    lv_obj_set_size(obj, 34, 36);
                    lv_obj_add_event_cb(obj, action_toggle_bluetooth, LV_EVENT_CLICKED, (void *)0);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x747474), LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    // ScanBluetoothFilterSlider
                    lv_obj_t *obj = lv_slider_create(parent_obj);
                    objects.scan_bluetooth_filter_slider = obj;
                    lv_obj_set_pos(obj, 8, 55);
                    lv_obj_set_size(obj, 151, 17);
                    lv_slider_set_range(obj, -100, -20);
                    lv_slider_set_value(obj, -70, LV_ANIM_OFF);
                    lv_obj_add_event_cb(obj, action_adjust_bluetooth_scan_range, LV_EVENT_VALUE_CHANGED, (void *)0);
                }
                {
                    // BLEScanRangeLabel
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.ble_scan_range_label = obj;
                    lv_obj_set_pos(obj, 173, 56);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text_static(obj, "-70dBm");
                }
                {
                    // DiscoveredBLEDevices
                    lv_obj_t *obj = lv_roller_create(parent_obj);
                    objects.discovered_ble_devices = obj;
                    lv_obj_set_pos(obj, 4, 86);
                    lv_obj_set_size(obj, 116, 112);
                    lv_roller_set_options(obj, "", LV_ROLLER_MODE_NORMAL);
                    lv_obj_add_event_cb(obj, action_select_bluetooth_device, LV_EVENT_VALUE_CHANGED, (void *)0);
                    lv_obj_add_event_cb(obj, event_handler_cb_bluetooth_menu_discovered_ble_devices, LV_EVENT_ALL, 0);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_10, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    // DiscoveredBLEDevicesInfo
                    lv_obj_t *obj = lv_roller_create(parent_obj);
                    objects.discovered_ble_devices_info = obj;
                    lv_obj_set_pos(obj, 121, 86);
                    lv_obj_set_size(obj, 116, 112);
                    lv_roller_set_options(obj, "", LV_ROLLER_MODE_NORMAL);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_10, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    // BluetoothScannerRSSI
                    lv_obj_t *obj = lv_bar_create(parent_obj);
                    objects.bluetooth_scanner_rssi = obj;
                    lv_obj_set_pos(obj, 5, 206);
                    lv_obj_set_size(obj, 231, 27);
                }
                {
                    // PairingMSGContainer
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    objects.pairing_msg_container = obj;
                    lv_obj_set_pos(obj, 30, 0);
                    lv_obj_set_size(obj, 181, 127);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
                    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_color(obj, lv_color_hex(0xb1b1b1), LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // ClosePairingContainer
                            lv_obj_t *obj = lv_btn_create(parent_obj);
                            objects.close_pairing_container = obj;
                            lv_obj_set_pos(obj, 139, 8);
                            lv_obj_set_size(obj, 32, 32);
                            lv_obj_add_event_cb(obj, action_close_pairing_msg_box, LV_EVENT_CLICKED, (void *)0);
                        }
                        {
                            // AcceptPairingButton
                            lv_obj_t *obj = lv_btn_create(parent_obj);
                            objects.accept_pairing_button = obj;
                            lv_obj_set_pos(obj, 8, 8);
                            lv_obj_set_size(obj, 32, 32);
                            lv_obj_add_event_cb(obj, action_accept_pairing, LV_EVENT_CLICKED, (void *)0);
                            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
                        }
                        {
                            // PairingMSGPin
                            lv_obj_t *obj = lv_textarea_create(parent_obj);
                            objects.pairing_msg_pin = obj;
                            lv_obj_set_pos(obj, 16, 55);
                            lv_obj_set_size(obj, 147, 55);
                            lv_textarea_set_accepted_chars(obj, "1234567890");
                            lv_textarea_set_max_length(obj, 7);
                            lv_textarea_set_placeholder_text(obj, "000000");
                            lv_textarea_set_one_line(obj, true);
                            lv_textarea_set_password_mode(obj, false);
                            lv_obj_add_event_cb(obj, action_on_text_area_focused, LV_EVENT_CLICKED, (void *)1);
                            lv_obj_add_event_cb(obj, action_hide_keyboard, LV_EVENT_DEFOCUSED, (void *)0);
                            lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
                            lv_obj_set_style_text_font(obj, &lv_font_montserrat_32, LV_PART_MAIN | LV_STATE_DEFAULT);
                        }
                    }
                }
            }
        }
        {
            // BluetoothSpooferContainer
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.bluetooth_spoofer_container = obj;
            lv_obj_set_pos(obj, 0, 78);
            lv_obj_set_size(obj, 240, 242);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // BTSpoofMACTextarea
                    lv_obj_t *obj = lv_textarea_create(parent_obj);
                    objects.bt_spoof_mac_textarea = obj;
                    lv_obj_set_pos(obj, 5, 8);
                    lv_obj_set_size(obj, 144, 36);
                    lv_textarea_set_accepted_chars(obj, "1234567890ABCDEFabcdef:");
                    lv_textarea_set_max_length(obj, 17);
                    lv_textarea_set_placeholder_text(obj, "XX:XX:XX:XX:XX:XX");
                    lv_textarea_set_one_line(obj, true);
                    lv_textarea_set_password_mode(obj, false);
                    lv_obj_add_event_cb(obj, action_enforce_btmac_format, LV_EVENT_VALUE_CHANGED, (void *)0);
                    lv_obj_add_event_cb(obj, action_on_text_area_focused, LV_EVENT_CLICKED, (void *)0);
                    lv_obj_add_event_cb(obj, action_hide_keyboard, LV_EVENT_DEFOCUSED, (void *)0);
                    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
                }
                {
                    // ToggleBluetoothMACSpoofingSwitch
                    lv_obj_t *obj = lv_switch_create(parent_obj);
                    objects.toggle_bluetooth_mac_spoofing_switch = obj;
                    lv_obj_set_pos(obj, 186, 14);
                    lv_obj_set_size(obj, 50, 25);
                    lv_obj_add_event_cb(obj, action_toggle_bluetooth_mac_spoofing, LV_EVENT_VALUE_CHANGED, (void *)0);
                }
                {
                    // BTCurrentMACLabel
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.bt_current_mac_label = obj;
                    lv_obj_set_pos(obj, 6, 58);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text_static(obj, "XX:XX:XX:XX:XX:XX");
                }
                {
                    // RandomizeBTSpoofMACButton
                    lv_obj_t *obj = lv_btn_create(parent_obj);
                    objects.randomize_bt_spoof_mac_button = obj;
                    lv_obj_set_pos(obj, 134, 51);
                    lv_obj_set_size(obj, 102, 31);
                    lv_obj_add_event_cb(obj, action_randomize_bluetooth_mac, LV_EVENT_CLICKED, (void *)0);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "Random MAC");
                        }
                    }
                }
                {
                    // PickBluetoothMACToSpoofRoller
                    lv_obj_t *obj = lv_roller_create(parent_obj);
                    objects.pick_bluetooth_mac_to_spoof_roller = obj;
                    lv_obj_set_pos(obj, 5, 90);
                    lv_obj_set_size(obj, 231, 146);
                    lv_roller_set_options(obj, "", LV_ROLLER_MODE_NORMAL);
                    lv_obj_add_event_cb(obj, action_select_bt_preset_mac, LV_EVENT_VALUE_CHANGED, (void *)0);
                }
            }
        }
        {
            // BluetoothWifiBridgingContainer
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.bluetooth_wifi_bridging_container = obj;
            lv_obj_set_pos(obj, 0, 78);
            lv_obj_set_size(obj, 240, 242);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
        }
        {
            // BluetoothJammerContainer
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.bluetooth_jammer_container = obj;
            lv_obj_set_pos(obj, 0, 78);
            lv_obj_set_size(obj, 240, 242);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
        }
        {
            // KeyboardBluetooth
            lv_obj_t *obj = lv_keyboard_create(parent_obj);
            objects.keyboard_bluetooth = obj;
            lv_obj_set_pos(obj, 0, 199);
            lv_obj_set_size(obj, 240, 120);
            lv_obj_add_event_cb(obj, action_hide_keyboard, LV_EVENT_READY, (void *)0);
            lv_obj_add_event_cb(obj, action_hide_keyboard, LV_EVENT_CANCEL, (void *)0);
            lv_obj_set_style_align(obj, LV_ALIGN_DEFAULT, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    
    tick_screen_bluetooth_menu();
}

void tick_screen_bluetooth_menu() {
    tick_user_widget_status_bar(62);
    {
        if (!(lv_obj_get_state(objects.bluetooth_option_dropdown) & LV_STATE_EDITED)) {
            int32_t new_val = get_var_bluetooth_option();
            int32_t cur_val = lv_dropdown_get_selected(objects.bluetooth_option_dropdown);
            if (new_val != cur_val) {
                tick_value_change_obj = objects.bluetooth_option_dropdown;
                lv_dropdown_set_selected(objects.bluetooth_option_dropdown, new_val);
                tick_value_change_obj = NULL;
            }
        }
    }
    {
        if (!(lv_obj_get_state(objects.discovered_ble_devices) & LV_STATE_EDITED)) {
            int32_t new_val = get_var_network_found_ssid();
            int32_t cur_val = lv_roller_get_selected(objects.discovered_ble_devices);
            if (new_val != cur_val) {
                tick_value_change_obj = objects.discovered_ble_devices;
                lv_roller_set_selected(objects.discovered_ble_devices, new_val, LV_ANIM_OFF);
                tick_value_change_obj = NULL;
            }
        }
    }
    {
        int32_t new_val = get_var_device_inventory_signal_strength();
        int32_t cur_val = lv_bar_get_value(objects.bluetooth_scanner_rssi);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.bluetooth_scanner_rssi;
            lv_bar_set_value(objects.bluetooth_scanner_rssi, new_val, LV_ANIM_ON);
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = get_var_show_keyboard();
        bool cur_val = lv_obj_has_flag(objects.keyboard_bluetooth, LV_OBJ_FLAG_HIDDEN);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.keyboard_bluetooth;
            if (new_val) {
                lv_obj_add_flag(objects.keyboard_bluetooth, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_clear_flag(objects.keyboard_bluetooth, LV_OBJ_FLAG_HIDDEN);
            }
            tick_value_change_obj = NULL;
        }
    }
}

void create_screen_nfc_menu() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.nfc_menu = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 240, 320);
    {
        lv_obj_t *parent_obj = obj;
        {
            // NFCMenuStatusBar
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.nfc_menu_status_bar = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 240, 320);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            create_user_widget_status_bar(obj, 91);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 81, 21);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_decor(obj, LV_TEXT_DECOR_UNDERLINE, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "NFC MENU");
        }
        {
            // BackButtonMain_2
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.back_button_main_2 = obj;
            lv_obj_set_pos(obj, 4, 20);
            lv_obj_set_size(obj, 31, 18);
            lv_obj_add_event_cb(obj, action_change_screen, LV_EVENT_CLICKED, (void *)2);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x545454), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    
    tick_screen_nfc_menu();
}

void tick_screen_nfc_menu() {
    tick_user_widget_status_bar(91);
}

void create_screen_rfid_menu() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.rfid_menu = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 240, 320);
    {
        lv_obj_t *parent_obj = obj;
        {
            // RFIDMenuStatusBar
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.rfid_menu_status_bar = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 240, 320);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            create_user_widget_status_bar(obj, 98);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 79, 21);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_decor(obj, LV_TEXT_DECOR_UNDERLINE, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "RFID MENU");
        }
        {
            // BackButtonMain_3
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.back_button_main_3 = obj;
            lv_obj_set_pos(obj, 4, 20);
            lv_obj_set_size(obj, 31, 18);
            lv_obj_add_event_cb(obj, action_change_screen, LV_EVENT_CLICKED, (void *)2);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x545454), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    
    tick_screen_rfid_menu();
}

void tick_screen_rfid_menu() {
    tick_user_widget_status_bar(98);
}

void create_screen_ir_menu() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.ir_menu = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 240, 320);
    {
        lv_obj_t *parent_obj = obj;
        {
            // IRMenuStatusBar
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.ir_menu_status_bar = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 240, 320);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            create_user_widget_status_bar(obj, 105);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 89, 21);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_decor(obj, LV_TEXT_DECOR_UNDERLINE, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "IR MENU");
        }
        {
            // BackButtonMain_4
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.back_button_main_4 = obj;
            lv_obj_set_pos(obj, 4, 20);
            lv_obj_set_size(obj, 31, 18);
            lv_obj_add_event_cb(obj, action_change_screen, LV_EVENT_CLICKED, (void *)2);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x545454), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    
    tick_screen_ir_menu();
}

void tick_screen_ir_menu() {
    tick_user_widget_status_bar(105);
}

void create_screen_rf_menu() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.rf_menu = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 240, 320);
    {
        lv_obj_t *parent_obj = obj;
        {
            // RFMenuStatusBar
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.rf_menu_status_bar = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 240, 320);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            create_user_widget_status_bar(obj, 112);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 87, 21);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_decor(obj, LV_TEXT_DECOR_UNDERLINE, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "RF MENU");
        }
        {
            // BackButtonMain_5
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.back_button_main_5 = obj;
            lv_obj_set_pos(obj, 4, 20);
            lv_obj_set_size(obj, 31, 18);
            lv_obj_add_event_cb(obj, action_change_screen, LV_EVENT_CLICKED, (void *)2);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x545454), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    
    tick_screen_rf_menu();
}

void tick_screen_rf_menu() {
    tick_user_widget_status_bar(112);
}

void create_user_widget_status_bar(lv_obj_t *parent_obj, int startWidgetIndex) {
    (void)startWidgetIndex;
    lv_obj_t *obj = parent_obj;
    {
        lv_obj_t *parent_obj = obj;
        {
            // StatusBarContainer
            lv_obj_t *obj = lv_obj_create(parent_obj);
            ((lv_obj_t **)&objects)[startWidgetIndex + 0] = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 240, 17);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x827d7d), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 100, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // WifiStatus
            lv_obj_t *obj = lv_img_create(parent_obj);
            ((lv_obj_t **)&objects)[startWidgetIndex + 1] = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 17, 17);
            lv_img_set_src(obj, &img_wifi);
            lv_img_set_zoom(obj, 15);
            lv_img_set_size_mode(obj, LV_IMG_SIZE_MODE_REAL);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        }
        {
            // BluetoothStatus
            lv_obj_t *obj = lv_img_create(parent_obj);
            ((lv_obj_t **)&objects)[startWidgetIndex + 2] = obj;
            lv_obj_set_pos(obj, 17, 0);
            lv_obj_set_size(obj, 18, 18);
            lv_img_set_src(obj, &img_bluetooth);
            lv_img_set_zoom(obj, 15);
            lv_img_set_size_mode(obj, LV_IMG_SIZE_MODE_REAL);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        }
        {
            // BatteryBar
            lv_obj_t *obj = lv_bar_create(parent_obj);
            ((lv_obj_t **)&objects)[startWidgetIndex + 3] = obj;
            lv_obj_set_pos(obj, 204, 4);
            lv_obj_set_size(obj, 28, 10);
            lv_bar_set_value(obj, 100, LV_ANIM_OFF);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        }
        {
            // BatteryPercentage
            lv_obj_t *obj = lv_label_create(parent_obj);
            ((lv_obj_t **)&objects)[startWidgetIndex + 4] = obj;
            lv_obj_set_pos(obj, 175, 3);
            lv_obj_set_size(obj, 29, 12);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_10, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "100%");
        }
    }
}

void tick_user_widget_status_bar(int startWidgetIndex) {
    (void)startWidgetIndex;
}

typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_logo,
    tick_screen_home_menu,
    tick_screen_wifi_menu,
    tick_screen_bluetooth_menu,
    tick_screen_nfc_menu,
    tick_screen_rfid_menu,
    tick_screen_ir_menu,
    tick_screen_rf_menu,
};
void tick_screen(int screen_index) {
    if (screen_index >= 0 && screen_index < 8) {
        tick_screen_funcs[screen_index]();
    }
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen(screenId - 1);
}

//
// Fonts
//

ext_font_desc_t fonts[] = {
#if LV_FONT_MONTSERRAT_8
    { "MONTSERRAT_8", &lv_font_montserrat_8 },
#endif
#if LV_FONT_MONTSERRAT_10
    { "MONTSERRAT_10", &lv_font_montserrat_10 },
#endif
#if LV_FONT_MONTSERRAT_12
    { "MONTSERRAT_12", &lv_font_montserrat_12 },
#endif
#if LV_FONT_MONTSERRAT_14
    { "MONTSERRAT_14", &lv_font_montserrat_14 },
#endif
#if LV_FONT_MONTSERRAT_16
    { "MONTSERRAT_16", &lv_font_montserrat_16 },
#endif
#if LV_FONT_MONTSERRAT_18
    { "MONTSERRAT_18", &lv_font_montserrat_18 },
#endif
#if LV_FONT_MONTSERRAT_20
    { "MONTSERRAT_20", &lv_font_montserrat_20 },
#endif
#if LV_FONT_MONTSERRAT_22
    { "MONTSERRAT_22", &lv_font_montserrat_22 },
#endif
#if LV_FONT_MONTSERRAT_24
    { "MONTSERRAT_24", &lv_font_montserrat_24 },
#endif
#if LV_FONT_MONTSERRAT_26
    { "MONTSERRAT_26", &lv_font_montserrat_26 },
#endif
#if LV_FONT_MONTSERRAT_28
    { "MONTSERRAT_28", &lv_font_montserrat_28 },
#endif
#if LV_FONT_MONTSERRAT_30
    { "MONTSERRAT_30", &lv_font_montserrat_30 },
#endif
#if LV_FONT_MONTSERRAT_32
    { "MONTSERRAT_32", &lv_font_montserrat_32 },
#endif
#if LV_FONT_MONTSERRAT_34
    { "MONTSERRAT_34", &lv_font_montserrat_34 },
#endif
#if LV_FONT_MONTSERRAT_36
    { "MONTSERRAT_36", &lv_font_montserrat_36 },
#endif
#if LV_FONT_MONTSERRAT_38
    { "MONTSERRAT_38", &lv_font_montserrat_38 },
#endif
#if LV_FONT_MONTSERRAT_40
    { "MONTSERRAT_40", &lv_font_montserrat_40 },
#endif
#if LV_FONT_MONTSERRAT_42
    { "MONTSERRAT_42", &lv_font_montserrat_42 },
#endif
#if LV_FONT_MONTSERRAT_44
    { "MONTSERRAT_44", &lv_font_montserrat_44 },
#endif
#if LV_FONT_MONTSERRAT_46
    { "MONTSERRAT_46", &lv_font_montserrat_46 },
#endif
#if LV_FONT_MONTSERRAT_48
    { "MONTSERRAT_48", &lv_font_montserrat_48 },
#endif
};

//
// Color themes
//

uint32_t active_theme_index = 0;

//
//
//

void create_screens() {

// Set default LVGL theme
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    
    // Initialize screens
    // Create screens
    create_screen_logo();
    create_screen_home_menu();
    create_screen_wifi_menu();
    create_screen_bluetooth_menu();
    create_screen_nfc_menu();
    create_screen_rfid_menu();
    create_screen_ir_menu();
    create_screen_rf_menu();
}