#include <Arduino.h>

#include "buttons_led.h"
#include "kostal_piko.h"
//#include "progmem.h"  


bool led_on  = false;
bool led_off = false;


bool current_button = false;
bool button_active = false;

bool prev_button = false;
uint8_t active_button_counter;

bool sec5_flag  = false;


const int analogInPin = A0;

//bool bl_relay_button;

//bool g_led_always_on = false; 
//bool g_led_always_off = false; 
int g_on_time = 0; 
int g_off_time = 0; 
int g_loopnumber = 0;
bool g_led_irq_enable = false;

// Define states
//
enum state {st_led_on, decr_on_cnt, st_led_off, decr_off_cnt, decr_loop_cnt};
state current_state = st_led_on;
int loop_counter = 0;

//-----------------------------------------------------------------------------
// Interrupt program for timer
// If button is not pressed all value are reset
// When button is pressed "active_button_counter" start counting until max value.
// If button is released "active_button_counter" is copied to "button_counter".
//    All other values are reset   
// Main program takes care of "button_counter".
//-----------------------------------------------------------------------------
//
void IRAM_ATTR timer1ISR() {
   
   //-------------------------------------------------------------
   // Detect button press for 5 seconds
   //-------------------------------------------------------------
   //detect_button_press();
   int ext_button = digitalRead(button_pin);
   //
   //
   if (ext_button == 1) {                   // is button pressed
      current_button = true;                // convert to boolean
   }   
   else {   
      current_button = false; 
   }  
   //
   // see if button is changed
   //   
   if ((not current_button) && (not prev_button)) {      // button not pressed (for long time)
      NULL;
   }
   else if (current_button && (not prev_button)) {       // first keypress
      active_button_counter = 0;                         // reset counter
 //     sec5_flag  = false;
   } 
   else if (current_button && prev_button){              // button pressed for long time, increment counter
      printf("*");
      if (active_button_counter != 255){
          active_button_counter = active_button_counter + 1;
      } 
   }      
   else  { 
      printf("^\n");
      // button released
      //
      // 1 IRQ every 0.05 sec
      // 4.5 sec press =  90 pulses;  5 sec press = 100 pulses;  5.5 sec press = 110 pulses
      //
      // 5 seconds means full erase the KostalPiko Readerr.
      if (active_button_counter >= 90){
         sec5_flag = true;      
         printf("5\n");
      // 5 seconds means complete reset
      active_button_counter = 0;                        // reset counter
      }

   }   

   //
   prev_button = current_button;
   if ((active_button_counter == 20) || (active_button_counter == 40)|| (active_button_counter == 60)|| (active_button_counter == 80)
        || (active_button_counter == 100)) {
      g_on_time = 10;       // 10 counts of 0.05 seconds;
      g_off_time = 0;
      loop_counter = 0;
      g_led_irq_enable = true;
   }   

   
   //
   //-------------------------------------------------------------
   // Drive led in TRQ steps
   //-------------------------------------------------------------
   //
   
   int led_counter; 

   if (g_led_irq_enable) {
      switch (current_state) {
         case st_led_on:
            digitalWrite(led_pin, LOW);          // turn the LED on
            led_counter = g_on_time;
            current_state = decr_on_cnt;
            break;
         //   
         case decr_on_cnt:
            if (led_counter == 0){
               current_state = st_led_off;
               break;
            }
            else {
               led_counter = led_counter - 1;    // stay in same state
               break;
            }
         //            
         case st_led_off:
            digitalWrite(led_pin, HIGH);         // turn the LED off
            led_counter = g_off_time;            // number of irq ticks
            current_state = decr_off_cnt;
            break;
         //   
         case decr_off_cnt:
            if (led_counter == 0){
               current_state = decr_loop_cnt;
               break;
            }
            else {
               led_counter = led_counter - 1;    // stay in same state
               break;
            }
         //   
         case decr_loop_cnt:
            if (loop_counter == 0){
               current_state = st_led_on;
               g_led_irq_enable = false;         // led function done
               break;
            }
            else {
               loop_counter = loop_counter - 1;    // stay in same state
               current_state = st_led_on;
               break;
            }
            
      }
   }
 
   
}

//-----------------------------------------------------------------------------
// Make settings for timer and interrupt
//-----------------------------------------------------------------------------
void setup_button_led_interrupt(){
   
   timer1_attachInterrupt(timer1ISR); 
   
    /* Prescalers:
        TIM_DIV1      80MHz => 80 ticks/µs => Max: (2^23 / 80) µs =  ~0.105s
        TIM_DIV16     5MHz  => 5 ticks/µs => Max: (2^23 / 5) µs = ~1.678s
        TIM_DIV256    0.3125MHz => 0.3125 ticks/µs => Max: (2^23 / 0,3125) µs = ~26.8s
      Interrupt TYPE:
        TIM_EDGE      no other choice here
      Repeat?:
        TIM_SINGLE  0 => one time interrupt, you need another timer1_write(ticks); to restart
        TIM_LOOP    1 => regular interrupt 
    */
   timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
   //
   // (ESP_frequency/DIV16)/interrupt_frequency
   //
   timer1_write(ticks_between_irq); // 25.000 ticks => 0.05s interval 

}
   
//-----------------------------------------------------------------------------
// Drive led On for 1 sec durig setup of KostalPiko Reader
//-----------------------------------------------------------------------------
void setup_button_led(){
   
   pinMode(button_pin, INPUT);                   // button as input
   
   pinMode(led_pin, OUTPUT);                     // led as output
   digitalWrite(led_pin, LOW);                   // turn the LED on
   delay(1000);
   digitalWrite(led_pin, HIGH);                   // turn the LED off
   
}   


//-----------------------------------------------------------------------------
// Drive the led depending on the first 2 booleans
// When loopnumber = -1, it loops forever with on and off times
// Any other loopnumber starts a loop
//-----------------------------------------------------------------------------
void drive_led_no_irq(bool led_on, bool led_off, int on_time, int off_time, int loopnumber) {

   if (led_on) {
      digitalWrite(led_pin, LOW);                // turn the LED on
   }
   else if (led_off) {  
      digitalWrite(led_pin, HIGH);               // turn the LED off
   }
   else   {
      if (loopnumber == -1) {
         while (true){                           // loop forever
            digitalWrite(led_pin, LOW);          // turn the LED on
            delay(on_time);                      // wait for on_time
            digitalWrite(led_pin, HIGH);         // turn the LED off
            delay(off_time);                     // wait for off_time
         } 
      }         
      else {
         for (int i = 0; i < loopnumber; i++) {         
            digitalWrite(led_pin, LOW);          // turn the LED on
            delay(on_time);                      // wait for on_time
            digitalWrite(led_pin, HIGH);         // turn the LED off
            delay(off_time);                     // wait for off_time
         }   
      }
   }
}


