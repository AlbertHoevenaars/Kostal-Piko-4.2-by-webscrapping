#ifndef _KOSTALPIKO_H
#define _KOSTALPIKO_H
/* 
// parameters for the kostal-piko reader
*/
extern const char compile_date[];
//
// Pinning; GPIO numbers 
//
// Outputs
const unsigned int led_pin       =  5;  // = pin  20
const unsigned int button_pin    = 16;  // = pin   4
//

const unsigned int loop_time = 60000;  // millisec -> 60 seconds

extern bool flash_ok;

#endif
