#ifndef _MQTT_H
#define _MQTT_H
/* 
** Values for MQTT connection
*/
#include "kostal_piko.h"

//enum on_auto_off {on, on_auto, off_auto, off};


extern bool mqtt_ok;


// define functions
//
//void onWifiConnect(const WiFiEventStationModeGotIP&);
void connectToMqtt(void);
void onMqttConnect(bool);  
bool setup_mqtt(void);
void publish_mqtt(void);

#endif
