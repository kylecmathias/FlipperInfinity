#non ui files
set(srcs 
    "main.cpp"
    "ui_bridge.cpp"
    "main_menu.cpp"
    "bluetooth_menu.cpp"
    "wifi_menu.cpp"
)

file(GLOB ui_srcs "ui/*.c" "ui/*.cpp")
list(APPEND srcs ${ui_srcs})

idf_component_register(
    SRCS 
        ${srcs}
    INCLUDE_DIRS 
        "."            
        "../include"   
        "ui"       
    REQUIRES
        lvgl             
        esp_lvgl_port   
        esp_lcd_ili9341
        esp_event
        mdns
        bt
)

#target_link_libraries(${COMPONENT_LIB} "-Wl,--wrap=ieee80211_raw_frame_sanity_check")