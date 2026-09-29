#ifndef _LITTLEFS_H
#define _LITTLEFS_H

#include <FS.h>

// Variables to store login credentials
extern String file_login_user;
extern String file_login_password;

//Variables to save wifi values from HTML form into following variable names
extern String file_wifi_ssid;
extern String file_wifi_password;
extern String file_wifi_ip;
extern String file_wifi_gateway;
extern String file_wifi_subnet;
extern String file_wifi_dns1;
extern String file_wifi_dns2;
extern String file_wifi_networkname;

// 
//Variables to save mqtt values from HTML form into following variable names
//
extern String file_mqtt_host;
extern String file_mqtt_port;
extern String file_mqtt_user;
extern String file_mqtt_password;


//-----------------------------------------------
// Kostal Piko access
//-----------------------------------------------
extern String file_kostalpiko_ip;
extern String file_kostalpiko_user;
extern String file_kostalpiko_password;

//-----------------------------------------------
// OTA access
//-----------------------------------------------
extern String file_ota_password;

extern bool littlefs_ok;

/*
//  fs::FS &fs: This is a reference to a file system object (e.g., SPIFFS, LittleFS, or any other compatible file system).
// const char * path: This represents the path to the file you want to read from the file system, 
//  given as a string (in C-style const char*).
*/


String readFile(fs::FS &fs, const char * path);                        // read File from LittleFS

void writeFile(fs::FS &fs, const char * path, const char * message);   // write file to LittleFS

void check_littlefs(void);                                             // check if littlefs exists and file exists 

void write_kostalpiko_files(void);                                     // write all files for kostalpiko


void read_login_files(void);                                           // read all files for wifi and mqtt connection
void read_wifi_files(void);                                            // read all files for wifi and mqtt connection
void read_mqtt_files(void);                                            // read all files for wifi and mqtt connection
void read_kostalpiko_files(void);                                      // read all files for kostalpiko
void read_ota_password(void);                                          // read ota password
void read_all_files(void);                                             // read all credential files

// clear
void clear_login_files(void);
void clear_wifi_files(void);
void clear_mqtt_files(void);
void clear_kostalpiko_files(void);
void clear_ota_files(void);
void clear_all_files(void);

#endif