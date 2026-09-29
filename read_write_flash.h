#ifndef read_write_flash_h
#define read_write_flash_h

// all memory definitions in bytes; constant values
// A W25Q128 device is used. Can hold 16Mbyte
//
const int FLASH_SIZE_bytes = 128*1024*1024/8; // 16 Mbyte


const int datagroup_size       =  2 * 64 * 1024;           // size of one block of data, holds storage for one day; Four groups of 64kByte
#define number_of_datagroups 128

extern const int datagroup_number;                      // number of data groups
const int flash_size           = number_of_datagroups * datagroup_size;   // memory size reserved for data storage
const int dataset_numbers      = 24 * 60;                  // 24 hours every minute 
const int dataset_size         = 64;

// For CSV and JSON file
static constexpr int NUMBER_OF_CHUNKES = 120;
static constexpr int SAMPLES_PER_DAY = 24 * 60; 
static constexpr int CHUNK_SIZE = SAMPLES_PER_DAY / NUMBER_OF_CHUNKES;

#define FLASH_ERASED_FLAG 0xFF
#define FLASH_USED_FLAG   0xAA


//const int electrical_value_size = 32;                    // size of electrical values stored every 15 seconds

extern bool     datagroup_today_flag;                      // if active datagroup is found
extern uint8_t  active_datagroup_number;                          // datagroup for today
extern uint32_t active_datagroup_address;                  // points in a datablock for storing electrical values
//
//
// definition of electrical sample, written in datablock
// Just 32 bytes which will prevent page crossing
//
struct write_electrical_sample_to {
   uint16_t hour_minute;
   uint16_t current_power;                                            // write current power
   float    total_energy;                                             // write total energy
   float    daily_energy;                                             // write daily energy
   uint16_t status;                                                   // write status flags
   uint16_t string1_voltage;                                          // write pv string 1 voltage
   float    string1_current;                                          // write pv string 1 current
   uint16_t string2_voltage;                                          // write pv string 2 voltage
   float    string2_current;                                          // write pv string 2 current
   uint16_t string3_voltage;                                          // write pv string 3 voltage
   float    string3_current;                                          // write pv string 3 current
   uint16_t L1_voltage;                                               // write phase 1 voltage
   uint16_t L1_power;                                                 // write phase 1 power
   uint16_t L2_voltage;                                               // write phase 2 voltage
   uint16_t L2_power;                                                 // write phase 2 power
   uint16_t L3_voltage;                                               // write phase 3 voltage
   uint16_t L3_power;                                                 // write phase 3 power
};  

//extern int sizeof_write_electrical_samples_to;

//
// For values read from flash
//
struct read_electrical_sample_from {
   uint16_t hour_minute;
   uint16_t current_power;                                            // write current power
   float    total_energy;                                             // write total energy
   float    daily_energy;                                             // write daily energy
   uint16_t status;                                                   // write status flags
   uint16_t string1_voltage;                                          // write pv string 1 voltage
   float    string1_current;                                          // write pv string 1 current
   uint16_t string2_voltage;                                          // write pv string 2 voltage
   float    string2_current;                                          // write pv string 2 current
   uint16_t string3_voltage;                                          // write pv string 3 voltage
   float    string3_current;                                          // write pv string 3 current
   uint16_t L1_voltage;                                               // write phase 1 voltage
   uint16_t L1_power;                                                 // write phase 1 power
   uint16_t L2_voltage;                                               // write phase 2 voltage
   uint16_t L2_power;                                                 // write phase 2 power
   uint16_t L3_voltage;                                               // write phase 3 voltage
   uint16_t L3_power;                                                 // write phase 3 power
};  

//extern int sizeof_write_electrical_samples_from;

extern uint8_t  int_hour_from_flash;  
extern uint8_t  int_minute_from_flash;
extern uint16_t int_status_from_flash; 

extern uint32_t flash_write_address;

//extern struct read_electrical_sample_from ;

extern char chr_hour_from_flash[3];
extern char chr_minute_from_flash[3];
extern char chr_current_power_from_flash[5];
extern char chr_total_energy_from_flash[12];
extern char chr_daily_energy_from_flash[10];
extern char chr_status_from_flash[6];
extern char chr_string1_voltage_from_flash[6];
extern char chr_string1_current_from_flash[8];
extern char chr_string2_voltage_from_flash[6];
extern char chr_string2_current_from_flash[8];
extern char chr_string3_voltage_from_flash[6];
extern char chr_string3_current_from_flash[8];
extern char chr_l1_voltage_from_flash[6];
extern char chr_l1_power_from_flash[6];
extern char chr_l2_voltage_from_flash[6];
extern char chr_l2_power_from_flash[6];
extern char chr_l3_voltage_from_flash[6];
extern char chr_l3_power_from_flash[6];

//-----------------------------------------------
// defeinition of first data set
//-----------------------------------------------
struct header {                                                    // define the struct
   uint8_t  datablock_flags;                                       // holds copies of datablock_active_flags
   uint16_t datablock_year;
   uint8_t  datablock_month;
   uint8_t  datablock_date;
};

extern struct header headers[number_of_datagroups];                   // 64 groups (0 - 63) 

extern bool header_datagroup_imported;                            // true if the pointer-groups have been read from flash memory

extern uint32_t erase_datagroup_number;                          // base address for erasing a data-group
extern bool     erase_flag;                                       // signals the erasing of a data-group 
extern int8_t   int_delete_value; 

extern uint32_t base_address_csv;                                 // base address used for csv read
extern uint32_t base_address_json;                                // base address used for json read

extern bool     flash_memory_full;


//-----------------------------------------------
// function definitions
//-----------------------------------------------
bool flash_get_info(void);
void flash_find_free_datagroup(void);
void flash_find_free_dataset(void);
void flash_read_headers(void);
void flash_find_header_for_today(void);
void flash_make_new_header(void);

// void flash_find_active_pointer_group(void);
// void flash_read_active_pointers(void);
// void flash_find_active_blockdata_for_today(void); 
// void flash_create_pointer_area(void); 

// void flash_check_disabled_blocks_and_move(void); 
bool flash_check_current_datablock (uint8_t); 
void flash_write_dataset(void);
void flash_calc_base_address_csv(uint32_t);
void flash_calc_base_address_json(uint32_t);
void flash_read_dataset (uint32_t);
void flash_erase_chip(void);
void flash_erase_datagroup (uint32_t);

#endif