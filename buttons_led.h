#ifndef buttons_led_h
#define buttons_led_h


const bool True  = 1;
const bool False = 0;

const int ESP_frequency = 80000000;  // Hz
const int interrupt_frequency = 20;  //Hz
const int freq_divider = 16;         //fixed hardware number for ESP8266


const int ticks_between_irq = (ESP_frequency/freq_divider)/interrupt_frequency; // 25000 counts => 0.05 sec
const int time_between_irq = 50; // millisec

//extern bool Button_Active;

extern bool sec5_flag;

//extern bool bl_relay_button;    // shows that button is used to switch relay on or off: FIXED

//extern bool g_led_always_on; 
//extern bool g_led_always_off; 
extern int g_on_time; 
extern int g_off_time; 
extern int g_loopnumber;
extern bool g_led_irq_enable;


//-----------------------------------------------------------------------------
//Interupt service routine
//-----------------------------------------------------------------------------
void IRAM_ATTR timer1ISR();


//-----------------------------------------------------------------------------
//setup timers for interrupt
//-----------------------------------------------------------------------------
void setup_button_led_interrupt();

//-----------------------------------------------------------------------------
//define pins for button and led
//-----------------------------------------------------------------------------
void setup_button_led();

//-----------------------------------------------------------------------------
// Detect button press
//-----------------------------------------------------------------------------
void detect_button_press();

//-----------------------------------------------------------------------------
//Drive led 
//-----------------------------------------------------------------------------
void drive_led_no_irq(bool, bool, int, int, int); 

void drive_led_with_irq(); 


#endif
