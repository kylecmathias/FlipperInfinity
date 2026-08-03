#ifndef EEZ_LVGL_UI_EVENTS_H
#define EEZ_LVGL_UI_EVENTS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void action_change_screen(lv_event_t * e);
extern void action_change_screen_delay(lv_event_t * e);
extern void action_on_text_area_focused(lv_event_t * e);
extern void action_hide_keyboard(lv_event_t * e);
extern void action_scan_networks(lv_event_t * e);
extern void action_toggle_wifi(lv_event_t * e);
extern void action_change_wifi_option(lv_event_t * e);
extern void action_select_ssid(lv_event_t * e);
extern void action_toggle_packet_logging(lv_event_t * e);
extern void action_send_device_to_deauth(lv_event_t * e);
extern void action_toggle_device_inventory(lv_event_t * e);
extern void action_toggle_device_inventory_manufacturers(lv_event_t * e);
extern void action_toggle_mac_spoofing(lv_event_t * e);
extern void action_randomize_mac(lv_event_t * e);
extern void action_toggle_macs_to_spoof(lv_event_t * e);
extern void action_select_mac_to_spoof(lv_event_t * e);
extern void action_toggle_mac_rotation(lv_event_t * e);
extern void action_enforce_mac_format(lv_event_t * e);
extern void action_toggle_deauthenticator_button(lv_event_t * e);
extern void action_change_bluetooth_option(lv_event_t * e);
extern void action_select_bluetooth_device(lv_event_t * e);
extern void action_toggle_bluetooth(lv_event_t * e);
extern void action_scan_bluetooth(lv_event_t * e);
extern void action_adjust_bluetooth_scan_range(lv_event_t * e);
extern void action_close_pairing_msg_box(lv_event_t * e);
extern void action_accept_pairing(lv_event_t * e);
extern void action_enforce_btmac_format(lv_event_t * e);
extern void action_toggle_bluetooth_mac_spoofing(lv_event_t * e);
extern void action_randomize_bluetooth_mac(lv_event_t * e);
extern void action_select_bt_preset_mac(lv_event_t * e);
extern void action_toggle_reading_nfc(lv_event_t * e);
extern void action_nfc_read_save(lv_event_t * e);
extern void action_change_nfc_option(lv_event_t * e);
extern void action_check_nfc_read_filename(lv_event_t * e);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_EVENTS_H*/