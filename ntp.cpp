/*
** Make a connection to the Wifi network.
** This connection will be used for MQTT and for a webpage to show numbers
** and make settings
*/

// change next line to use with another board/shield
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <time.h>
#include <Timezone.h>
#include <string.h>
#include <stdlib.h>

#include "ntp.h"
#include "progmem.h"

const char* ntpServer = "europe.pool.ntp.org";
unsigned long epoch;
const long  gmtOffset_sec = 0;
//const int   daylightOffset_sec = 3600;
//
uint8_t  current_weekday;  // 0 = Sunday
uint16_t current_year;
uint8_t  current_month;
uint8_t  current_mday;
uint8_t  current_hour;
uint8_t  current_minute;
uint8_t  current_second;

bool ntp_ok = false;    //signals if ntp connection is OK

char date_time[35];
char char_zero[]= "0";


//
// Input time in epoch format and return tm time format
// by Renzo Mischianti <www.mischianti.org> 
//
static tm getDateTimeByParams(long time){
  
   struct tm *newtime;
   const time_t tim = time;
  
   newtime = localtime(&tim);
   return *newtime;
}
 
/**
 * Input tm time format and return String with format pattern
 * by Renzo Mischianti <www.mischianti.org>
 */
static String getDateTimeStringByParams(tm *newtime, char* pattern = (char *)"%Y/%m/%d %H:%M:%S"){
    
   char buffer[30];
   
   strftime(buffer, 30, pattern, newtime);
   return buffer;
}

/**
 * Input time in epoch format format and return String with format pattern
 * by Renzo Mischianti <www.mischianti.org> 
 */
static String getEpochStringByParams(long time, char* pattern = (char *)"%Y/%m/%d %H:%M:%S"){
//    struct tm *newtime;
   tm newtime;
   newtime = getDateTimeByParams(time);
   return getDateTimeStringByParams(&newtime, pattern);
}
 
WiFiUDP ntpUDP;

//NTPClient timeClient(ntpUDP);

// By default 'pool.ntp.org' is used with 60 seconds update interval and
// no offset
// NTPClient timeClient(ntpUDP);
 
// You can specify the time server pool and the offset, (in seconds)
// additionaly you can specify the update interval (in milliseconds).
int8_t GTMOffset = 0; // Set to UTC time
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", GTMOffset*3600, 60*60*1000);

// Central European Time (Frankfurt, Paris)
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     // Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       // Central European Standard Time
Timezone CE(CEST, CET);

//-------------------------------------------------------------------
bool init_ntp(){
  
   // Initialize a NTPClient to get time
   timeClient.begin();
   delay (1000);
   if (timeClient.update()){
//      printf("%s\n",FPSTR(adjust_local_clock) );
      epoch = timeClient.getEpochTime();
//      epoch = CE.toLocal(timeClient.getEpochTime());
      // HERE I'M UPDATE LOCAL CLOCK
      setTime(epoch);
      return true;
   }
   else{
      return false;                                              // no update
   }
} 

//-------------------------------------------------------------------
void get_ntptime(){
  
   char char_current_year[5];
   char char_current_month[3];
   char char_current_mday[3];
   char char_current_hour[3];   // 2 numbers + \0
   char char_current_minute[3];
   char char_current_second[3];
  
   //
   timeClient.update();
 
   // Get local epoch from the timezone-corrected system clock
   time_t localEpoch = CE.toLocal(now());
   tm localTime = getDateTimeByParams(localEpoch);

   current_weekday = localTime.tm_wday;
   current_year    = localTime.tm_year + 1900;
   current_month   = localTime.tm_mon + 1;
   current_mday    = localTime.tm_mday;
   current_hour    = localTime.tm_hour;
   current_minute  = localTime.tm_min;
   current_second  = localTime.tm_sec;
  //
//  printf("current_hour = %d\n", current_hour);
   itoa(current_hour, char_current_hour, 10); //(integer, yourBuffer, base)
//  printf("char_current_hour = %s\n", char_current_hour);

//  current_minute = timeClient.getMinutes();
   itoa(current_minute, char_current_minute, 10); //(integer, yourBuffer, base)
//  printf("char_current_minute = %s\n", char_current_minute);

//  current_second = timeClient.getSeconds();
//  printf("--current_second = %d\n", current_second);
   itoa(current_second, char_current_second, 10); //(integer, yourBuffer, base)
//  printf("++char_current_second = %s\n", char_current_second);

//  current_year = year();
   itoa(current_year, char_current_year, 10);
//  printf("current_year = %d\n", current_year);
  //
//  current_month = month();
   itoa(current_month, char_current_month, 10);
//  printf("month = %d\n", current_month);
  //
//  current_mday = day();
   itoa(current_mday, char_current_mday, 10);
//  printf("current_month_day = %d\n", current_mday);
  //  
//  printf("time = %d:%d\n",current_hour, current_minute);

   strcpy(date_time, "Date: ");
   strcat(date_time, char_current_year);
   strcat(date_time, "/");
   //
   if (strlen(char_current_month) == 1) {
      strcat(date_time, "0");
   }    
   strcat(date_time, char_current_month);
   strcat(date_time, "/");
   //
   if (strlen(char_current_mday) == 1) {
      strcat(date_time, "0");
   }   
   strcat(date_time, char_current_mday);
   strcat(date_time, "   ");
   //
   strcat(date_time, "Time: ");
   strcat(date_time, char_current_hour);
//   printf("date_time1 = %s\n", date_time);
   strcat(date_time, ":");
//   printf("date_time2 = %s\n", date_time);
   if (strlen(char_current_minute) == 1) {
      strcat(date_time, "0");
   }    
   strcat(date_time, char_current_minute);
//   printf("date_time3 = %s\n", date_time);
   strcat(date_time, ":");
//   printf("date_time4 = %s\n", date_time);
   if (strlen(char_current_second) == 1) {
      strcat(date_time, "0");
   }      
   strcat(date_time, char_current_second);
//   printf("date_time5 = %s\n", date_time); 

}
