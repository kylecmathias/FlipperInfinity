#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <lvgl.h>
#include "ui.h" 
#include <XPT2046_Touchscreen.h>
#include <ESPAsyncWebServer.h>
#include "ui_bridge.h"

#define TOUCH_CS_PIN  D3  
#define TOUCH_IRQ_PIN D2  

#define TFT_DC    D9
#define TFT_CS    A2
#define TFT_RST   D8

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen ts(TOUCH_CS_PIN, TOUCH_IRQ_PIN);
AsyncWebServer server(80); 

//tasks
SemaphoreHandle_t xGuiSemaphore;
TaskHandle_t WiFiLogicTask;

void WiFiLogicLoop(void * pvParameters) {
    while (true) { 
        checkWifiStatus();     
        runBeaconSpam();      
        runDeauthLoop();      
        updatePacketChart();  
        
        if (isPortalRunning) {
            dnsServer.processNextRequest(); 
            updatePortalUI();
        }

        vTaskDelay(10 / portTICK_PERIOD_MS); 
    }
}


static const uint16_t screenWidth  = 240;
static const uint16_t screenHeight = 320;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 20]; 

void my_touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data) {
    if (ts.touched()) {
        TS_Point p = ts.getPoint();
        data->state = LV_INDEV_STATE_PR;
        
        data->point.x = map(p.x, 3800, 200, 0, screenWidth);
        data->point.y = map(p.y, 3800, 200, 0, screenHeight);
    } 
    else {
        data->state = LV_INDEV_STATE_REL;
    }
}

void my_disp_flush(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    
    tft.writePixels((uint16_t *)&color_p->full, w * h);
    
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

void setup() {
  Serial.begin(115200);

  if(!LittleFS.begin(true)){
      Serial.println("LittleFS Mount Failed");
      return;
  }

  xGuiSemaphore = xSemaphoreCreateMutex();
  
  tft.begin();
  tft.setRotation(0); 
  tft.fillScreen(ILI9341_WHITE);
  
  ts.begin();
  ts.setRotation(0); 

  lv_init();

  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 20);

  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  ui_init();

  setupWifiMenu(); 
  WiFi.mode(WIFI_STA);

  xTaskCreatePinnedToCore(
    WiFiLogicLoop,    
    "WiFiTask",     
    10000,            
    NULL,           
    1,                
    &WiFiLogicTask,  
    0);             

  Serial.println("System Initialized. UI on Core 1, Logic on Core 0.");
  Serial.println("Setup done");
}

void loop() {
  if (xSemaphoreTake(xGuiSemaphore, (TickType_t)10) == pdTRUE) {
        lv_timer_handler();
        xSemaphoreGive(xGuiSemaphore);
    }
    vTaskDelay(5);
}