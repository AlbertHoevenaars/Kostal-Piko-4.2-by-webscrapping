#ifndef _WEBSCRAPER_H
#define _WEBSCRAPER_H

// values extracted from html page
typedef struct PikoData_struct {
    uint16_t currentpower;
    float    totalEnergy;
    float    dailyEnergy;
    String   status;
    uint16_t str1Voltage;
    float    str1Current;
    uint16_t str2Voltage;
    float    str2Current;
    uint16_t str3Voltage;
    float    str3Current;
    uint16_t l1Voltage; 
    uint16_t l1Power;
    uint16_t l2Voltage;
    uint16_t l2Power;
    uint16_t l3Voltage;
    uint16_t l3Power;
    String kostalpiko_code;
    float    base_totalEnergy;
    float    out_totalEnergy;
    float    out_dailyEnergy;
} PikoData;

extern PikoData data;

extern int kostalpiko_code;  // html code from KostalPiko device

const int HTTP_GET_CODE_OK = 200;

//extern struct PikoData_struct PikoData; // declaration

extern char char_current_power[8];
extern char char_total_energy[10];
extern char char_daily_energy[10];

extern char char_string1_voltage[6];
extern char char_string1_current[8];
extern char char_string2_voltage[6];
extern char char_string2_current[8];
extern char char_string3_voltage[6];
extern char char_string3_current[8];

extern char char_L1_output_voltage[6];
extern char char_L1_output_power[6];
extern char char_L2_output_voltage[6];
extern char char_L2_output_power[6];
extern char char_L3_output_voltage[6];
extern char char_L3_output_power[6];

extern char char_status[15];
extern char char_kostalpiko_code[10];

extern char char_base_total_energy[10];
extern char char_out_total_energy[10] ;
extern char char_out_daily_energy[10] ;



//----------------------------------------------------
// Functions
//----------------------------------------------------

void setup_webscraper();
void fetch_kostalpiko_webpage();
void check_midnight();
String extractBetween(const String& , const String& , const String& );
void parseHTML(const String& ); 
void printData();

#endif