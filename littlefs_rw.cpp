#include <Arduino.h>
#include <FS.h>
#include <string.h>

#include "LittleFS.h"
#include "littlefs_rw.h"
#include "progmem.h"
#include "webserver.h"

String file_login_user;
String file_login_password;

String file_wifi_ssid;
String file_wifi_password;
String file_wifi_ip;
String file_wifi_gateway;
String file_wifi_subnet;
String file_wifi_dns1;
String file_wifi_dns2;
String file_wifi_networkname;

String file_mqtt_host;
String file_mqtt_port;
String file_mqtt_user;
String file_mqtt_password;

String file_kostalpiko_ip;
String file_kostalpiko_user;
String file_kostalpiko_password;

String file_ota_password;

bool littlefs_ok;
bool debug = false;  // used for printing

//----------------------------------------------------
// Read File from LittleFS
//----------------------------------------------------
String readFile(fs::FS &fs, const char * path){
  
   printf("Reading file: %s\r\n", path);
   
   File file = fs.open(path,"r");
   if(!file || file.isDirectory()){
      printf("- failed to open file for reading\n");
      return String();
   }
   //
   String fileContent;
   while(file.available()){
      fileContent = file.readStringUntil('\n');
      break;     
   }
   file.close();
   return fileContent;
}

//----------------------------------------------------
// Write file to LittleFS
//----------------------------------------------------
void writeFile(fs::FS &fs, const char * path, const char * message){
//  printf("Writing file: %s\r\n", path);

   File file = fs.open(path,"w");
   if(!file){
//     printf("- failed to open file for writing\n");
      return;
   }
//   printf("%s\n", message);
   if (strlen(message) == 0) {
      file.close(); 
//     printf("message length = %d\n", strlen(message));
//     printf("- file cleared\n");
   }
   else {
      if(file.print(message)){
        printf("- file written\n");
      } else {
        printf("- write failed\n");
      }
   }
}

//----------------------------------------------------
// Check if littefs exists and file can be read
//----------------------------------------------------
void check_littlefs(void){

   // check if there is a LittleFS file system  
   if(!LittleFS.begin()){
//     printf("littlefs_error = %s\n",FPSTR(littlefs_error));
      littlefs_ok = false;
   }
   else{
//       printf("littlefs_success %s\n",FPSTR(littlefs_success));
   }    
   
   // Check if at least one file exists in the file system
   File file = LittleFS.open(FPSTR(index_html), "r");
   if(!file){
//     printf("littlefs_check = %s\n",FPSTR(littlefs_check));
      littlefs_ok = false;
   }
   else {
      littlefs_ok = true;
   }    
}

//---------------------------------------------------- 
// Read all login files
//----------------------------------------------------
void read_login_files(void) {
   //
   // read files with login credentials
   //
   file_login_user        = readFile(LittleFS, "/login_user.txt");
   file_login_password    = readFile(LittleFS, "/login_password.txt");
   //
   if (debug) {
      printf("Content of file_login_user = '%s'\n", file_login_user.c_str());
      printf("Content of file_login_password = '%s'\n", file_login_password.c_str());
   }
}

//---------------------------------------------------- 
// Read all wifi files
//----------------------------------------------------
void read_wifi_files(void) {
   //
   // read files with WiFi credentials
   //
   file_wifi_ssid         = readFile(LittleFS, "/wifi_ssid.txt");
   file_wifi_password     = readFile(LittleFS, "/wifi_password.txt");
   file_wifi_ip           = readFile(LittleFS, "/wifi_ip.txt");
   file_wifi_gateway      = readFile(LittleFS, "/wifi_gateway.txt");  
   file_wifi_subnet       = readFile(LittleFS, "/wifi_subnet.txt");
   file_wifi_dns1         = readFile(LittleFS, "/wifi_dns1.txt");
   file_wifi_dns2         = readFile(LittleFS, "/wifi_dns2.txt");
   file_wifi_networkname  = readFile(LittleFS, "/wifi_networkname.txt");
   //
   if (debug) {
      printf("Content of file_wifi_ssid = '%s'\n", file_wifi_ssid.c_str());
      printf("Content of file_wifi_password = '%s'\n", file_wifi_password.c_str());
      printf("Content of file_wifi_ip = '%s'\n", file_wifi_ip.c_str());
      printf("Content of file_wifi_gateway = '%s'\n", file_wifi_gateway.c_str());
      printf("Content of file_wifi_subnet = '%s'\n", file_wifi_subnet.c_str());
      printf("Content of file_wifi_dns1 = '%s'\n", file_wifi_dns1.c_str());
      printf("Content of file_wifi_dns2 = '%s'\n", file_wifi_dns2.c_str());
      printf("Content of file_wifi_networkname = '%s'\n", file_wifi_networkname.c_str());
   }
   //
 }
  
//---------------------------------------------------- 
// Read all mqtt files
//----------------------------------------------------
void read_mqtt_files(void) {
   //
   // read files with MQTT credentials
   //

   File mqtt_host_file = LittleFS.open("/mqtt_host.txt", "r");              // open for read
   if(!mqtt_host_file){
      Serial.println("No Saved Data in mqtt_host!"); 
   }
   else{     
      file_mqtt_host    = mqtt_host_file.readStringUntil('\n');
   }
   //  
   File mqtt_port_file = LittleFS.open("/mqtt_port.txt", "r");              // open for read
   if(!mqtt_port_file){
      Serial.println("No Saved Data in mqtt_port!"); 
   }
   else{     
      file_mqtt_port    = mqtt_port_file.readStringUntil('\n');
   }
   //  
   File mqtt_user_file = LittleFS.open("/mqtt_user.txt", "r");              // open for read
   if(!mqtt_user_file){
      Serial.println("No Saved Data in mqtt_user!"); 
   }
   else{     
      file_mqtt_user    = mqtt_user_file.readStringUntil('\n');
   }
   //  
   File mqtt_password_file = LittleFS.open("/mqtt_password.txt", "r");      // open for read
   if(!mqtt_password_file){
      Serial.println("No Saved Data in mqtt_password.txt!"); 
   }
   else{     
      file_mqtt_password    = mqtt_password_file.readStringUntil('\n');
   }   
   
   
   file_mqtt_host     = readFile(LittleFS, "/mqtt_host.txt");
   file_mqtt_port     = readFile(LittleFS, "/mqtt_port.txt");
   file_mqtt_user     = readFile(LittleFS, "/mqtt_user.txt");  
   file_mqtt_password = readFile(LittleFS, "/mqtt_password.txt");
   
   //
   if (debug) {
      printf("Content of file_mqtt_host = '%s'\n", file_mqtt_host.c_str());
      printf("Content of file_mqtt_port = '%s'\n", file_mqtt_port.c_str());
      printf("Content of file_mqtt_user = '%s'\n", file_mqtt_user.c_str());
      printf("Content of file_mqtt_password = '%s'\n", file_mqtt_password.c_str());
   }
   
   }
  

//---------------------------------------------------- 
// Read KostalPiko credentail files
//----------------------------------------------------
void read_kostalpiko_files(void) {
   //
   // read files with login credentials
   //
   file_kostalpiko_ip        = readFile(LittleFS, "/kostalpiko_ip.txt");
   file_kostalpiko_user      = readFile(LittleFS, "/kostalpiko_user.txt");
   file_kostalpiko_password  = readFile(LittleFS, "/kostalpiko_password.txt");
   //
   if (debug) {
      printf("Content of file_kostalpiko_ip = '%s'\n", file_kostalpiko_ip.c_str());
      printf("Content of file_kostalpiko_user = '%s'\n", file_kostalpiko_user.c_str());
      printf("Content of file_kostalpiko_password = '%s'\n", file_kostalpiko_password.c_str());
   }
}


//---------------------------------------------------- 
// Read OTA password
//----------------------------------------------------
void read_ota_password(void){
   
   file_ota_password  = readFile(LittleFS, "/ota_password.txt");
   
   if (debug) {
     printf("Content of file_ota_password = '%s'\n", file_ota_password.c_str());
   }
   
}   
  
//---------------------------------------------------- 
// Read all credential files
//----------------------------------------------------
void read_all_files(void){
  
  read_login_files();      
  read_wifi_files();       
  read_mqtt_files();       
  read_kostalpiko_files(); 
  read_ota_password();
  
}
  
//---------------------------------------------------- 
// Clear some txt files or all
//----------------------------------------------------
void clear_login_files(void){
  
   char empty[] = "";    // 0 zero's
   
   writeFile(LittleFS, "/login_user.txt", empty);
   writeFile(LittleFS, "/login_password.txt", empty);
}


//---------------------------------------------------- 
// Clear wifi files
//----------------------------------------------------  
void clear_wifi_files(void){   
  
   char empty[] = "";    // 0 zero's
   char empty1[] = "\r\n";             // give faster readback
   //
   writeFile(LittleFS, "/wifi_ssid.txt", empty);
   writeFile(LittleFS, "/wifi_password.txt", empty);
   writeFile(LittleFS, "/wifi_ip.txt", empty);
   writeFile(LittleFS, "/wifi_subnet.txt", empty1);
   writeFile(LittleFS, "/wifi_gateway.txt", empty1);
   writeFile(LittleFS, "/wifi_dns1.txt", empty1);
   writeFile(LittleFS, "/wifi_dns2.txt", empty1);
   writeFile(LittleFS, "/wifi_networkname.txt", empty1);
   //
   // restart kostal-piko reader
   restart = true;
}

//---------------------------------------------------- 
// Clear mqtt files
//----------------------------------------------------  
void clear_mqtt_files(void){   
  
   char empty[] = "";    // 0 zero's
   char empty1[] = "\r\n";             // give faster readback
   //
   writeFile(LittleFS, "/mqtt_host.txt", empty);
   writeFile(LittleFS, "/mqtt_port.txt", empty1);
   writeFile(LittleFS, "/mqtt_user.txt", empty1);
   writeFile(LittleFS, "/mqtt_password.txt", empty1);
   //
   // restart kostal-piko reader
   restart = true;
}

//---------------------------------------------------- 
// Clear kostalpiko files
//----------------------------------------------------  
void clear_kostalpiko_files(void){   
  
   char empty[] = "";    // 0 zero's
   char empty1[] = "\r\n";             // give faster readback
   //
   writeFile(LittleFS, "/kostalpiko_ip.txt", empty);
   writeFile(LittleFS, "/kostalpiko_user.txt", empty);
   writeFile(LittleFS, "/kostalpiko_password.txt", empty1);
   //
   // restart kostal-piko reader
   restart = true;
}

//---------------------------------------------------- 
// Clear ota password
//----------------------------------------------------  
void clear_ota_files(void){   
  
   char empty1[] = "\r\n";             // give faster readback
   //
   writeFile(LittleFS, "/ota_password.txt", empty1);
   //
   // restart kostal-piko reader
   restart = true;
}
  
  
//---------------------------------------------------- 
// Clear all credential files
//----------------------------------------------------   
void clear_all_files(void) {

   clear_login_files();       // will not restart the KostalPiko Reader
   clear_wifi_files();        // will     restart the KostalPiko Reader
   clear_mqtt_files();        // will     restart the KostalPiko Reader
   clear_kostalpiko_files();  // will     restart the KostalPiko reader
   clear_ota_files();  // will     restart the KostalPiko reader
}   
  