/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp8266-nodemcu-async-web-server-espasyncwebserver-library/
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

// Import required libraries
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FloatToAscii.h>
#include <string>
//
#include "LittleFS.h"
#include "webserver.h"
#include "wifi_connect.h"
#include "kostal_piko.h"
#include "progmem.h"
#include "ntp.h"
#include "littlefs_rw.h"
#include "read_write_flash.h"
#include "buttons_led.h"
#include "mqtt.h"
#include "csv_chunked.h"                         // for reading csv file
#include "json_chunked.h"                        // for reading csv file
#include "webscraper.h"
#include "ota.h"
//

bool restart = false;     // restart processor after switch from AP to STA network

//
int int_ItemNu;    // pointer in the switchtimes array
//
// Holds message if webpage not correctly filled-in
//      
String ts_Message = "No message";
bool ts_Message_error = false;
//
//char text_buffer[30];
//

//-----------------------------------------------
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


//-----------------------------------------------
// LOGIN values
//-----------------------------------------------
String web_login_user;
String web_login_password;
//-----------------------------------------------
// WIFI values
//-----------------------------------------------
String web_wifi_ssid;
String web_wifi_password;
String web_wifi_ip;
String web_wifi_gateway;
String web_wifi_subnet;
String web_wifi_dns1;
String web_wifi_dns2;
String web_wifi_networkname;
String web_wifi_mac;

//-----------------------------------------------
// MQTT values
//-----------------------------------------------
String web_mqtt_host;
String web_mqtt_port;
String web_mqtt_user;
String web_mqtt_password;

String web_kostalpiko_host;
String web_kostalpiko_user;
String web_kostalpiko_password;

String web_ota_password;

String on_off_state;  // state of on/off/auto mode

//String date_time;  // date and time from ntp server

// boolean for visibilty of passwords when viewing credentials
bool view_login_password = false;
bool view_wifi_password  = false;
bool view_mqtt_password  = false;
bool view_ota_password   = false;
bool view_kostalpiko_password = false;

int8_t int_csv_value = 0;
int8_t int_json_value = 0;

//
//
// The procesor will replace PLACEHOLDERS from the webpage (%..%) by values
// It does this for the Powermeter values webpage
//
String processor_index(const String &var) {
//
   //printf("%s\n", var);
   String substring;
   //
//      printf("last_compile = %s\n", last_compile);
   if(var == FPSTR(last_compile)){ 
      substring = compile_date; 
      return substring;
   }
   return String();
}

//
// The procesor will replace PLACEHOLDERS from the webpage (%..%) by values
// It does this for the Powermeter values webpage
//
String processor_value(const String &var) {
//
   //printf("%s\n", var);
   String substring;
   //
//      printf("char_current_power in webserver = %s\n", char_current_power);
   if(var == FPSTR(string_actual_power_w)){ 
//      printf("---\n");
//      printf("char_current_power as char = %s\n",char_current_power);
//      substring = String(char_current_power); 
      substring = char_current_power; 
//      printf("char_current_power %s\n",substring);
      return substring;
   }
   if(var == FPSTR(string_total_energy_kwh)){ 
      substring = char_total_energy;
      return substring;
   }  
   if(var == FPSTR(string_daily_energy_kwh)){ 
      substring = char_daily_energy;
      return substring;
   }  
   if(var == FPSTR(string_string1_voltage_v)){ 
      substring = char_string1_voltage;
      return substring;
   }  
   if(var == FPSTR(string_string1_current_a)){ 
      substring = char_string1_current;
      return substring;
   }  
   if(var == FPSTR(string_string2_voltage_v)){ 
      substring = char_string2_voltage;
      return substring;
   }  
   if(var == FPSTR(string_string2_current_a)){ 
      substring = char_string2_current;
      return substring;
   }  
   if(var == FPSTR(string_string3_voltage_v)){ 
      substring = char_string3_voltage;
      return substring;
   }  
   if(var == FPSTR(string_string3_current_a)){ 
      substring = char_string3_current;
      return substring;
   }  
   
   if(var == FPSTR(string_L1_output_voltage_v)){ 
      substring = char_L1_output_voltage;
      return substring;
   }  
   if(var == FPSTR(string_L1_output_power_w)){ 
      substring = char_L1_output_power;
      return substring;
   }  
   if(var == FPSTR(string_L2_output_voltage_v)){ 
      substring = char_L2_output_voltage;
      return substring;
   }  
   if(var == FPSTR(string_L2_output_power_w)){ 
      substring = char_L2_output_power;
      return substring;
   }  
    if(var == FPSTR(string_L3_output_voltage_v)){ 
      substring = char_L3_output_voltage;
      return substring;
   }  
   if(var == FPSTR(string_L3_output_power_w)){ 
      substring = char_L3_output_power;
      return substring;
   }  
    if(var == FPSTR(string_status)){ 
     substring = char_status;
     return substring;
   }  
   
   return String();
}
 

//-----------------------------------------------
// The procesor will replace PLACEHOLDERS from the webpage (%..%) by values
// It does this here for the memory webpage
//-----------------------------------------------
String processor_memory(const String &var) {
   //
   String substring;
   uint8_t  number;
   //
  //printf("%s\n", var);
  //printf("%s\n", var.length());
//  printf("var[4] = %d\n",(var[4] - '0'));
//  printf("var[5] = %d\n",(var[5] - '0'));

   number = (var[4]-'0') * 10 + (var[5]-'0');

   //
   if ((var[0] == 'D') && (var[1] == 'A') &&  (var[2] == 'T') && (var[3] == 'E')) {
   // 
      return (processor_memory_date(number));
   } 
   return String();  // in case there is nothing to find here
}


//---------------------------------------------------------
// Create date, "free" or "error", depending on data-goup
//  flag and date
//---------------------------------------------------------
String processor_memory_date(uint8_t number) {

   String substring;

   uint8_t  flags;
   uint16_t year;
   uint8_t  month;
   uint8_t  date;
//   printf("number = %d\n", number);
   flags = headers[number].datablock_flags;
//   printf("flags, %d\n", flags);
   year  = headers[number].datablock_year;
   month = headers[number].datablock_month;
   date  = headers[number].datablock_date;
   if (flags == 0xFF) {
      substring = "Free";
   }
   else if (flags == 0xAA){
      substring = String(year) + "/" + String (month) + "/" + String(date);
   }
   else {
      substring = "Error";
   }
   return (substring);
}

//-----------------------------------------------
// The procesor will replace PLACEHOLDERS from the webpage (%..%) by values
// It does this here for the memory webpage
//-----------------------------------------------
String processor_memory_json(const String &var) {
   //
   String substring;
   uint8_t  number;

   if (var == FPSTR("JSON_HEADER")) {   
      return "header found";
   }
   else {
      if (var == FPSTR("JSON_DATA_0")) {   
      return "json_data_0 found";
      }
   }
   return (substring);
}

//-----------------------------------------------
// The procesor will replace PLACEHOLDERS from the webpage (%..%) by values
// It does this here for the Wifimanager webpage
//-----------------------------------------------
String processor_login_wifi_mqtt_manager(const String &var) {
   //
   String substring;
   //
   //printf("%s\n", var);
   //printf("%s\n", var.length());
   //
   //
   // login_user
   if(var == FPSTR(wm_login_user)) {                 // "LOGIN_USER"
      if (file_login_user == ""){                    // content of file empty?
        substring = "" ;                             // nothing to fill in
      }   
      else {
        substring = file_login_user;
      }  
      return (substring);
   }
   // login_password
   if(var == FPSTR(wm_login_password)) {             // "LGIN_PASSWORD"       
      if (file_login_password == ""){
        substring = "";                           // no text on second line
      }   
      else {
        if (view_login_password) 
           substring = file_login_password;
        else
           substring = "**********";
      }  
      return (substring);
   }
 
   //
   //  WIFI settings
   //
   // ssid
   if(var == FPSTR(wm_wifi_ssid)) {                 // "WIFI_SSID"
      if (file_wifi_ssid == ""){
        substring = "" ;              // nothing to fill in
      }   
      else {
        substring = file_wifi_ssid;
      }  
      return (substring);
   }
   // password
   if(var == FPSTR(wm_wifi_password)) {                   
      if (file_wifi_password == ""){
        substring = "";                           // no text on second line
      }   
      else {
        if (view_wifi_password) 
           substring = file_wifi_password;
        else
           substring = "**********";
      }  
      return (substring);
   }
   // ip
   if(var == FPSTR(wm_wifi_ip)) {                   
      if (file_wifi_ip == ""){
        substring = "";                           // no text on second line
      }   
      else {
        substring = file_wifi_ip;
      }  
      return (substring);
   }
   // gateway
   if(var == FPSTR(wm_wifi_gateway)) {                   
      if (file_wifi_gateway == ""){
        substring = "";                           // no text on second line
      }   
      else {
        substring = file_wifi_gateway;
      }  
      return (substring);
   }
   // subnet
   if(var == FPSTR(wm_wifi_subnet)) {                   
      if (file_wifi_subnet == ""){
        substring = "";                           // no text on second line
      }   
      else {
        substring = file_wifi_subnet;
      }  
      return (substring);
   }
   // dns1
   if(var == FPSTR(wm_wifi_dns1)) {                   
      if (file_wifi_dns1 == ""){
        substring = "";                           // no text on second line
      }   
      else {
        substring = file_wifi_dns1;
      }  
      return (substring);
   }
   // dns2
   if(var == FPSTR(wm_wifi_dns2)) {                   
      if (file_wifi_dns2 == ""){
        substring = "";                           // no text on second line
      }   
      else {
        substring = file_wifi_dns2;
      }  
      return (substring);
   }
   // networkname
   if(var == FPSTR(wm_wifi_networkname)) {                   
      if (file_wifi_networkname == ""){
        substring = "";                           // no text on second line
      }   
      else {
        substring = file_wifi_networkname;
      }  
      return (substring);
   }
   // mac
   if(var == FPSTR(wm_wifi_mac)) {                   
        substring = str_bssid;
      return (substring);
   }
   // wifi type
   //
   // wifi_type = 0 if no connect with wifi
   // wifi_type = 1 if STA with static IP
   // wifi_type = 2 if STA with dynamic IP
   // wifi_type = 3 if AP
   //
   if(var == FPSTR(wm_wifi_type)) {                   
      if (wifi_type == 1){
        substring = FPSTR(STA_staticIP);                     
      }   
      if (wifi_type == 2){
        substring = FPSTR(STA_dynamicIP);
      }   
      if (wifi_type == 3){
        substring = FPSTR(APmode); 
      }   
      return (substring);
   }
   //
   if(var == FPSTR(mqtt_connected)) {                 // "mqtt host"
      if (mqtt_ok){
        substring = "<font color=\"green\"> MQTT connected" ;              // nothing to fill in
      }   
      else {
        substring = "<font color=\"red\"> MQTT not connected";
      }  
      return (substring);
   }
   
   
   // mqqtt host
   //
   if(var == FPSTR(wm_mqtt_host)) {                 // "mqtt host"
      if (file_mqtt_host == ""){
        substring = "" ;              // nothing to fill in
      }   
      else {
        substring = file_mqtt_host;
      }  
      return (substring);
   }
   // mqtt_port
   if(var == FPSTR(wm_mqtt_port)) {                   
      if (file_mqtt_port == ""){
        substring = "";                           // no text on second line
      }   
      else {
        substring = file_mqtt_port;
      }  
      return (substring);
   }
   // mqtt_user
   if(var == FPSTR(wm_mqtt_user)) {                   
      if (file_mqtt_user == ""){
        substring = "";                           // no text on second line
      }   
      else {
        substring = file_mqtt_user;
      }  
      return (substring);
   }
   // mqtt_password
   if(var == FPSTR(wm_mqtt_password)) {                   
      if (file_mqtt_password == ""){
        substring = "";                           // no text on second line
      }   
      else {
         if (view_mqtt_password) 
           substring = file_mqtt_password;
        else
           substring = "**********";
      }  
      return (substring);
   }
   //
   // KostalPiko device credentials
   //
   if(var == FPSTR(wm_kostalpiko_ip)) {                   
      if (file_kostalpiko_ip == ""){
        substring = "";                           // no text on second line
      }   
      else {
        substring = file_kostalpiko_ip;
      }  
      return (substring);
   }
   // Kostalpiko_user
   if(var == FPSTR(wm_kostalpiko_user)) {                   
      if (file_kostalpiko_user == ""){
        substring = "";                           // no text on second line
      }   
      else {
        substring = file_kostalpiko_user;
      }  
      return (substring);
   }
   // Kostalpiko_password
   if(var == FPSTR(wm_kostalpiko_password)) {                   
      if (file_kostalpiko_password == ""){
        substring = "";                           // no text on second line
      }   
      else {
         if (view_kostalpiko_password) 
           substring = file_kostalpiko_password;
        else
           substring = "**********";
      }  
      return (substring);
   }
   // Kostalpiko_connected
   if(var == FPSTR(kostalpiko_connected)) {                   
      if (data.kostalpiko_code == "OK"){
        substring = "<font color=\"green\"> KostalPiko connection OK" ;             
      }   
      else {
        substring = "<font color=\"red\"> KostalPiko connection NOK";
      }  
      return (substring);
   }
   //
   // ota_password
   //
   if(var == FPSTR(wm_ota_password)) {                   
         if (view_ota_password) 
           substring = file_ota_password;
        else
           substring = "**********";

      return (substring);
   }
   //
   return String();  // in case there is nothing to find here
}


//-----------------------------------------------
// Initialize webserver
//-----------------------------------------------
void setup_webserver() {
   
  server.on("/logout", HTTP_GET, [](AsyncWebServerRequest *request){
//     printf("/logout\n");
    request->send(401);
  });

  server.on("/logged-out", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, FPSTR(text_html),"Logged-out, please close your browser");
//    FPSTR(text_html)
  }); 

  //
  // Route for root / web page
   server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      if(!request->authenticate(file_login_user.c_str(), file_login_password.c_str()))
         return request->requestAuthentication();
      request->send(LittleFS,FPSTR(index_html), String(), false, processor_index);
   });
   //
   // Route to load style.css file
   server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(LittleFS, FPSTR(style_css), FPSTR(text_css));
   });
   //
   // Route to load favicon.png file
   server.on("/favicon.png", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(LittleFS, FPSTR(favicon_png), FPSTR(text_css));
   });
   //
   // Route to load date & time file
   server.on("/datetime", HTTP_GET, [](AsyncWebServerRequest *request) {
//    printf("date-time : %d\n", date-time);
   if (ntp_ok) {
      request->send(200, FPSTR(text_plain), date_time); //.c_str()
   }
   else {
      request->send(200, FPSTR(text_plain), "No date/time available"); 
   }
});

   

   //------------------------------------------------
   // Route for powermeter value page -- Full page refresh
   //------------------------------------------------
   server.on("/kostalpiko_value.html", HTTP_GET, [](AsyncWebServerRequest *request) {
      if(!request->authenticate(file_login_user.c_str(), file_login_password.c_str())){
         return request->requestAuthentication();
      }
      request->send(LittleFS, FPSTR(webpage_kostalpiko_value), String(), false, processor_value);
   });


   //------------------------------------------------
   // Route for memory page
   //------------------------------------------------
   //
   server.on("/memory.html", HTTP_GET, [](AsyncWebServerRequest *request) {
      if(!request->authenticate(file_login_user.c_str(), file_login_password.c_str())){
         return request->requestAuthentication();
      }
      request->send(LittleFS, FPSTR(webpage_memory), String());
   });

   //------------------------------------------------
   // Route for memory dates AJAX endpoint
   // Returns JSON array of date strings for all memory areas
   //------------------------------------------------
   server.on("/memory_dates", HTTP_GET, [](AsyncWebServerRequest *request) {
      if(!request->authenticate(file_login_user.c_str(), file_login_password.c_str())){
         return request->requestAuthentication();
      }
      String json;
      json.reserve(datagroup_number * 20 + 2); // ~20 chars per entry + brackets
      json = "[";
      for (uint8_t i = 0; i < datagroup_number; i++) {
         if (i > 0) json += ",";
         json += "\"";
         json += processor_memory_date(i);
         json += "\"";
      }
      json += "]";
      request->send(200, "application/json", json);
   });

 

   //------------------------------------------------
   // Web settings for Login, Wifi & SSID credentials
   //------------------------------------------------
   // web page for wifi credentials
   // Web Server Root URL
   server.on("/signin_manager.html", HTTP_GET, [](AsyncWebServerRequest *request){
      if(!request->authenticate(file_login_user.c_str(), file_login_password.c_str())){
         return request->requestAuthentication();
      }
      view_login_password = true;
      request->send(LittleFS, "/signin_manager.html", "text/html", false, processor_login_wifi_mqtt_manager);
   });
     
   
   server.on("/login_manager_update", HTTP_POST, [](AsyncWebServerRequest *request) {    // submit changes
      if(!request->authenticate(file_login_user.c_str(), file_login_password.c_str())){
         return request->requestAuthentication();
      }
     //
      view_login_password= false;
      // login user
      if (request->hasParam(FPSTR(wm_login_user), true)) {
         web_login_user = request->getParam(FPSTR(wm_login_user), true)->value();
      } 
      else {
         web_login_user = "";
      }
      printf("Login user set to: %s\n", web_login_user);
      writeFile(LittleFS, "/login_user.txt", web_login_user.c_str());    // Write file to save value
      //
      // login password
      if (request->hasParam(FPSTR(wm_login_password), true)) {
         web_login_password = request->getParam(FPSTR(wm_login_password), true)->value();
      } 
      else {
         web_login_password = "";
      }
      printf("Login_Password set to: %s\n", web_login_password);
      writeFile(LittleFS, "/login_password.txt", web_login_password.c_str());    // Write file to save value
      // 
      restart = true;
      request->send(200, "text/plain", "Login changed. ESP will restart");
   });
   
   
   // web page for wifi credentials
   // Web Server Root URL
   server.on("/wifimanager.html", HTTP_GET, [](AsyncWebServerRequest *request){
      if(!request->authenticate(file_login_user.c_str(), file_login_password.c_str())){
         return request->requestAuthentication();
      }
      view_wifi_password = true; // don't show password
      request->send(LittleFS, "/wifimanager.html", "text/html", false, processor_login_wifi_mqtt_manager);
   });
    
   server.on("/wifimanager_update", HTTP_POST, [](AsyncWebServerRequest *request) {    // submit changes
      if(!request->authenticate(file_login_user.c_str(), file_login_password.c_str())){
         return request->requestAuthentication();
      }
     //
      view_wifi_password= false;
      //
      // wifi ssid
      if (request->hasParam(FPSTR(wm_wifi_ssid), true)) {
         web_wifi_ssid = request->getParam(FPSTR(wm_wifi_ssid), true)->value();
      } 
      else {
         web_wifi_ssid = "";
      }
      printf("Wifi_SSID set to: %s\n", web_wifi_ssid);
      writeFile(LittleFS, "/wifi_ssid.txt", web_wifi_ssid.c_str());    // Write file to save value
      //
      // wifi password
      if (request->hasParam(FPSTR(wm_wifi_password), true)) {
         web_wifi_password = request->getParam(FPSTR(wm_wifi_password), true)->value();
      } 
      else {
         web_wifi_password = "";
      }
      printf("Wifi_Password set to: %s\n", web_wifi_password.c_str());
      writeFile(LittleFS, "/wifi_password.txt", web_wifi_password.c_str());    // Write file to save value
      //
      // wifi ip
      if (request->hasParam(FPSTR(wm_wifi_ip), true)) {
         web_wifi_ip = request->getParam(FPSTR(wm_wifi_ip), true)->value();
      } 
      else {
         web_wifi_ip = "";
      }
      printf("Wifi_IP set to: %s\n",web_wifi_ip);
      writeFile(LittleFS, "/wifi_ip.txt", web_wifi_ip.c_str());     // Write file to save value
      //
      // wifi gateway
      if (request->hasParam(FPSTR(wm_wifi_gateway), true)) {
         web_wifi_gateway = request->getParam(FPSTR(wm_wifi_gateway), true)->value();
      } 
      else {
         web_wifi_gateway = "";
      }
      printf("Wifi_Gateway set to: %s\n", web_wifi_gateway);
      writeFile(LittleFS, "/wifi_gateway.txt", web_wifi_gateway.c_str());     // Write file to save value
      //
      // wifi Subnet
      if (request->hasParam(FPSTR(wm_wifi_subnet), true)) {
         web_wifi_subnet = request->getParam(FPSTR(wm_wifi_subnet), true)->value();
      } 
      else {
         web_wifi_subnet = "";
      }
      printf("Wifi_Subnet set to: %s\n", web_wifi_subnet);
      writeFile(LittleFS, "/wifi_subnet.txt", web_wifi_subnet.c_str());     // Write file to save value
      //
      // wifi dns1
      if (request->hasParam(FPSTR(wm_wifi_dns1), true)) {
         web_wifi_dns1 = request->getParam(FPSTR(wm_wifi_dns1), true)->value();
      } 
      else {
         web_wifi_dns1 = "";
      }
      printf("Wifi_Primary_DNS set to: %s\n", web_wifi_dns1);
      writeFile(LittleFS, "/wifi_dns1.txt", web_wifi_dns1.c_str());     // Write file to save value
      //
      // wifi dns2
      if (request->hasParam(FPSTR(wm_wifi_dns2), true)) {
         web_wifi_dns2 = request->getParam(FPSTR(wm_wifi_dns2), true)->value();
      } 
      else {
         web_wifi_dns2 = "";
      }
      printf("Wifi_Secondary_DNS set to: %s\n", web_wifi_dns2);
      writeFile(LittleFS, "/wifi_dns2.txt", web_wifi_dns2.c_str());     // Write file to save value    //
      //
      // wifi networkname
      if (request->hasParam(FPSTR(wm_wifi_networkname), true)) {
         web_wifi_networkname = request->getParam(FPSTR(wm_wifi_networkname), true)->value();
      } 
      else {
         web_wifi_networkname = "";
      }
      if (strlen (web_wifi_networkname.c_str()) > 30) {                                          // check length of name
         web_wifi_networkname = web_wifi_networkname.substring(0,30);
      }         
      printf("Wifi_Network_name set to: %s\n", web_wifi_networkname);
      writeFile(LittleFS, "/wifi_networkname.txt", web_wifi_networkname.c_str());     // Write file to save value    //
      //
      // wifi mac
      if (request->hasParam(FPSTR(wm_wifi_mac), true)) {
         web_wifi_mac = request->getParam(FPSTR(wm_wifi_mac), true)->value();
      } 
      else {
         web_wifi_mac = "";
      }
      printf("Wifi_MAC: %s\n", web_wifi_mac);
//      writeFile(LittleFS, "/wifi_networkname.txt", web_wifi_networkname.c_str());     // Write file to save value    //
      //
      restart = true;
      request->send(200, "text/plain", "Network credentials changed. ESP will restart, reconnect to your home network and go to IP address: " + web_wifi_ssid);
   });
      
   //------------------------------------------------
   // Web settings for MQTT Credentials
   //------------------------------------------------
   //  server.on("/websettings", HTTP_GET, [](AsyncWebServerRequest *request) { 
   //     request->send(LittleFS, FPSTR(websettings), "text/html", false, processor);
   //  });

   // web page for mqtt credentials
   // Web Server Root URL
   server.on("/mqttmanager.html", HTTP_GET, [](AsyncWebServerRequest *request){
      if(!request->authenticate(file_login_user.c_str(), file_login_password.c_str())){
         return request->requestAuthentication();
      }
      view_mqtt_password = true; // show password
      request->send(LittleFS, "/mqttmanager.html", "text/html", false, processor_login_wifi_mqtt_manager);
   });
    
   server.on("/mqttmanager_update", HTTP_POST, [](AsyncWebServerRequest *request) {
      if(!request->authenticate(file_login_user.c_str(), file_login_password.c_str())){
         return request->requestAuthentication();
      }
     //
      view_mqtt_password = false; // don't show password
      //
      // mqtt host
      if (request->hasParam(FPSTR(wm_mqtt_host), true)) {
         web_mqtt_host = request->getParam(FPSTR(wm_mqtt_host), true)->value();
      } 
      else {
         web_mqtt_host = "";
      }
      printf("MQTT_Host set to: %s\n", web_mqtt_host.c_str());
      writeFile(LittleFS, "/mqtt_host.txt", web_mqtt_host.c_str());    // Write file to save value
      //
      // mqtt port
      if (request->hasParam(FPSTR(wm_mqtt_port), true)) {
         web_mqtt_port = request->getParam(FPSTR(wm_mqtt_port), true)->value();
      } 
      else {
         web_mqtt_port = "";
      }
      printf("MQTT Port set to: %s\n", web_mqtt_port.c_str());
      writeFile(LittleFS, "/mqtt_port.txt", web_mqtt_port.c_str());    // Write file to save value
      //
      // mqtt user
      if (request->hasParam(FPSTR(wm_mqtt_user), true)) {
         web_mqtt_user = request->getParam(FPSTR(wm_mqtt_user), true)->value();
      } 
      else {
         web_mqtt_user = "";
      }
      printf("MQTT_User set to: %s\n", web_mqtt_user.c_str());
      writeFile(LittleFS, "/mqtt_user.txt", web_mqtt_user.c_str());     // Write file to save value
      //
      // mqtt password
      if (request->hasParam(FPSTR(wm_mqtt_password), true)) {
         web_mqtt_password = request->getParam(FPSTR(wm_mqtt_password), true)->value();
      } 
      else {
         web_mqtt_password = "";
      }
      printf("MQTT_password set to: %d\n", web_mqtt_password.c_str());
      writeFile(LittleFS, "/mqtt_password.txt", web_mqtt_password.c_str());     // Write file to save value
      //
      //
      restart = true;
      request->send(200, "text/plain", "MQTT credentials changed. MQTT settings are saved, KostalPiko Reader will restart now!");
   });

   // web page for kostalpiko credentials
   // Web Server Root URL
   server.on("/kostalpikomanager.html", HTTP_GET, [](AsyncWebServerRequest *request){
      if(!request->authenticate(file_login_user.c_str(), file_login_password.c_str())){
         return request->requestAuthentication();
      }
      view_kostalpiko_password = true; // show password
      request->send(LittleFS, "/kostalpikomanager.html", "text/html", false, processor_login_wifi_mqtt_manager);
   });
    
   server.on("/kostalpikomanager_update", HTTP_POST, [](AsyncWebServerRequest *request) {
      if(!request->authenticate(file_login_user.c_str(), file_login_password.c_str())){
         return request->requestAuthentication();
      }
     //
      view_kostalpiko_password = false; // don't show password
      //
      // kostalpiko host
      if (request->hasParam(FPSTR(wm_kostalpiko_ip), true)) {
         web_kostalpiko_host = request->getParam(FPSTR(wm_kostalpiko_ip), true)->value();
      } 
      else {
         web_kostalpiko_host = "";
      }
      printf("KostalPiko_Host set to: %s\n", web_kostalpiko_host.c_str());
      writeFile(LittleFS, "/kostalpiko_ip.txt", web_kostalpiko_host.c_str());    // Write file to save value
      //
      // kostalpiko user
      if (request->hasParam(FPSTR(wm_kostalpiko_user), true)) {
         web_kostalpiko_user = request->getParam(FPSTR(wm_kostalpiko_user), true)->value();
      } 
      else {
         web_kostalpiko_user = "";
      }
      printf("KostalPiko_User set to: %s\n", web_kostalpiko_user.c_str());
      writeFile(LittleFS, "/kostalpiko_user.txt", web_kostalpiko_user.c_str());     // Write file to save value
      //
      // kostalpiko password
      if (request->hasParam(FPSTR(wm_kostalpiko_password), true)) {
         web_kostalpiko_password = request->getParam(FPSTR(wm_kostalpiko_password), true)->value();
      } 
      else {
         web_kostalpiko_password = "";
      }
      printf("KostalPiko_password set to: %d\n", web_kostalpiko_password.c_str());
      writeFile(LittleFS, "/kostalpiko_password.txt", web_kostalpiko_password.c_str());     // Write file to save value
      //
      //
      restart = true;
      request->send(200, "text/plain", "KostalPiko credentials changed. KostalPiko settings are saved, KostalPiko Reader will restart now!");
   });

   // web page for OTA credentials
   // Web Server Root URL
   server.on("/otamanager.html", HTTP_GET, [](AsyncWebServerRequest *request){
      if(!request->authenticate(file_login_user.c_str(), file_login_password.c_str())){
         return request->requestAuthentication();
      }
      view_ota_password = true; // show password
      request->send(LittleFS, "/otamanager.html", "text/html", false, processor_login_wifi_mqtt_manager);
   });
    
   server.on("/otamanager_update", HTTP_POST, [](AsyncWebServerRequest *request) {
      if(!request->authenticate(file_login_user.c_str(), file_login_password.c_str())){
         return request->requestAuthentication();
      }
     //
      view_ota_password = false; // don't show password
      //
      //
      // ota password
      if (request->hasParam(FPSTR(wm_ota_password), true)) {
         web_ota_password = request->getParam(FPSTR(wm_ota_password), true)->value();
      } 
      else {
         web_ota_password = "";
      }
      printf("OTA password set to: %d\n", web_ota_password);
      writeFile(LittleFS, "/ota_password.txt", web_ota_password.c_str());     // Write file to save value
      //
      //
      restart = true;
      request->send(200, "text/plain", "OTA password changed. OTA password is saved, KostalPiko Reader will restart now!");
   });


   //------------------------------------------------
   // Route for system settings page
   //------------------------------------------------
   server.on("/systemsettings.html", HTTP_GET, [](AsyncWebServerRequest *request) {
      if(!request->authenticate(file_login_user.c_str(), file_login_password.c_str())){
         return request->requestAuthentication();
      }
      request->send(LittleFS, FPSTR(webpage_systemsettings), String());
   });
   //
   //
   server.on( webpage_show_network_credentials, HTTP_GET, [](AsyncWebServerRequest *request) {
      if(!request->authenticate(file_login_user.c_str(), file_login_password.c_str())){
         return request->requestAuthentication();
      }
      request->send(LittleFS,  FPSTR(webpage_show_network_credentials), FPSTR(text_html), false, processor_login_wifi_mqtt_manager);
   });
   //
   //
   server.on(toggle_login_password, HTTP_GET, [](AsyncWebServerRequest *request) {
      if(!request->authenticate(file_login_user.c_str(), file_login_password.c_str())){
         return request->requestAuthentication();
      }
      view_login_password = !view_login_password;
      request->send(LittleFS,  FPSTR(webpage_show_network_credentials), FPSTR(text_html), false, processor_login_wifi_mqtt_manager);
   });
   //
   server.on(toggle_wifi_password, HTTP_GET, [](AsyncWebServerRequest *request) {
      if(!request->authenticate(file_login_user.c_str(), file_login_password.c_str())){
         return request->requestAuthentication();
      }
      view_wifi_password = !view_wifi_password;
      request->send(LittleFS, FPSTR(webpage_show_network_credentials), FPSTR(text_html), false, processor_login_wifi_mqtt_manager);
   });
   //
   server.on(toggle_mqtt_password, HTTP_GET, [](AsyncWebServerRequest *request) {
      if(!request->authenticate(file_login_user.c_str(), file_login_password.c_str())){
         return request->requestAuthentication();
      }
      view_mqtt_password = !view_mqtt_password;
   //  printf("view_mqtt_password = %d\n", view_mqtt_password);
      request->send(LittleFS, FPSTR(webpage_show_network_credentials), FPSTR(text_html), false, processor_login_wifi_mqtt_manager);
   });
   //
   server.on(toggle_kostalpiko_password, HTTP_GET, [](AsyncWebServerRequest *request) {
      if(!request->authenticate(file_login_user.c_str(), file_login_password.c_str())){
         return request->requestAuthentication();
      }
      view_kostalpiko_password = !view_kostalpiko_password;
   //  printf("view_kostalpiko_password = %d\n", view_kostalpiko_password);
      request->send(LittleFS, FPSTR(webpage_show_network_credentials), FPSTR(text_html), false, processor_login_wifi_mqtt_manager);
   });
   // OTA password
   server.on(toggle_ota_password, HTTP_GET, [](AsyncWebServerRequest *request) {
      if(!request->authenticate(file_login_user.c_str(), file_login_password.c_str())){
         return request->requestAuthentication();
      }
      view_ota_password = !view_ota_password;
   //  printf("view_ota_password = %d\n", view_ota_password);
      request->send(LittleFS, FPSTR(webpage_show_network_credentials), FPSTR(text_html), false, processor_login_wifi_mqtt_manager);
   });
    
   //
   // Reset webpage
   //
   server.on("/reset.html", HTTP_GET, [](AsyncWebServerRequest *request) {
      if(!request->authenticate(file_login_user.c_str(), file_login_password.c_str())){
         return request->requestAuthentication();
      }
      request->send(LittleFS, FPSTR(webpage_reset), "text/html");
   });
   //
   // buttons on reset webpage
   //
   server.on("/reset_wifi", HTTP_GET, [](AsyncWebServerRequest *request) {
      if(!request->authenticate(file_login_user.c_str(), file_login_password.c_str())){
         return request->requestAuthentication();
      }
      clear_wifi_files();    // will restart the KostalPiko Reader 
      request->send(LittleFS, FPSTR(webpage_reset), "text/html");
      // restart kostal-piko reader
      restart = true;
   });
   //
   //
   server.on("/reset_mqtt", HTTP_GET, [](AsyncWebServerRequest *request) {
      if(!request->authenticate(file_login_user.c_str(), file_login_password.c_str())){
         return request->requestAuthentication();
      }
      clear_mqtt_files();    // will restart the KostalPiko Reader 
      request->send(LittleFS, FPSTR(webpage_reset), "text/html");
      // restart kostal-piko reader
      restart = true;
   });
   //
   //
   server.on("/reset_kostalpiko", HTTP_GET, [](AsyncWebServerRequest *request) {
      if(!request->authenticate(file_login_user.c_str(), file_login_password.c_str())){
         return request->requestAuthentication();
      }
      clear_kostalpiko_files();    // will restart the KostalPiko Reader 
      request->send(LittleFS, FPSTR(webpage_reset), "text/html");
      // restart kostal-piko reader
      restart = true;
   });
   //
   //
   server.on("/reset_ota", HTTP_GET, [](AsyncWebServerRequest *request) {
      if(!request->authenticate(file_login_user.c_str(), file_login_password.c_str())){
         return request->requestAuthentication();
      }
      clear_ota_files();    // will restart the KostalPiko Reader 
      request->send(LittleFS, FPSTR(webpage_reset), "text/html");
      // restart kostal-piko reader
      restart = true;
   });
   //
   //
   // Erase flash chip completely, device will restart
   //
   server.on("/reset_memory", HTTP_GET, [](AsyncWebServerRequest *request) {
      if(!request->authenticate(file_login_user.c_str(), file_login_password.c_str())){
         return request->requestAuthentication();
      }
      request->send(200, "text/plain", "Memory device will be erased. Then the KostalPiko Reader will restart. This takes a while");
//    delay(5000);
      flash_erase_chip();
//    request->send(LittleFS, FPSTR(webpage_reset), "text/html");
      request->send(200, "text/plain", "Memory will be completely erased, KostalPiko reader will restart now!");
      restart = true;
   });
   //
   //

   //
   server.on("/reset_all", HTTP_GET, [](AsyncWebServerRequest *request) {
      if(!request->authenticate(file_login_user.c_str(), file_login_password.c_str())){
         return request->requestAuthentication();
      }
      clear_all_files(); 
      request->send(LittleFS, FPSTR(index_html), "text/html");
   });
   //
   // from memory webpage
   //
   server.on("/memory_update", HTTP_GET, [](AsyncWebServerRequest *request) {
      if(!request->authenticate(file_login_user.c_str(), file_login_password.c_str())){
         return request->requestAuthentication();
      }
     // read data set as CSV, JSON or just DELETE data sets
   int paramsNr = request->params();
   //
   // Check is a number is received
   //
   if (paramsNr == 0) {
      sendErrorResponse(request, 400, "No parameters provided");
      return;
   }   
   else {
//      printf("%d Params are sent \n", paramsNr);
   }

   //
   // ===== CSV DOWNLOAD =====
   //
   if (request->hasParam("CSV")) {
      String str_csv_value = request->getParam("CSV")->value();
      //
      // Validate CSV parameter is numeric
      if (str_csv_value.length() == 0 || !isdigit(str_csv_value.charAt(0))) {
         sendErrorResponse(request, 400, "Invalid CSV parameter - must be numeric");
         return;
      }
      //
      int8_t int_csv_value = atoi(str_csv_value.c_str());
      //
      // Validate CSV index is in valid range
      if (int_csv_value < 0 || int_csv_value >= datagroup_number) {
        sendErrorResponse(request, 400, "CSV index out of range");
        return;
      }
      //
      // Check if datablock exists and is valid
     if (headers[int_csv_value].datablock_flags != 0xAA) {
         sendErrorResponse(request, 400, "Data block not found or not in use");
         return;
      }
      //
      printf("int_csv_value = %d\n",int_csv_value);
      // Create and send CSV response
      AsyncAbstractResponse *response = new CSVChunkedResponse(int_csv_value);
      if (response == nullptr) {
         sendErrorResponse(request, 500, "Failed to create CSV response");
         return;
      }

      request->send(response);
      //    return;
   }


   //
   // ===== JSON DOWNLOAD =====
   //
   else if (request->hasParam("JSON")) {  // for HTTP_GET do not end with true
      String str_json_value = request->getParam("JSON")->value();
      //      
      // Validate CSV parameter is numeric
      if (str_json_value.length() == 0 || !isdigit(str_json_value.charAt(0))) {
         sendErrorResponse(request, 400, "Invalid JSON parameter - must be numeric");
         return;
      }
      //
      int8_t int_json_value = atoi(str_json_value.c_str());
      //
      // Validate JSON index is in valid range (400 = bad request)
      if (int_json_value < 0 || int_json_value >= datagroup_number) {
        sendErrorResponse(request, 400, "JSON index out of range");
        return;
      }
      //
      // Check if datablock exists and is valid (404 = not found)
     if (headers[int_json_value].datablock_flags != 0xAA) {
         sendErrorResponse(request, 404, "Data block not found or not in use");
         return;
      }
      //
      // Create and send JSON response (500 = internal server error)
      AsyncAbstractResponse *response = new JSONChunkedResponse(int_json_value);
      if (response == nullptr) {
         sendErrorResponse(request, 500, "Failed to create JSON response");
         return;
      }

      request->send(response);
      //    return;
   } 
   // 
   //
   // ===== DELETE DOWNLOAD =====
   //
   else if (request->hasParam("DELETE")) {  // for HTTP_GET do not end with true
      printf("Delete pressed\n");
      String str_delete_value = request->getParam("DELETE")->value();
      int_delete_value = atoi(str_delete_value.c_str());

      //
      // If a data-group must be deleted, 4 blocks of 64 kByte must be erased.
      // The value received here, points to the right blocks.
      // Also the pointer to this block must be set to 0x00, to signal that the block is free
      // Pointers can only be erased by moving all pointers to the other pointer-area
      // Erase only allowed if the pointer is in use. not allowed if already erased or not in use
      //
      erase_datagroup_number = int_delete_value;                             // get datagroup number
      if (headers[int_delete_value].datablock_flags != 0xFF) {                // only erase if in use
         erase_flag = true;                                                   // real erase in read_write_flash
      }   
      request->send(LittleFS, FPSTR(webpage_memory), "text/html", false, processor_memory);
   } 
   // ===== UNKNOWN PARAMETER =====
   else {
      sendErrorResponse(request, 400, "Unknown parameter - use CSV, JSON or DELETE buttons");
      return;
   }  
});

   // 404 and 405 error handler
   server.onNotFound([](AsyncWebServerRequest *request) {
      if (request->method() == HTTP_GET) {
         // Handle 404 Not Found error
         //printf("Web Server: Not Found\n");
         request->send(404, FPSTR(text_html), FPSTR(webpage_not_found));
      } else {
         // Handle 405 Method Not Allowed error
         //printf("Web Server: Method Not Allowed\n");
         request->send(405, FPSTR(text_html), FPSTR(method_not_allowed));
      }
   });

  server.begin();
}

void sendErrorResponse(AsyncWebServerRequest *request, int code, const char* message) {
   request->send(code, "application/json", String("{\"error\":\"") + message + "\"}");
}    
