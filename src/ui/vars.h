#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations

// Flow global variables

enum FlowGlobalVariables {
    FLOW_GLOBAL_VARIABLE_PAGE_ID = 0,
    FLOW_GLOBAL_VARIABLE_BATTERY_LEVEL = 1,
    FLOW_GLOBAL_VARIABLE_WIFI_ACTIVE = 2,
    FLOW_GLOBAL_VARIABLE_BLUETOOTH_ACTIVE = 3,
    FLOW_GLOBAL_VARIABLE_WIFI_OPTION = 4,
    FLOW_GLOBAL_VARIABLE_SHOW_KEYBOARD = 5,
    FLOW_GLOBAL_VARIABLE_NETWORK_FOUND_SSID = 6,
    FLOW_GLOBAL_VARIABLE_DEVICE_INVENTORY_SIGNAL_STRENGTH = 7,
    FLOW_GLOBAL_VARIABLE_ON_BATTERY = 8,
    FLOW_GLOBAL_VARIABLE_BLUETOOTH_OPTION = 9
};

// Native global variables

extern int32_t get_var_page_id();
extern void set_var_page_id(int32_t value);
extern int32_t get_var_battery_level();
extern void set_var_battery_level(int32_t value);
extern bool get_var_wifi_active();
extern void set_var_wifi_active(bool value);
extern bool get_var_bluetooth_active();
extern void set_var_bluetooth_active(bool value);
extern int32_t get_var_wifi_option();
extern void set_var_wifi_option(int32_t value);
extern bool get_var_show_keyboard();
extern void set_var_show_keyboard(bool value);
extern int32_t get_var_network_found_ssid();
extern void set_var_network_found_ssid(int32_t value);
extern int32_t get_var_device_inventory_signal_strength();
extern void set_var_device_inventory_signal_strength(int32_t value);
extern bool get_var_on_battery();
extern void set_var_on_battery(bool value);
extern int32_t get_var_bluetooth_option();
extern void set_var_bluetooth_option(int32_t value);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/