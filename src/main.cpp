#include "ui_bridge.hpp"
#include "main_menu.hpp"
#include "bluetooth_menu.hpp"
#include "wifi_menu.hpp"

adc_oneshot_unit_handle_t adc1_handle = nullptr;

extern "C" {
    void app_main(void);
    //bool __wrap_ieee80211_raw_frame_sanity_check(void *frame, int length) { return true; }
    void* lv_psram_malloc(size_t size) { return heap_caps_malloc(size, MALLOC_CAP_SPIRAM); }
    void* lv_psram_realloc(void* ptr, size_t size) { return heap_caps_realloc(ptr, size, MALLOC_CAP_SPIRAM); }
}

void touch_calibration(esp_lcd_touch_handle_t tp, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num) {
    if (*point_num > 0) {
        float raw_x = *x;
        float raw_y = *y;

        float x_multiplier = LCD_X_MULTIPLIER;
        float y_multiplier = LCD_Y_MULTIPLIER; 

        int adjusted_x = (raw_x * x_multiplier) - LCD_X_OFFSET; 
        int adjusted_y = (raw_y * y_multiplier) - (raw_y < Y_DRIFT_END ? LCD_Y_OFFSET : 0); 


        if (adjusted_x < 0) adjusted_x = 0;
        if (adjusted_x > LCD_H_RES - 1) adjusted_x = LCD_H_RES - 1;
        if (adjusted_y < 0) adjusted_y = 0;
        if (adjusted_y > LCD_V_RES - 1) adjusted_y = LCD_V_RES - 1;

        *x = adjusted_x;
        *y = adjusted_y;
    }
}

void init_tft() {
    printf("Initializing TFT\n");

    //initialize spi bus
    spi_bus_config_t buscfg = {};
    buscfg.sclk_io_num = GPIO_PINS::SCK;
    buscfg.mosi_io_num = GPIO_PINS::MOSI;
    buscfg.miso_io_num = GPIO_PINS::MISO;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = LCD_H_RES * 80 * sizeof(uint16_t);

    spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO);

    //initialize tft screen
    esp_lcd_panel_io_handle_t io_handle = nullptr;
    esp_lcd_panel_io_spi_config_t io_config = {};
    io_config.dc_gpio_num = GPIO_PINS::TFT_DC;
    io_config.cs_gpio_num = GPIO_PINS::TFT_SS;
    io_config.pclk_hz = LCD_PIXEL_CLK;
    io_config.lcd_cmd_bits = 8;
    io_config.lcd_param_bits = 8;
    io_config.spi_mode = 0;
    io_config.trans_queue_depth = 10;

    esp_lcd_new_panel_io_spi(static_cast<esp_lcd_spi_bus_handle_t>(LCD_HOST), &io_config, &io_handle);

    //add the tfts chip
    esp_lcd_panel_handle_t panel_handle = nullptr;
    esp_lcd_panel_dev_config_t panel_config = {};
    panel_config.reset_gpio_num = GPIO_PINS::TFT_RST;
    panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR;
    panel_config.bits_per_pixel = 16;

    esp_lcd_new_panel_ili9341(io_handle, &panel_config, &panel_handle);
    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);
    esp_lcd_panel_disp_on_off(panel_handle, true);

    //initialize the tfts touch controller
    esp_lcd_panel_io_handle_t tp_io_handle = nullptr;
    esp_lcd_panel_io_spi_config_t tp_io_config = {};
    tp_io_config.cs_gpio_num = GPIO_PINS::TOUCH_SS;
    tp_io_config.pclk_hz = LCD_TOUCH_CLK;
    tp_io_config.spi_mode = 0;
    tp_io_config.lcd_cmd_bits = 8;
    tp_io_config.lcd_param_bits = 0;
    tp_io_config.trans_queue_depth = 3;

    esp_lcd_new_panel_io_spi(static_cast<esp_lcd_spi_bus_handle_t>(LCD_HOST), &tp_io_config, &tp_io_handle);

    esp_lcd_touch_handle_t tp = nullptr;
    esp_lcd_touch_config_t tp_cfg = {};
    tp_cfg.x_max = LCD_H_RES;
    tp_cfg.y_max = LCD_V_RES;
    tp_cfg.rst_gpio_num = GPIO_PINS::NONE;
    tp_cfg.int_gpio_num = GPIO_PINS::TOUCH_IRQ;
    tp_cfg.flags.swap_xy = 0;  
    tp_cfg.flags.mirror_x = 1; 
    tp_cfg.flags.mirror_y = 0;
    tp_cfg.process_coordinates = touch_calibration;

    esp_lcd_touch_new_spi_xpt2046(tp_io_handle, &tp_cfg, &tp);

    //initialize lvgl
    lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    lvgl_port_init(&lvgl_cfg);

    //pass the screen to lvgl
    lvgl_port_display_cfg_t disp_cfg = {};
    disp_cfg.io_handle = io_handle;
    disp_cfg.panel_handle = panel_handle;
    disp_cfg.buffer_size = LCD_H_RES * 40;
    disp_cfg.hres = LCD_H_RES;
    disp_cfg.vres = LCD_V_RES;
    disp_cfg.rotation.swap_xy = false;
    disp_cfg.rotation.mirror_x = false;
    disp_cfg.rotation.mirror_y = true;
    lv_disp_t *disp = lvgl_port_add_disp(&disp_cfg);

    //pass the touch panel
    lvgl_port_touch_cfg_t touch_cfg = {};
    touch_cfg.disp = disp;
    touch_cfg.handle = tp;
    lvgl_port_add_touch(&touch_cfg);

    printf("TFT initialized\n");
}

void init_wifi() {
    printf("Initializing Wifi\n");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    esp_netif_set_hostname(sta_netif, DISPLAY_HOSTNAME);

    wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_cfg));

    esp_wifi_set_mode(WIFI_MODE_STA);

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_SCAN_DONE, &wifi_scan_done_handler, nullptr, nullptr));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &wifi_connection_handler, nullptr, nullptr));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_connection_handler, nullptr, nullptr));

    esp_wifi_start();

    printf("Wifi initialized\n");
}

void init_bluetooth() {
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    ESP_ERROR_CHECK(nimble_port_init());

    int rc = ble_svc_gap_device_name_set("FlipperInfinity");
    assert(rc == 0);

    nimble_port_freertos_init(ble_host_task);
}

void init_pins() {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_PINS::BATT_SRC),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .clk_src  = static_cast<adc_oneshot_clk_src_t>(0),                      
        .ulp_mode = ADC_ULP_MODE_DISABLE
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_3, &config));
}

void app_main(void) {
    init_pins();
    init_wifi();
    init_bluetooth();
    init_psram_oui_database();
    vTaskDelay(pdMS_TO_TICKS(10));
    init_tft();

    lvgl_port_lock(0);
    ui_init();
    lvgl_port_unlock();

    vTaskDelay(pdMS_TO_TICKS(10));
    set_initial_flags();

    init_wifi_menu();
    init_bluetooth_menu();

    loadScreen(SCREEN_ID_LOGO);

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}