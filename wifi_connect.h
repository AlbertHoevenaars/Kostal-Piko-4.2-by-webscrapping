#ifndef _WIFI_H
#define _WIFI_H

/* 
** wifi_type = 0 if no connect with wifi
** wifi_type = 1 if STA with static IP
** wifi_type = 2 if STA with dynamic IP
** wifi_type = 3 if AP
*/

extern short int wifi_type; // 0-3
extern byte bssid[6];
extern String str_bssid;
extern String str_wifi_hostname;

// define functions
//
void setup_wifi(void);

#endif

