#include "ui_bridge.hpp"

extern adc_oneshot_unit_handle_t adc1_handle;


//helpers
ScreensEnum screen_id(uint16_t screen) {
    return (screen >= ScreensEnum::_SCREEN_ID_FIRST && screen <= ScreensEnum::_SCREEN_ID_LAST) ? static_cast<ScreensEnum>(screen) : ScreensEnum::SCREEN_ID_HOME_MENU;
}
lv_obj_t* keyboard_id(uint8_t id) {
    lv_obj_t* keyboard;
    switch (id) {
        case 0:
            keyboard = objects.keyboard_wifi;
            break;
        default:
            keyboard = objects.keyboard_wifi;
    }

    return keyboard;
}
static void home_transition(lv_timer_t * timer) {
    loadScreen(ScreensEnum::SCREEN_ID_HOME_MENU);
}
void status_bar_update(bool wifi_active, bool bluetooth_active, bool on_battery, uint8_t battery_level) {
    lv_obj_t* wifi_icons[] = {
        objects.home_menu_status_bar__wifi_status,
        objects.wifi_menu_status_bar__wifi_status,
        objects.bluetooth_menu_status_bar__wifi_status,
        objects.nfc_menu_status_bar__wifi_status,
        objects.rfid_menu_status_bar__wifi_status,
        objects.ir_menu_status_bar__wifi_status,
        objects.rf_menu_status_bar__wifi_status,
    };
    lv_obj_t* bluetooth_icons[] = {
        objects.home_menu_status_bar__bluetooth_status,
        objects.wifi_menu_status_bar__bluetooth_status,
        objects.bluetooth_menu_status_bar__bluetooth_status,
        objects.nfc_menu_status_bar__bluetooth_status,
        objects.rfid_menu_status_bar__bluetooth_status,
        objects.ir_menu_status_bar__bluetooth_status,
        objects.rf_menu_status_bar__bluetooth_status,
    };
    lv_obj_t* batt_labels[] = {
        objects.home_menu_status_bar__battery_percentage,
        objects.wifi_menu_status_bar__battery_percentage,
        objects.bluetooth_menu_status_bar__battery_percentage,
        objects.nfc_menu_status_bar__battery_percentage,
        objects.rfid_menu_status_bar__battery_percentage,
        objects.ir_menu_status_bar__battery_percentage,
        objects.rf_menu_status_bar__battery_percentage,
    };
    lv_obj_t* batt_bars[] = {
        objects.home_menu_status_bar__battery_bar,
        objects.wifi_menu_status_bar__battery_bar,
        objects.bluetooth_menu_status_bar__battery_bar,
        objects.nfc_menu_status_bar__battery_bar,
        objects.rfid_menu_status_bar__battery_bar,
        objects.ir_menu_status_bar__battery_bar,
        objects.rf_menu_status_bar__battery_bar,
    };

    size_t num_status_bars = sizeof(wifi_icons) / sizeof(lv_obj_t*);

    uint8_t level;
    lv_color_t current_color;
    char batt[6];

    if (on_battery) {
        level = clamp(static_cast<uint8_t>(get_var_battery_level()), BATTERY_LEVEL_MIN, BATTERY_LEVEL_MAX);
        snprintf(batt, sizeof(batt), "%d%%", level);
        current_color = lv_palette_main(level > 50 ? LV_PALETTE_GREEN : level <= 50 && level > 20 ? LV_PALETTE_ORANGE : LV_PALETTE_RED);
    }
    else {
        level = 100;
        batt[0] = '\0'; 
        current_color = lv_palette_main(LV_PALETTE_BLUE);
    }

    const void* current_wifi_img = wifi_active ? static_cast<const void*>(&img_wifi) : static_cast<const void*>(&img_no_wifi);
    const void* current_bluetooth_img = bluetooth_active ? static_cast<const void*>(&img_bluetooth) : static_cast<const void*>(&img_no_bluetooth);

    for (size_t i = 0; i < num_status_bars; i++) {
        lv_img_set_src(wifi_icons[i], current_wifi_img);
        lv_img_set_src(bluetooth_icons[i], current_bluetooth_img);
        
        lv_label_set_text(batt_labels[i], batt);
        
        lv_bar_set_value(batt_bars[i], level, LV_ANIM_OFF);
        lv_obj_set_style_bg_color(batt_bars[i], current_color, LV_PART_INDICATOR);
    }
}
void status_timer_cb(lv_timer_t * timer) {
    bool wifi = false;
    bool bluetooth = false;
    bool is_on_battery = false;
    uint8_t battery_lvl = 100;
    int battery_adc;
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif != nullptr && esp_netif_is_netif_up(netif)) {
        esp_netif_ip_info_t ip_info;
        if (esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
            if (ip_info.ip.addr != 0) {
                wifi = true;
            }
        }
    }

    if (ble_hs_is_enabled() && ble_hs_synced()) {
    if (ble_gap_conn_active()) {
        bluetooth = true;
    }
}

    is_on_battery = gpio_get_level(GPIO_PINS::BATT_SRC) == 0 ? true : false;

    int adc_sum = 0;
    int single_read = 0;
    
    for (int i = 0; i < ADC_SAMPLES; i++) {
        adc_oneshot_read(adc1_handle, ADC_CHANNEL_3, &single_read);
        adc_sum += single_read;
        delay_us(50); 
    }
    battery_adc = adc_sum / ADC_SAMPLES;

    adc_oneshot_read(adc1_handle, ADC_CHANNEL_3, &battery_adc);
    battery_adc = clamp(battery_adc, BATT_ADC_MIN, BATT_ADC_MAX);

    battery_lvl = map(battery_adc, 0, (1UL << ADC_RES), 0, 100);

    set_var_wifi_active(wifi);
    set_var_bluetooth_active(bluetooth);
    set_var_on_battery(is_on_battery);
    set_var_battery_level(battery_lvl);

    status_bar_update(wifi, bluetooth, is_on_battery, battery_lvl);
}


//variables
int32_t get_var_page_id() { return page_id; }
void set_var_page_id(int32_t value) { page_id = value; }
int32_t get_var_battery_level() { return battery_level; }
void set_var_battery_level(int32_t value) { battery_level = value; }
bool get_var_on_battery() { return on_battery; }
void set_var_on_battery(bool value) { on_battery = value; }
bool get_var_wifi_active() { return wifi_active; }
void set_var_wifi_active(bool value) { wifi_active = value; }
bool get_var_bluetooth_active() { return bluetooth_active; }
void set_var_bluetooth_active(bool value) { bluetooth_active = value; }
bool get_var_show_keyboard() { return show_keyboard; }
void set_var_show_keyboard(bool value) { show_keyboard = value; }


//actions
void action_change_screen(lv_event_t * e) {
    uintptr_t screen_num = reinterpret_cast<uintptr_t>(lv_event_get_user_data(e));
    ScreensEnum screen = screen_id(static_cast<uint16_t>(screen_num));
    loadScreen(screen);
    set_screen_initial_content(screen);
    status_bar_update(wifi_active, bluetooth_active, on_battery, battery_level);
}
void action_change_screen_delay(lv_event_t * e) {
    lv_timer_t * transition_timer = lv_timer_create(home_transition, (2 * 1000), nullptr);
    lv_timer_set_repeat_count(transition_timer, 1);
}
void action_on_text_area_focused(lv_event_t * e) {
    set_var_show_keyboard(true);

    uintptr_t keyboard = reinterpret_cast<uintptr_t>(lv_event_get_user_data(e));
    lv_obj_t* keyboard_p = keyboard_id(static_cast<uint8_t>(keyboard));

    if (keyboard_p != nullptr) {
        lv_keyboard_set_textarea(keyboard_p, lv_event_get_target(e));
        lv_obj_clear_flag(keyboard_p, LV_OBJ_FLAG_HIDDEN);
    }
}
void action_hide_keyboard(lv_event_t * e) {
    set_var_show_keyboard(false);

    uintptr_t keyboard = reinterpret_cast<uintptr_t>(lv_event_get_user_data(e));
    lv_obj_t* keyboard_p = keyboard_id(static_cast<uint8_t>(keyboard));

    if (keyboard_p != nullptr) {
        lv_keyboard_set_textarea(keyboard_p, nullptr);
        lv_obj_add_flag(keyboard_p, LV_OBJ_FLAG_HIDDEN);
    }
}


//setups
void set_initial_flags() {
    set_var_wifi_active(false);
    set_var_bluetooth_active(false);
    set_var_show_keyboard(false);

    lv_timer_create(status_timer_cb, 1, nullptr);
}
void set_screen_initial_content(ScreensEnum screen) {
    status_bar_update(wifi_active, bluetooth_active, on_battery, battery_level);
    switch (screen) {
        case ScreensEnum::SCREEN_ID_HOME_MENU:

            break;
        case ScreensEnum::SCREEN_ID_WIFI_MENU:
            lv_obj_add_flag(objects.keyboard_wifi, LV_OBJ_FLAG_HIDDEN);
            break;
        case ScreensEnum::SCREEN_ID_BLUETOOTH_MENU:
            lv_obj_add_flag(objects.keyboard_bluetooth, LV_OBJ_FLAG_HIDDEN);
            break;
        default:
            break;
    }
}