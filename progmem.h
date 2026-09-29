/*
** Define fixed string to be placed outside the RAM
*/

//
// Wifi messages
//
const char STA_staticIP[]                     PROGMEM = "Station mode with static IP";
const char STA_dynamicIP[]                    PROGMEM = "Station mode with dynamic IP";
const char APmode[]                           PROGMEM = "Access Point mode";
//
// for kostalpiko value webpage
//       
const char string_actual_power_w[]            PROGMEM = "ACTUAL_POWER";
const char string_total_energy_kwh[]          PROGMEM = "TOTAL_ENERGY";
const char string_daily_energy_kwh[]          PROGMEM = "DAILY_ENERGY";
const char string_string1_voltage_v[]         PROGMEM = "STRING1_VOLTAGE";
const char string_string1_current_a[]         PROGMEM = "STRING1_CURRENT";
const char string_string2_voltage_v[]         PROGMEM = "STRING2_VOLTAGE";
const char string_string2_current_a[]         PROGMEM = "STRING2_CURRENT";
const char string_string3_voltage_v[]         PROGMEM = "STRING3_VOLTAGE";
const char string_string3_current_a[]         PROGMEM = "STRING3_CURRENT";
const char string_L1_output_voltage_v[]       PROGMEM = "L1_VOLTAGE";
const char string_L1_output_power_w[]         PROGMEM = "L1_POWER";
const char string_L2_output_voltage_v[]       PROGMEM = "L2_VOLTAGE";
const char string_L2_output_power_w[]         PROGMEM = "L2_POWER";
const char string_L3_output_voltage_v[]       PROGMEM = "L3_VOLTAGE";
const char string_L3_output_power_w[]         PROGMEM = "L3_POWER";
const char string_status[]                    PROGMEM = "STATUS";

//                                            
//  LittleFS messages                         
//                                            
const char littlefs_error[]                   PROGMEM = "An Error has occurred while mounting LittleFS";
const char littlefs_success[]                 PROGMEM = "Success in mounting LittleFS";
const char littlefs_check[]                   PROGMEM = "Failed to open file index.html for reading";

//                                        
// main webpage strings                   
//                                        
const char index_html[]                       PROGMEM = "/index.html"; 
const char style_css[]                        PROGMEM = "/style.css";
const char favicon_png[]                      PROGMEM = "/favicon.png";
const char favicon_ico[]                      PROGMEM = "/favicon.ico";
const char data_time[]                        PROGMEM = "/datetime";
const char text_css[]                         PROGMEM = "text/css";
const char text_html[]                        PROGMEM = "text/html";
const char text_plain[]                       PROGMEM = "text/plain";
const char webpage_not_found[]                PROGMEM = "Webpage Not Found";
const char method_not_allowed[]               PROGMEM = "Method Not Allowed";

const char last_compile[]                     PROGMEM = "LAST_COMPILE";

  
//                                            
// for powermeter webpage                     
//                                            
const char webpage_kostalpiko_value[]         PROGMEM = "/kostalpiko_value.html";

//                                        
// memory webpage                         
//                                        
const char webpage_memory[]                   PROGMEM = "/memory.html";

//                                            
//                                            
// standby systemsetting webpage                      
//                                            
const char webpage_systemsettings[]           PROGMEM = "/systemsettings.html";
const char webpage_show_network_credentials[] PROGMEM = "/show_network_credentials.html";

const char toggle_login_password[]            PROGMEM = "/toggle_login_password";
const char toggle_wifi_password[]             PROGMEM = "/toggle_wifi_password";
const char toggle_mqtt_password[]             PROGMEM = "/toggle_mqtt_password";
const char toggle_kostalpiko_password[]       PROGMEM = "/toggle_kostalpiko_password";
const char toggle_ota_password[]              PROGMEM = "/toggle_ota_password";

//                                        
// KostalPiko MQTT Topics                
//                                        

const char connecting_to_mqtt[]              PROGMEM = "Subscribe acknowledged.";
const char unsubscribe_acknowledged[]        PROGMEM = "Unsubscribe acknowledged.";
const char packetid[]                        PROGMEM = "  packetId: ";
const char qos[]                             PROGMEM = "  qos: ";
const char publish_acknowledged[]            PROGMEM = "Publish acknowledged.";
//const char mqtt_connected[]                  PROGMEM = "mqtt connected";
//const char mqtt_not_connected[]              PROGMEM = "mqtt not connected";
//const char mqtt_reconnected[]                PROGMEM = "mqtt reconnected";
//                                           
// mqtt items (only used for printing)       
//                                           
const char mqtt_pub_voltage_v[]              PROGMEM = "kostalpiko/Voltage"; 
const char mqtt_pub_current_a[]              PROGMEM = "kostalpiko/Current";
const char mqtt_pub_active_power_w[]         PROGMEM = "kostalpiko/Active_Power";
const char mqtt_pub_energy_kwh[]             PROGMEM = "kostalpiko/Energy";
const char mqtt_pub_powerfactor[]            PROGMEM = "kostalpiko/PowerFactor"; 
const char mqtt_pub_energy_kva[]             PROGMEM = "kostalpiko/Apparent_Power";
const char mqtt_pub_reactiveenergy_kvar[]    PROGMEM = "kostalpiko/Reactive_Power";
const char mqtt_pub_linefreq_hz[]            PROGMEM = "kostalpiko/LineFreq";
  
//                                           
// ntp                                       
//                                           
const char adjust_local_clock[]              PROGMEM = "Adjust local clock"; 
const char ntp_update_not_work[]             PROGMEM = "NTP Update not WORKING!!";
//
// 
//
const char webpage_wifisettings[]            PROGMEM = "/wifimanager.html"; 
const char webpage_wifimanager_update[]      PROGMEM = "/wifimanger_update";
//
// Logout 
//
const char webpage_logout[]                  PROGMEM = "/logout.html";
//
// Login user and password
//
const char webpage_signin_manager[]          PROGMEM = "/signin_manager.html";
const char wm_login_user[]                   PROGMEM = "LOGIN_USER";
const char filename_login_user[]             PROGMEM = "/login_user.txt";
const char wm_login_password[]               PROGMEM = "LOGIN_PASSWORD";
const char filename_login_password[]         PROGMEM = "/login_password.txt";


//
// Values from WifiManager webpage identified by:
//
const char wm_wifi_ssid[]                    PROGMEM = "WIFI_SSID";
const char filename_wifi_ssid[]              PROGMEM = "/wifi_ssid.txt";
//const char wm_wifi_ssid_message[]            PROGMEM = "name of your wifi network";
//const char filename_ssid_message
const char wm_wifi_password[]                PROGMEM = "WIFI_PASSWORD";
const char filename_wifi_password[]          PROGMEM = "/wifi_password.txt";
const char wm_wifi_ip[]                      PROGMEM = "WIFI_IP";
const char filename_wifi_ip[]                PROGMEM = "/wifi_ip.txt";
const char wm_wifi_gateway[]                 PROGMEM = "WIFI_GATEWAY";
const char filename_wifi_gateway[]           PROGMEM = "/wifi_gateway.txt";
const char wm_wifi_subnet[]                  PROGMEM = "WIFI_SUBNET";
const char filename_wifi_subnet[]            PROGMEM = "/wifi_subnet.txt";
const char wm_wifi_dns1[]                    PROGMEM = "WIFI_DNS1";
const char filename_wifi_dns1[]              PROGMEM = "/wifi_dns1.txt";
const char wm_wifi_dns2[]                    PROGMEM = "WIFI_DNS2";
const char filename_wifi_dns2[]              PROGMEM = "/wifi_dns2.txt";
const char wm_wifi_networkname[]             PROGMEM = "WIFI_NETWORKNAME";
const char filename_wifi_networkname[]       PROGMEM = "/wifi_networkname.txt";
const char wm_wifi_mac[]                     PROGMEM = "WIFI_MAC";
//
const char wm_wifi_type[]                    PROGMEM = "WIFI_TYPE";
//
// Values from MQTTManager webpage identified by:
//
const char webpage_mqttsettings[]            PROGMEM = "/mqttmanager.html"; 
const char mqttmanager_update[]              PROGMEM = "/mqttmanger_update";
//
const char mqtt_connected[]                  PROGMEM = "MQTT_CONNECTED";
const char wm_mqtt_host[]                    PROGMEM = "MQTT_HOST";
const char filename_mqtt_host[]              PROGMEM = "/mqtt_host.txt";
const char wm_mqtt_port[]                    PROGMEM = "MQTT_PORT";
const char filename_mqtt_port[]              PROGMEM = "/mqtt_port.txt";
const char wm_mqtt_user[]                    PROGMEM = "MQTT_USER";
const char filename_mqtt_user[]              PROGMEM = "/mqtt_user.txt";
const char wm_mqtt_password[]                PROGMEM = "MQTT_PASSWORD";
const char filename_mqtt_password[]          PROGMEM = "/mqtt_password.txt";
//
// values for KostalPiko credentials
//
const char webpage_kostalpikosettings[]      PROGMEM = "/kostalpikomanager.html"; 
const char kostalpikomanager_update[]        PROGMEM = "/kostalpikomanger_update";
//
const char wm_kostalpiko_ip[]                PROGMEM = "KOSTALPIKO_IP";
const char filename_kostalpiko_ip[]          PROGMEM = "/kostalpiko_ip.txt";
const char wm_kostalpiko_user[]              PROGMEM = "KOSTALPIKO_USER";
const char filename_kostalpiko_user[]        PROGMEM = "/kostalpiko_user.txt";
const char wm_kostalpiko_password[]          PROGMEM = "KOSTALPIKO_PASSWORD";
const char filename_kostalpiko_password[]    PROGMEM = "/kostalpiko_password.txt";
const char kostalpiko_connected[]            PROGMEM = "KOSTALPIKO_CONNECTED";

//
// values for OTA credentials
//
const char webpage_otasettings[]            PROGMEM = "/otamanager.html"; 
const char otamanager_update[]              PROGMEM = "/otamanger_update";
//
const char wm_ota_password[]                 PROGMEM = "OTA_PASSWORD"; 
const char filename_ota_password[]           PROGMEM = "/ota_password.txt";
//
// Values for reset 
//
const char webpage_reset[]                   PROGMEM = "/reset.html";

//
// read csv files
//
const char text_csv[]                         PROGMEM = "text/csv";
const char invalid_csv_index[]                PROGMEM = "Invalid CSV index";
const char data_block_not_in_use[]            PROGMEM = "Data block not in use";
const char invalid_date_in_header[]           PROGMEM = "Invalid date in header";


