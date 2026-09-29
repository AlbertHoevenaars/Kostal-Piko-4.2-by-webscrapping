/*
** Make a connection to the MQTT server.
** This connection will be used for sending and reading values to/from MQTT
** 
*/

#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>
//#include <HTTPClient.h>

#include "mqtt.h"
#include "kostal_piko.h"
#include "progmem.h"
#include "littlefs_rw.h"
#include "wifi_connect.h"
#include "webserver.h"
#include "webscraper.h"

bool mqtt_ok = false;

const char* MQTT_ID    = "KostalPiko";    // unique client ID
// --- MQTT topic base — all values published under this prefix ---
// e.g. solar/piko/ac/power, solar/piko/string1/voltage, etc.
const char* TOPIC_BASE = "solar/piko";

// KostalPiko MQTT Topics

const char *MQTT_PUB_Current_power          = "/sensor/Current_Power/state"; 
const char *MQTT_PUB_Total_energy           = "/sensor/Total_Energy/state";
const char *MQTT_PUB_Daily_energy           = "/sensor/Daily_Energy/state";
const char *MQTT_PUB_String1_voltage        = "/sensor/String1_Voltage/state";
const char *MQTT_PUB_String1_current        = "/sensor/String1_Current/state"; 
const char *MQTT_PUB_String2_voltage        = "/sensor/String2_Voltage/state";
const char *MQTT_PUB_String2_current        = "/sensor/String2_Current/state"; 
const char *MQTT_PUB_String3_voltage        = "/sensor/String3_Voltage/state";
const char *MQTT_PUB_String3_current        = "/sensor/String3_Current/state"; 
const char *MQTT_PUB_L1_output_voltage      = "/sensor/L1_output_voltage/state";
const char *MQTT_PUB_L1_output_power        = "/sensor/L1_output_power/state";
const char *MQTT_PUB_L2_output_voltage      = "/sensor/L2_output_voltage/state";
const char *MQTT_PUB_L2_output_power        = "/sensor/L2_output_power/state";
const char *MQTT_PUB_L3_output_voltage      = "/sensor/L3_output_voltage/state";
const char *MQTT_PUB_L3_output_power        = "/sensor/L3_output_power/state";
const char *MQTT_PUB_status                 = "/sensor/status/state";
const char *MQTT_PUB_kostalpiko_code        = "/sensor/kostalpiko_code/state";

const char *MQTT_PUB_base_total_energy      = "/sensor/base_Total_Energy/state";
const char *MQTT_PUB_out_total_energy       = "/sensor/out_Total_Energy/state";
const char *MQTT_PUB_out_daily_energy       = "/sensor/out_Daily_Energy/state";


AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;


//WiFiEventHandler wifiConnectHandler;
//WiFiEventHandler wifiDisconnectHandler;
//Ticker wifiReconnectTimer;


void connectToMqtt() {
   printf("Connecting to MQTT...\n");
   mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {
   printf("Connected to MQTT.");
   printf("Session present: %d\n", sessionPresent);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
   printf("Disconnected from MQTT.\n");

   if (WiFi.isConnected()) {
      mqttReconnectTimer.once(2, connectToMqtt);
   }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
   printf("Subscribe acknowledged.\n");
   printf("  packetId: %d\n", packetId);
   printf("  qos: %d\n", qos);

}

void onMqttUnsubscribe(uint16_t packetId) {
   printf("Unsubscribe acknowledged.\n");
   printf("  packetId: %d\n", packetId);
}

/*
// --- MQTT reconnect (non-blocking retry) ---
bool mqttReconnect() {
    if (mqttClient.connected()) return true;

    Serial.print("MQTT connecting...");
    bool ok = (strlen(file_mqtt_user.c_str()) > 0)
        ? mqttClient.connect(MQTT_ID, file_mqtt_user.c_str(), file_mqtt_password.c_str())
        : mqttClient.connect(MQTT_ID);

    if (ok) {
        Serial.println("connected");
        // Publish online status
        char topic[80];
        snprintf(topic, sizeof(topic), "%s/status", TOPIC_BASE);
        mqttClient.publish(topic, "online", true);
    } else {
        Serial.printf("failed (rc=%d), retry in 5s\n", mqtt.state());
        delay(5000);
    }
    return ok;
}
*/

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
   printf("Publish received.\n");
   printf("  topic: ", topic);
   printf("  qos: ");
   Serial.println(properties.qos);
   printf("  dup: ");
   Serial.println(properties.dup);
   printf("  retain: ");
   Serial.println(properties.retain);
   printf("  len: ");
   Serial.println(len);
   printf("  index: ");
   Serial.println(index);
   printf("  total: ");
   Serial.println(total);
}

void onMqttPublish(uint16_t packetId) {
  //printf("Publish acknowledged.\n");
  //printf("  packetId: %d\n", packetId);

}

bool setup_mqtt() {

   // Credentials must be defined before the OnConnect
   // If your broker requires authentication (username and password), set them below
   mqttClient.setCredentials(file_mqtt_user.c_str(), file_mqtt_password.c_str());
   //
   mqttClient.onConnect(onMqttConnect);             // set callback functions
   mqttClient.onDisconnect(onMqttDisconnect);
   mqttClient.onSubscribe(onMqttSubscribe);
   mqttClient.onUnsubscribe(onMqttUnsubscribe);
   mqttClient.onMessage(onMqttMessage);
   mqttClient.onPublish(onMqttPublish);
   //
   mqttClient.setKeepAlive(120);
   //    
   IPAddress ip;
   ip.fromString(file_mqtt_host.c_str());
   
   mqttClient.setServer(ip, atoi(file_mqtt_port.c_str()));
   
   int mqtt_count = 0;
   Serial.println();
   Serial.print("MQTT Connecting.\n");
   
   // Loop until we're reconnected
   while (!mqttClient.connected() && (mqtt_count < 15)) {
      printf(".");
      delay (1500);                         // 1.5 second delay
      mqttClient.connect();                 // try to make a connection
      mqtt_count++;                         // 10 loops max
   }   
   printf("\n");
   
   if (mqtt_count == 15){                     // no success 
//     Serial.print(mqttClient.state());
     mqtt_ok= false;
   }
   else {                                      // success
      if (mqttClient.connected()) {
         printf("Connected to MQTT\n");
         // Subscribe
//         mqttClient.subscribe("kostalpiko");
         mqtt_ok = true;
      } 
      else {
         mqtt_ok= false;
      }
   }
   return mqtt_ok;
}  

//
// Publish some electrical values to MQTT
//
void publish_mqtt() {
        
    if (mqttClient.connected()){
    //  printf("mqtt connected.\n"); 
    }
    else {
      printf("mqtt not connected.\n");
      mqttClient.connect();
      printf("mqtt reconnected\n");
    }

   if (mqttClient.connected()){  // only if reconnected
 
      uint16_t packetIdPub16 = mqttClient.publish((str_wifi_hostname + MQTT_PUB_status).c_str(), 1, true, char_status);                            
   //  printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_LineFreq, packetIdPub2);
   //  printf("Message: %.2f \n", hum);
   
      uint16_t packetIdPub17 = mqttClient.publish((str_wifi_hostname + MQTT_PUB_kostalpiko_code).c_str(), 1, true, char_kostalpiko_code);                            
   //  printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_LineFreq, packetIdPub2);
   //  printf("Message: %.2f \n", hum);
   
      uint16_t packetIdPub1 = mqttClient.publish((str_wifi_hostname + MQTT_PUB_Current_power).c_str(), 1, true, char_current_power); 
//     printf("Publishing on topic %s at QoS 1, packetId: %i \n", FPSTR(string_actual_power_w), packetIdPub1);
     //printf("Message: %.2f \n", temp);
  
      uint16_t packetIdPub2 = mqttClient.publish((str_wifi_hostname + MQTT_PUB_Total_energy).c_str(), 1, true, char_total_energy);                            
   //  printf("Publishing on topic %s at QoS 1, packetId %i: ", FPSTR(string_total_energy_kwh), packetIdPub2);
   //  printf("Message: %.2f \n", hum);
  
      uint16_t packetIdPub3 = mqttClient.publish((str_wifi_hostname + MQTT_PUB_Daily_energy).c_str(), 1, true, char_out_daily_energy);                            
   //  printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_Active_Power_W, packetIdPub2);
   //  printf("Message: %.2f \n", hum);
  
      uint16_t packetIdPub4 = mqttClient.publish((str_wifi_hostname + MQTT_PUB_String1_voltage).c_str(), 1, true, char_string1_voltage);                            
   //  printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_Energy_kWh, packetIdPub2);
   //  printf("Message: %.2f \n", hum);
  
      uint16_t packetIdPub5 = mqttClient.publish((str_wifi_hostname + MQTT_PUB_String1_current).c_str(), 1, true, char_string1_current);                            
   //  printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_Powerfactor, packetIdPub2);
   //  printf("Message: %.2f \n", hum);
  
      uint16_t packetIdPub6 = mqttClient.publish((str_wifi_hostname + MQTT_PUB_String2_voltage).c_str(), 1, true, char_string2_voltage);                            
   //  printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_Energy_kWh, packetIdPub2);
   //  printf("Message: %.2f \n", hum);
  
      uint16_t packetIdPub7 = mqttClient.publish((str_wifi_hostname + MQTT_PUB_String2_current).c_str(), 1, true, char_string2_current);                            
   //  printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_Powerfactor, packetIdPub2);
   //  printf("Message: %.2f \n", hum);

      uint16_t packetIdPub8 = mqttClient.publish((str_wifi_hostname + MQTT_PUB_String3_voltage).c_str(), 1, true, char_string3_voltage);                            
   //  printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_Energy_kWh, packetIdPub2);
   //  printf("Message: %.2f \n", hum);
  
      uint16_t packetIdPub9 = mqttClient.publish((str_wifi_hostname + MQTT_PUB_String3_current).c_str(), 1, true, char_string3_current);                            
   //  printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_Powerfactor, packetIdPub2);
   //  printf("Message: %.2f \n", hum);

      uint16_t packetIdPub10 = mqttClient.publish((str_wifi_hostname + MQTT_PUB_L1_output_voltage).c_str(), 1, true, char_L1_output_voltage);                            
   //  printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_Energy_kVA, packetIdPub2);
   //  printf("Message: %.2f \n", hum);
  
      uint16_t packetIdPub11 = mqttClient.publish((str_wifi_hostname + MQTT_PUB_L1_output_power).c_str(), 1, true, char_L1_output_power);  
     //  printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_ReactiveEnergy_kVAR, packetIdPub2);
     //  printf("Message: %.2f \n", hum);
  
      uint16_t packetIdPub12 = mqttClient.publish((str_wifi_hostname + MQTT_PUB_L2_output_voltage).c_str(), 1, true, char_L2_output_voltage);                            
   //  printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_Energy_kVA, packetIdPub2);
   //  printf("Message: %.2f \n", hum);
  
      uint16_t packetIdPub13 = mqttClient.publish((str_wifi_hostname + MQTT_PUB_L2_output_power).c_str(), 1, true, char_L2_output_power);  
     //  printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_ReactiveEnergy_kVAR, packetIdPub2);
     //  printf("Message: %.2f \n", hum);
  
      uint16_t packetIdPub14 = mqttClient.publish((str_wifi_hostname + MQTT_PUB_L3_output_voltage).c_str(), 1, true, char_L3_output_voltage);                            
   //  printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_Energy_kVA, packetIdPub2);
   //  printf("Message: %.2f \n", hum);
  
      uint16_t packetIdPub15 = mqttClient.publish((str_wifi_hostname + MQTT_PUB_L3_output_power).c_str(), 1, true, char_L3_output_power);  
     //  printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_ReactiveEnergy_kVAR, packetIdPub2);
     //  printf("Message: %.2f \n", hum);

//      uint16_t packetIdPub18 = mqttClient.publish((str_wifi_hostname + MQTT_PUB_base_total_energy).c_str(), 1, true, char_base_total_energy);  
     //  printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_ReactiveEnergy_kVAR, packetIdPub2);
     //  printf("Message: %.2f \n", hum);
  
//      uint16_t packetIdPub19 = mqttClient.publish((str_wifi_hostname + MQTT_PUB_out_total_energy).c_str(), 1, true, char_out_total_energy);                            
   //  printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_Energy_kVA, packetIdPub2);
   //  printf("Message: %.2f \n", hum);
  
//      uint16_t packetIdPub20 = mqttClient.publish((str_wifi_hostname + MQTT_PUB_out_daily_energy).c_str(), 1, true, char_out_daily_energy);  
     //  printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_ReactiveEnergy_kVAR, packetIdPub2);
     //  printf("Message: %.2f \n", hum);

   }
}
