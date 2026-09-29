#ifndef _WEBSERVER_H
#define _WEBSERVER_H

//extern bool littlefs_ok;   // littlefs file found and servers started
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
//
#include "kostal_piko.h"

//
// define number of on/off switch times
const int max_items = 16;
//
//
// Values received from webpage
//
extern String web_ItemNu;
extern String web_OnOff;
extern String web_Sunday;
extern String web_Monday;
extern String web_Tuesday;
extern String web_Wednesday;
extern String web_Thursday;
extern String web_Friday;
extern String web_Saterday;
extern String web_Hours;
extern String web_Minutes;
//
// Holds message if webpage not correctly filled-in
//      
extern String ts_Message;
//
//
extern int int_ItemNu;    // pointer in the switchtimes array
//
// Struct with on/off days and times
//
struct switchtime {
  int int_OnOff;             // 0 = off; 1 = on
  int int_Sunday;            // 0 = not active; 1 = active
  int int_Monday;
  int int_Tuesday;
  int int_Wednesday;
  int int_Thursday;
  int int_Friday;
  int int_Saterday;
  int int_Hours;
  int int_Minutes;
};
//
// Array with on/off days and times
//
extern struct switchtime switchtimes[max_items];
//
// Values from wifimanager webpage
//
extern String web_wifi_ssid;
extern String web_wifi_password;
extern String web_wifi_ip;
extern String web_wifi_gateway;
extern String web_wifi_subnet;
extern String web_wifi_dns1;
extern String web_wifi_dns2;
extern String web_wifi_networkname;
//
// Values from mqttmanager webpage
//
extern String web_mqtt_host;
extern String web_mqtt_port;
extern String web_mqtt_user;
extern String web_mqtt_password;

extern String web_kostalpiko_host;
extern String web_kostalpiko_user;
extern String web_kostalpiko_password;

extern String web_ota_password;


//-----------------------------------------------
// Standbykiller    
//-----------------------------------------------
extern String str_sk_power;
extern String str_sk_hour;
extern String str_sk_minute;

extern String sk_Message;

extern int int_sk_Power;  
extern int int_sk_Hour;   
extern int int_sk_Minute; 
extern bool int_sk_Update;

extern bool restart;


//void view_webpage(void);
String processor_on_off(const String&);
String processor_value(const String&);
String processor_switch(const String&);
String processor_standbykiller(const String&);
String processor_memory(const String&);
String processor_memory_date(uint8_t);
String processor_wigi_mqtt_manager(const String&);
String processor_limit(const String&);
String processor_outputcontrol(const String&);
//
void setup_webserver(void);
  
// for reading CSV and JSON  
//
void sendErrorResponse(AsyncWebServerRequest *request, int code, const char* message);  
  
#endif
