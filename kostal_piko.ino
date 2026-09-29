/* Top level file for the KostalPiko Reader design
// Main part is now a ESP8266 processor connceted to Wifi, to be able to read values
//  for a KostalPiko device by means of webpages scraping.
//
*/

#include <Arduino.h>
#include <Wire.h> 
#include <ArduinoOTA.h>
//
#include "webscraper.h"
#include "kostal_piko.h"
#include "buttons_led.h"
#include "read_write_flash.h"
#include "wifi_connect.h"
#include "ntp.h"
#include "webserver.h"
#include "mqtt.h"
#include "littlefs_rw.h"
#include "csv_chunked.h"                  // for reading and formatting csv file
#include "json_chunked.h"                 // for reading and formatting json file
#include "ota.h"

const char compile_date[] = __DATE__ " " __TIME__;  // could give "Sep 22 2013 01:19:49";

//bool prev_midnight;
bool correct_base_energy = false;

//int error;

char c;

static unsigned long current_time = 0;

//
//
// Variables will change:
//int ledState = LOW;  // ledState used to set the LED

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;  // will store last time LED was updated

const long interval = 1000;  // interval at which to blink (milliseconds)

bool flash_ok;


//------------------------------------------
// Setup executed at power on, only once
//------------------------------------------
//
void setup() {
  // set communication speed with outside world
  //
  Serial.begin(115200 );  // native speed for esp8266
  printf("\n Started \n");
  //
  pinMode(4, OUTPUT);           // GPIO 4   
  digitalWrite(4, LOW);         // relay off
  // Setup 
  setup_button_led();                        // set button as input and led as output; set LED on
  //
  // Check LittleFS system
  //
  check_littlefs();                          // check if littlefs exists and file can be read
  if (littlefs_ok) {
      read_all_files();                      // read all credential files for connectivity 
//     printf("Wifi and Mqtt files are read\n");
      printf("\n\n");
   }
   else {
      // there is an error in the LittleFS system, flash led forever
      printf("Littlefs is not found; KostalPiko Reader placed on hold\n");
      drive_led_no_irq(false, false, 1000, 5000, -1) ; // forever
   }   
   //
   // Start wifi connection
   //
   setup_wifi();                       // create a wifi connection as AP or STA
   setup_webserver();                  // setup all services for web requests
   if (wifi_type == 0){
      printf("Wifi is not found at all; KostalPiko Reader placed on hold\n");
      drive_led_no_irq(false, false, 2000, 4000, -1) ; // forever
   }
   //
   // If we have an known SSID, connect to time server.
   //
   if (wifi_type == 3) {             //AP, time server can not be reached, gives hangup
      ntp_ok = false;
   }
   else {
      printf("Starting NTP connection\n");
      ntp_ok = false;
      // for (int i = 0; i<<10 || ntp_ok; i++){
      while (not ntp_ok){                         // toevoegen max 5 seconden
         ntp_ok = init_ntp();
         if (not ntp_ok) {
            delay (1000);              // retry after 1 second
         }   
      }
   }      
   //drive_led(false, false, 3000, 3000, -1) ; // forever

   
   get_ntptime();                   // get time 
   printf("ntp_ok after init_ntp function = %d\n", ntp_ok);
   //
   // Check external flash
   //
   flash_ok = flash_get_info();  // true if OK
   if (flash_ok) {
     printf("flash found and OK\n");
   }  
   else {
     printf("flash not found or not OK; KostalPiko Reader is placed on hold\n");
     drive_led_no_irq(false, false, 5000, 1000, -1) ; // forever
   }  

   // THERE MUST BE A VALID TIME!!!!!!
   //------------------------------------------------------
   // Check external flash, we need the ntp connection running for this
   //------------------------------------------------------ 
   if (ntp_ok) {                                 // read time and date for access to flash, only if ntp = OK
      flash_find_free_datagroup();
   }   
//   flash_write_address = active_datagroup * datagroup_size;
   //
   //------------------------------------------------------
   // Check if MQTT exist
   // if AP connection then no MQTT
   //-----------------------------------------------------
   mqtt_ok = false;
   if ((wifi_type == 2) || (wifi_type == 1)) {  // only when STA, dynamic or static
      if (file_mqtt_host != "" && file_mqtt_port != "" && file_mqtt_user != "") {   // there must be a host, port and user
         printf("Starting mqtt\n");
         mqtt_ok = setup_mqtt();
         if (mqtt_ok) {
            printf("Init_mqtt done\n");
         }
      }   
      else {
         printf("MQTT server not found; KostalPiko Reader will continue\n");
         drive_led_no_irq(false, false, 500, 500, 3) ; // 3 flashes
      }
   }      
   yield();
   //
   setup_ota();
   //
   //
   setup_webscraper();    // first read of Kostal-Piko webpage
   //
   // Enable IRQ for button press
   //
   digitalWrite(led_pin, HIGH);                   // turn the LED off
   setup_button_led_interrupt();                  // enable irq for button and led driving 
   //
   //
   printf("\n");
   printf("--------RESULT OF INITALIZATION--------\n");
   printf("littlefs_ok = %d\n", littlefs_ok);
   printf("wifi_type = %d\n", wifi_type);
   printf("ntp_ok = %d\n", ntp_ok);
   printf("flash_ok = %d\n", flash_ok);
   printf("mqtt_ok = %d\n", mqtt_ok);
   printf("----- END OF INITALIZATION------------\n\n");
}

//------------------------------------------
// loop executes continiously every 60 seconds
//------------------------------------------
//
void loop() {

   ArduinoOTA.handle();
   drive_led_no_irq(false, false, 500, 500, 2) ; // 2 flashes  
   
   while (true) {
      current_time = millis();                 // save the current time, will be used later
      //
      // Check for software update
      //
      //******************************************
      // Check wifi and ntp and MQTT connection
      //******************************************
      //
      // Still to do!
   

      //******************************************
      // Get all kind of inputs
      //******************************************
      //
      get_ntptime();                                  // get new time value
      //
      fetch_kostalpiko_webpage();                     // read the webpage
      //
      //
      //******************************************
      // Output values to mqtt & flash
      //******************************************
      //
      if (kostalpiko_code == HTTP_GET_CODE_OK) {      // code is "OK"or "NOK", webpage could be read
         printData();                                 // convert KostalPiko numbers to text
         //
         if (mqtt_ok ) {                              // is there a MQTT connection 
            publish_mqtt();                           // publish data
         }
//         else {
//            printf("New KostalPiko values found but MQTT not active\n");
//         }  
      }
      //
      // do not write values in flash if status of KostalPiko device != MPP
      //
      if (flash_ok){
         if (not flash_memory_full) {                                // free space
            if (flash_check_current_datablock (current_mday)){       // true if still on correct date
                  flash_write_dataset();                             // write system settings to external flash memory
            }
            else {                                                   // new datagroup needed  
               flash_make_new_header();                              // try to find a free datagroup
               if (not flash_memory_full) {                          // write if not full
                     flash_write_dataset();                         // write system settings to external flash memory 
               }
            } 
         }        
      }
      //     
      //
      // Wait for long time to slowdown the kostal-piko reader
      //
      while ((millis() - current_time) < loop_time) { // wait here until loop_time, 15 seconds, is passed
         yield();
         //
         ArduinoOTA.handle();                        // check OTA action
         //
         // check if data-group must be erased
         //
         if (erase_flag) {
            flash_erase_datagroup(erase_datagroup_number);
         }
         //
         if (sec5_flag) {
            printf("clear all\n");
            sec5_flag = false;
            clear_all_files();                      // erase all credentials
            restart = true;                         // restart KostalPiko reader
         }
         //
         // Restart ESP after IP/Password update
         //
         if (restart){
            printf("restart");
            delay(1000);
            ESP.restart();
         }
      }
      drive_led_no_irq(false, false, 250, 0, 1) ;     // 1 flash of 0.25 sec
   }
}  
