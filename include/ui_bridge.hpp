#pragma once

#include "header.hpp"

#include "ui.h"
#include "screens.h"
#include "vars.h"
#include "actions.h"
#include "images.h"

inline int32_t page_id;
inline int32_t battery_level;
inline bool wifi_active;
inline bool bluetooth_active;
inline bool show_keyboard;
inline bool on_battery;

ScreensEnum screen_id(uint16_t screen);
lv_obj_t* keyboard_id(uint8_t id);
void status_bar_update(bool wifi_active, bool bluetooth_active, bool on_battery, uint8_t battery_level);
void set_screen_initial_content(ScreensEnum screen);
void set_initial_flags();