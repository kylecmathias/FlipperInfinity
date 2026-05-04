#ifndef UI_BRIDGE_H
#define UI_BRIDGE_H

#include "ui.h"

#ifdef __cplusplus
#include <Arduino.h> 
#include "wifi_menu.h"
#include <WiFi.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <esp_wifi.h>
#include <set>
#include <string>
#include <DNSServer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

extern SemaphoreHandle_t xGuiSemaphore;
extern bool isLoggingPackets;
extern bool isInventoryRunning;
extern bool isPortalRunning;
extern bool isSpamming;
extern bool isDeautherRunning;
extern DNSServer dnsServer;

void setupWifiMenu();
void checkWifiStatus();
void updatePacketChart();
void updatePortalUI();
void runBeaconSpam();
void runDeauthLoop();
void execute_deauth_sweep(uint8_t* victim, uint8_t* ap);
int craft_advanced_deauth(uint8_t* buf, uint8_t* client_mac, uint8_t* ap_mac, uint8_t type, uint8_t reason);
#endif 

#ifdef __cplusplus
extern "C" {
#endif 

void wifimenu_dropdown_handler(lv_event_t * e);
void WIFIConnectHandler(lv_event_t * e);
void ToggleFileDump(lv_event_t * e);
void TogglePacketLog(lv_event_t * e);
void ToggleDeviceInv(lv_event_t * e);
void TargetDeviceMAC(lv_event_t * e);
void ToggleCaptivePortal(lv_event_t * e);
void ToggleSSIDSpam(lv_event_t * e);
void RandomizeMAC(lv_event_t * e);
void StartMACSpoofing(lv_event_t * e);
void StartDeauth(lv_event_t * e);
void ScanNetworks(lv_event_t * e);
void RetrieveSSID(lv_event_t * e);

#ifdef __cplusplus
}
#endif 

#endif