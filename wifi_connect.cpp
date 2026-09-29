/*
** Make a connection to the Wifi network.
** This connection will be used for MQTT and for a webpage to set values
** and make settings
** 
** wifi_type = 0 if no connect with wifi
** wifi_type = 1 if STA with static IP
** wifi_type = 2 if STA with dynamic IP
** wifi_type = 3 if AP
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncTCP.h>
#include <string.h>

#include "wifi_connect.h"
#include "webserver.h"
#include "kostal_piko.h"
#include "littlefs_rw.h"

IPAddress localIP;
IPAddress localGateway;    // Set your Gateway IP address
IPAddress localSubnet; 
IPAddress localdns1; 
IPAddress localdns2; 


short int wifi_type = 0;
bool use_wifi_AP = false;  // true if STA gives error
short int wifi_count = 0;

byte bssid[6];
String str_bssid;
String str_wifi_hostname;

//
// Try to create a STA wifi connection, if without success create a AP connection
//
void setup_wifi() {

   use_wifi_AP = false;   // if true use AP
   wifi_type = 0;
   //
   // create network name
   //
   str_wifi_hostname = "";
   String hostname = "KostalPiko-";
   int randNumber = random(1000, 9999);  // 4 digit number
   String total_hostname = hostname + String(randNumber);
   
//   printf("total_hostname *%s*\n", total_hostname.c_str());
//   printf("file_wifi_networkname *%s*\n", file_wifi_networkname.c_str());
   
   //  
   //  if no networkkname is found (empty), use generate network name
   //
   if (file_wifi_networkname == ""){                   // name from file empty?
      str_wifi_hostname = total_hostname;              // use kostalpiko-xxx name
   }
   else {
      str_wifi_hostname = file_wifi_networkname;
   }     
   printf("WiFi hostname = *%s*\n", str_wifi_hostname.c_str());
   WiFi.hostname(str_wifi_hostname);
   //
   // Check if a ssid and password are defined, if not AP
   // 
   if(file_wifi_ssid=="" || file_wifi_password==""){
      printf("Undefined SSID or password, use AP (Access Point)\n");
      // NULL sets an open Access Point
      WiFi.softAP(total_hostname.c_str(), NULL);  // use generated networkname
      wifi_type = 3;                      // AP
   }
     //
     // Now detect for dynamic or static IPnumber connection
     // For dynamic IP address, IP & gateway & subnet must be empty
     //
   else if (file_wifi_ssid!="" && file_wifi_password!="" && strlen(file_wifi_ip.c_str())<=1){  // use string length because of \r\n in file
//  else if (file_wifi_ip=="" && file_wifi_gateway=="" && file_wifi_subnet==""){
      WiFi.mode(WIFI_STA);
      printf("Dynamic IP address\n");
      WiFi.begin(file_wifi_ssid.c_str(), file_wifi_password.c_str());  // start connection
      wifi_type = 2;                                 // dynamic IP
      }
   //
   // If IP, gateway, subnet & dns1 are given it's a static IPnumber connection
   //
   else if (file_wifi_ssid!="" && file_wifi_password!="" && file_wifi_ip!="" && file_wifi_gateway!="" && 
            file_wifi_subnet!="" && file_wifi_dns1!=""){
      WiFi.mode(WIFI_STA);
      printf("Static IP address\n");       
      localIP.fromString(file_wifi_ip.c_str());
      localGateway.fromString(file_wifi_gateway.c_str());
      localSubnet.fromString(file_wifi_subnet.c_str());
      localdns1.fromString(file_wifi_dns1.c_str());
      localdns2.fromString(file_wifi_dns2.c_str());
         
      //
      // Check if connection can be configured, if not --> AP
      //
      if (!WiFi.config(localIP, localGateway, localSubnet, localdns1, localdns2)){
        printf("STA with Static IP number failed to configure\n");
        use_wifi_AP = true;
      }
      else {
         WiFi.begin(file_wifi_ssid.c_str(), file_wifi_password.c_str());  // start connection
         wifi_type = 1;                                 // static IP
      } 
   }   
   else {
      printf("No correct parameters found for Wifi connection with static IP\n");
   }       
   //
   // Now, start the connection
   //
   if (use_wifi_AP) {                      // due to an error with STA use AP
      WiFi.softAP(total_hostname.c_str(), NULL);       // use generated networkname
   }
   //
   // Now check the Wifi.status
   //
   wifi_count = 0;
   printf("Check Wifi Connection.");
   while ((WiFi.status() != WL_CONNECTED) && (wifi_count < 15)){
      // NULL sets an open Access Point
      printf(".");
      delay (1000);                         // 1 second delay
      wifi_count = wifi_count + 1;          // 10 loops max
   } 
   printf("\r\n");
   //
   if (wifi_count == 15){                             // no success 
      printf("Failed to connect.\n");
      printf("Setting AP (Access Point)\n");
      WiFi.softAP(total_hostname.c_str(), NULL);          // fixed name
      wifi_type = 3;
   } 
   //
   if (wifi_type == 3) {
      IPAddress IP = WiFi.softAPIP();
     
      Serial.print("\nSoft-AP IP address = ");
      Serial.println(IP);
//     printf("AP IP address: %s",IP);
/*
    printf("new hostname = %s\n",WiFi.hostname());
    printf("Wifi_type = %d\n", wifi_type);
*/
  }    
   else if ((wifi_type == 2) || (wifi_type == 1)){  // connection STA, dynamic or static.
      if (wifi_type == 2) {
         printf("Connection with Dynamic IP address is done.\n"); 
      }
      else {
         printf("Connection with Static IP address\n"); 
      }       
      printf("\nWifi Connection OK with IP number = %s\n", WiFi.localIP().toString().c_str());   // print IPnumber for STA         
//    Serial.println(WiFi.localIP());           // print IPnumber for STA
//    printf("new hostname = %s\n", WiFi.hostname().c_str());   
//    printf("Wifi_type = %d\n", wifi_type);
  }
   else {
      printf("No wifi connection found!!\n");
   }  
   //
   // get MAC address
   //
   str_bssid =  WiFi.BSSIDstr();            // get MAC address as 6 bytes

}
