
#include <stdio.h>
#include <Arduino.h>
//#include <math.h>
#include <FloatToAscii.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncTCP.h>

#include "webscraper.h"
#include "littlefs_rw.h"
#include "ntp.h" 


PikoData data;

int kostalpiko_code;

//int HTTP_GET_CODE_OK = 200;

char char_current_power[8] = "0";
char char_total_energy[10] = "0";
char char_daily_energy[10] = "0";
char char_string1_voltage[6] = "0";
char char_string1_current[8] = "0";
char char_string2_voltage[6] = "0";
char char_string2_current[8] = "0";
char char_string3_voltage[6] = "0";
char char_string3_current[8] = "0";
char char_L1_output_voltage[6] = "0";
char char_L1_output_power[6] = "0";
char char_L2_output_voltage[6] = "0";
char char_L2_output_power[6] = "0";
char char_L3_output_voltage[6] = "0";
char char_L3_output_power[6] = "0";
char char_status[15] = "";
char char_kostalpiko_code[10] = "";

char char_base_total_energy[10] = "0";
char char_out_total_energy[10] = "0";
char char_out_daily_energy[10] = "0";

bool daily_energy_flag = false;

//
//
//
void setup_webscraper() {
   
   fetch_kostalpiko_webpage(); // get values from KostalPiko device
   //
   if (kostalpiko_code == HTTP_GET_CODE_OK) {      // code is "OK"or "NOK", webpage could be read
      printData();                                 // convert KostalPiko numbers to text
      //
//      if (data.status == "MPP") {
         data.base_totalEnergy = data.totalEnergy - data.dailyEnergy;                  // start energy of new day 
         data.out_dailyEnergy = data.dailyEnergy;                   // use value from KostalPiko
         data.out_totalEnergy = data.base_totalEnergy + data.out_dailyEnergy; // gives more accurate total energy
//      }
   }   
   else {
      printf("New KostalPiko values found but MQTT not active\n");
      data.base_totalEnergy = 0;
      data.out_dailyEnergy = 0;
      data.out_totalEnergy = 0;
   }  
}

//
// --- Fetch webpage ---
//
void fetch_kostalpiko_webpage() {

   HTTPClient http;
   WiFiClient client;

   http.begin(client, "http://" + file_kostalpiko_ip + "/");

   http.setAuthorization(file_kostalpiko_user.c_str(), file_kostalpiko_password.c_str()); // Basic authentication
   //
   //
   kostalpiko_code = http.GET();                        // code signals reading of website

   if (kostalpiko_code == HTTP_GET_CODE_OK) {
      parseHTML(http.getString());                      // retrieve values from html
      data.kostalpiko_code = "OK";
   } 
   else {
      printf("HTTP error: %d\n", kostalpiko_code);
      data.kostalpiko_code = "NOK";
   }
   //
   http.end();
}


// --- HTML helpers ---
//
//
String extractBetween(const String& html, const String& before, const String& after) {
   
   int start = html.indexOf(before);
   
   if (start == -1) return "";
   start += before.length();
   int end = html.indexOf(after, start);
   if (end == -1) return "";
   String val = html.substring(start, end);
   val.trim();
   return val;
}

//
// Find values & status in html from Kostal Piko
//
void parseHTML(const String& html) {
//   PikoData d;
   String values[20];
   int count = 0; 
   int pos = 0;

   while (count < 20) {
      int cell = html.indexOf("bgcolor=\"#FFFFFF\">", pos);
      if (cell == -1) break;
      int valStart = cell + 18;
      int valEnd   = html.indexOf("</td>", valStart);
      if (valEnd == -1) break;
      String v = html.substring(valStart, valEnd);
      v.trim();
      values[count++] = v;
      pos = valEnd;
   }
   //
   if (count >= 13) {
      data.currentpower = values[0].toInt();
      data.totalEnergy  = values[1].toFloat();
      data.dailyEnergy  = values[2].toFloat();
      data.str1Voltage  = values[3].toInt();
      data.l1Voltage    = values[4].toInt();
      data.str1Current  = values[5].toFloat();
      data.l1Power      = values[6].toInt();
      data.str2Voltage  = values[7].toInt();
      data.l2Voltage    = values[8].toInt();
      data.str2Current  = values[9].toFloat();
      data.l2Power      = values[10].toInt();
      //                
      data.str3Voltage  = 0;               // normal    
      data.str3Current  = 0.0;      
      data.l3Voltage    = values[11].toInt();
      data.l3Power      = values[12].toInt();
   }
   //
   data.status = "undefined";                             // set as unknown
   data.status = extractBetween(html, "supply ", "<");
   if (data.status == "") {
      data.status = extractBetween(html, "status</td>\n<td colspan=\"4\">\n  ", "<");
   }    
   if (data.status == "") {
      data.status = "noMPP";
   }   
   //
   // make correction for daily energy - set back to 0 at 00:00 - out_daily_energy
   // outgoing total energy will be total energy at day start + daily_energy
   // A flag will be set at 00:00 and reset if status = "MPP"  
   //
   if (current_hour == 0 && current_minute == 0) {
      daily_energy_flag = true;
   }
   else if (data.status == "MPP"){
      daily_energy_flag = false;
   }    
   //
   // check flag
   //
   if (daily_energy_flag) {
//      data.base_totalEnergy = data.totalEnergy;                  // start energy of new day 
      data.out_dailyEnergy = 0;                                  // force value to zero
//      data.out_totalEnergy = data.base_totalEnergy ;
   }
   else {   
      data.out_dailyEnergy = data.dailyEnergy;                   // use value from KostalPiko
//      data.out_totalEnergy = data.base_totalEnergy + data.out_dailyEnergy; // gives more accurate total energy
   }
}   



//
// Print value from Kostal Piko and convert them to strings (char)
//
void printData(void) {
   
//   printf("=== PIKO 4.2 ===\n");
//   printf("AC Power:      %d W\n",     data.currentpower);
//   printf("Total energy:  %.2f kWh\n", data.totalEnergy);
//   printf("Daily energy:  %.2f kWh\n", data.dailyEnergy);
//   printf("Status:        %s\n",       data.status.c_str());
//   printf("--- PV Strings ---\n");
//   printf("String 1:      %d V  %.2f A\n", data.str1Voltage, data.str1Current);
//   printf("String 2:      %d V  %.2f A\n", data.str2Voltage, data.str2Current);
//   printf("--- AC Phases ---\n");
//   printf("L1:            %d V  %d W\n", data.l1Voltage, data.l1Power);
//   printf("L2:            %d V  %d W\n", data.l2Voltage, data.l2Power);
//   printf("L3:            %d V  %d W\n", data.l3Voltage, data.l3Power);
   //
   // Convert to strings
   //
   itoa(data.currentpower, char_current_power, 10); //(integer, yourBuffer, base)
//   printf("currentpower =%d\n", data.currentpower);
//   printf("char_current_power in webscraper = %s\n", char_current_power);
   dtostrf(data.totalEnergy, 7, 0, char_total_energy);  // float,total_length,remainder, char
   dtostrf(data.dailyEnergy, 7, 2, char_daily_energy);
   //
   itoa(data.str1Voltage, char_string1_voltage, 10); //(integer, yourBuffer, base)
   dtostrf(data.str1Current, 7, 2, char_string1_current);  // float,total_length,remainder, char
   itoa(data.str2Voltage, char_string2_voltage, 10); //(integer, yourBuffer, base)
   dtostrf(data.str2Current, 7, 2, char_string2_current);  // float,total_length,remainder, char
   itoa(data.str3Voltage, char_string3_voltage, 10); //(integer, yourBuffer, base)
   dtostrf(data.str3Current, 7, 2, char_string3_current);  // float,total_length,remainder, char
   //
   itoa(data.l1Voltage, char_L1_output_voltage, 10); //(integer, yourBuffer, base)
   itoa(data.l1Power, char_L1_output_power, 10); //(integer, yourBuffer, base)
   itoa(data.l2Voltage, char_L2_output_voltage, 10); //(integer, yourBuffer, base)
   itoa(data.l2Power, char_L2_output_power, 10); //(integer, yourBuffer, base)
   itoa(data.l3Voltage, char_L3_output_voltage, 10); //(integer, yourBuffer, base)
   itoa(data.l3Power, char_L3_output_power, 10); //(integer, yourBuffer, base)

   data.status.toCharArray(char_status, 15); //copy string to char
   data.kostalpiko_code.toCharArray(char_kostalpiko_code, 10);
   //
   dtostrf(data.base_totalEnergy, 7, 0, char_base_total_energy);  // float,total_length,remainder, char
   dtostrf(data.out_totalEnergy, 7, 0, char_out_total_energy);
   dtostrf(data.out_dailyEnergy, 7, 2, char_out_daily_energy);
   
}
