#ifndef _NTP_H
#define _NTP_H
/* 
** Values for Wifi connection
*/

extern char date_time[];
extern uint8_t  current_weekday; // 0 = Sunday
extern uint16_t current_year;
extern uint8_t  current_month;
extern uint8_t  current_mday;
extern uint8_t  current_hour;
extern uint8_t  current_minute;
extern uint8_t  current_second;


extern bool ntp_ok;

// define functions
//
bool init_ntp(void);
void get_ntptime(void); 

#endif
