/*
 * Creates datagroups
 * Writes and reads datasets
 *
 *  SPIFlash connected via SPI standard
 *
 */

#include "SPIMemory.h"
#include "kostal_piko.h"
#include "read_write_flash.h"
#include "webscraper.h"
#include "buttons_led.h"
#include "ntp.h"
 
SPIFlash flash;                                           // If you don't specify the library use standard SPI connection

const int datagroup_number = number_of_datagroups;

struct   header headers[datagroup_number];                // used to store flash data in ram for quick access

bool     datagroup_today_flag;                             // flag signaling that a datagroup is found for today
uint8_t  active_datagroup_number= 255;                     // datagroup number for today
bool     dataset_address_found;                            // signals if free dataset is found
//uint32_t active_datagroup_address = 0;                   // address of active datagroup
uint32_t dataset_write_address = 0;                        // address where to write a dataset in flash
uint32_t dataset_read_address  = 0;                        // address where to read a dataset from flash

bool     erase_flag = false;                               // signals the erasing of a data-group   
uint32_t erase_datagroup_number;                           // data-group number to erase
int8_t   int_delete_value;                                 // needed for erasing the header flag


struct write_electrical_sample_to     write_electrical_samples_to;        // values written to flash
//uint8_t sizeof_write_electrical_samples_to = dataset_size;

struct read_electrical_sample_from    read_electrical_samples_from;       // values read from flash
//uint8_t sizeof_write_electrical_samples_from = dataset_size;

uint8_t int_hour_from_flash;  
uint8_t int_minute_from_flash;
uint8_t int_flags_from_flash; 

uint32_t base_address_csv;                                 // base address used for csv read from flash
uint32_t base_address_json;                                // base address used for json read from flash

bool flash_memory_full = false;

//
//Strings for values from flash
//
char chr_hour_from_flash[3];
char chr_minute_from_flash[3];
char chr_current_power_from_flash[5];
char chr_total_energy_from_flash[12];
char chr_daily_energy_from_flash[10];
char chr_status_from_flash[6];
char chr_string1_voltage_from_flash[6];
char chr_string1_current_from_flash[8];
char chr_string2_voltage_from_flash[6];
char chr_string2_current_from_flash[8];
char chr_string3_voltage_from_flash[6];
char chr_string3_current_from_flash[8];
char chr_l1_voltage_from_flash[6];
char chr_l1_power_from_flash[6];
char chr_l2_voltage_from_flash[6];
char chr_l2_power_from_flash[6];
char chr_l3_voltage_from_flash[6];
char chr_l3_power_from_flash[6];


//-------------------------------------------------------------------
bool flash_get_info() {

   uint8_t b1, b2, b3;
   uint32_t JEDEC = 0;
//   String outputString = "";
   
   
   constexpr uint32_t SPI_CLOCK_SPEED = 40000000; // 40 MHz SPI clock speed
   constexpr uint32_t FLASH_SIZE_MB = 16;

   flash.setClock(SPI_CLOCK_SPEED);        // SPI speed
   
   flash.begin(MB(FLASH_SIZE_MB)); // If SPIMemory isn't recognized you can specify the size of memory
   
   
   // remove possible powerdown
   // security registers
   // unlock all
   // enable and reset device
   
   uint32_t capacity = flash.getCapacity()/1000;   
   
   uint16_t ManID = flash.getManID();            // RDID command = 0x9F
//   printf("RDID: %02xh\n", ManID);               // 0xef17
   
   JEDEC = flash.getJEDECID();                   // REMS command = 0x90
   b1 = (JEDEC >> 16) & 0xFF;
   b2 = (JEDEC >> 8)& 0xFF;
   b3 = (JEDEC >> 0)& 0xFF;
   //clearprintBuffer();
//   printf("Manufacturer ID: %02xh\n", b1);
//   printf("Memory Type: %02xh\n", b2);
//   printf("Capacity: %02xh\n", b3);
//   printf("JEDEC ID: %04xh\n", JEDEC);           // 0xef6018
//   printf("\n");
   //
   // Check the different values to see of interface to flash is correct and flash is working
   //
   if (ManID == 0xef17) {
      if (JEDEC == 0xef6018) {
          if (capacity == 16777) {
             return true;              // correct RDID and JEDEC code found
          }   
      }
   }
   return false;                    // incorrect flash info
}

//---------------------------------------------------------
// Find a free datagroup in flash and the start of a free dataset
//---------------------------------------------------------
void flash_find_free_datagroup (){
   
   printf("Starting flash readout and finding storage area.\n");
   dataset_address_found = false;
   flash_read_headers();                      // read all datagroup pointers add find the first free one
   flash_find_header_for_today();             // find header with date of today
   //
   if (datagroup_today_flag) {                // datagroup found for today
      // datagroup for today already exists, find unwritten dataset
      printf("Find free dataset in datagroup\n");
      flash_find_free_dataset();
   }
   else {                                     // no datagroup found for today, is there one available?
      printf("No active datagroup found for today, will try to create one.\n");
      flash_make_new_header();               // no active datagroup found make one if possible
      if (datagroup_today_flag) {
         printf("New datagroup created for today.\n");
      }    
      else {
         printf("No free datagroup found; KostalPiko Reader will continue.\n");
         flash_ok = false;
         drive_led_no_irq(false, false, 500, 500, 5) ; // 5 flashes
      }            
   }
   //   
   printf("Active datagroup number = %d\n", active_datagroup_number);
   printf("Active datagroup base address = %08x\n", active_datagroup_number* datagroup_size);
//   printf("Dataset read address = %08x\n", dataset_read_address);
   printf("Dataset write address = %08x\n", dataset_write_address);
   // dataset_write_address holds active write address  
   printf("Flash_memory_full = %d\n", flash_memory_full);      
}

//---------------------------------------------------------
// Datagroup for today already exists, find free dataset to continue
//---------------------------------------------------------
void flash_find_free_dataset(){
   
   uint32_t dataset_read_address = (active_datagroup_number * datagroup_size) + dataset_size; // skip first dataset = date
   uint8_t hour;
   
   for (uint8_t i = 0; i<(dataset_numbers); i++) {          // all 128 datasets, but skip first
      hour = flash.readByte (dataset_read_address);             // read first address of dataset
      //
      // if hour = 0xFF, this dataset area is not used.
      if (hour == 0xFF) {
         dataset_write_address = dataset_read_address;     // the just read location is available for writing
         printf("Free dataset found on address = %08x\n", dataset_write_address); 
         dataset_address_found = true;
         break;    
      }
      else {      
         dataset_read_address = dataset_read_address + dataset_size;  // to next dataset
      }
   }
   if (!dataset_address_found){
      printf("No free dataset found, ERROR\n");
   }   
}   

//---------------------------------------------------------
// Read the Datablock active flags from the active pointer-group and store
// them in an array
// If the value of active flag = 0xFF the datablock is erased and not used
// If the value 0xAA then the block is used and holds data
// If the value = 0x00 then the data arra is erased, but this flag still exists.
//---------------------------------------------------------
void flash_read_headers() {

   uint8_t i;
   uint32_t datagroup_base_address = 0;                                     // address in flash memory
   //
   // now copy the datablock_header from flash to array "headers"
   //
   for (i = 0; i<(datagroup_number); i++) {       // all 128 datagroups
      headers[i].datablock_flags = flash.readByte (datagroup_base_address + 0); 
      headers[i].datablock_year  = flash.readShort(datagroup_base_address + 1);
      headers[i].datablock_month = flash.readByte (datagroup_base_address + 3);
      headers[i].datablock_date  = flash.readByte (datagroup_base_address + 4);
      //
//      printf("headers[i].datablock_year  = %d\n", headers[i].datablock_year);
//      printf("headers[i].datablock_month = %d\n", headers[i].datablock_month);
//      printf("headers[i].datablock_date  = %d\n", headers[i].datablock_date);
      //
//      printf(" i = %d\n", i);
      datagroup_base_address = datagroup_base_address + datagroup_size;  // to next datagroup
   }
   printf("All datagroup headers are read\n");
  //
//  printf("datagroup_number = %d\n", datagroup_number);
  //
//  for(i = 0; i <(datagroup_number); i++) {
//     printf("i = %d ", i);
//     printf("datablock_flag = %d \n", headers[i].datablock_flags);
//  }
//  printf("\n");
 
}

//-------------------------------------------------------------------
// Find header with same date as today
//-------------------------------------------------------------------
void flash_find_header_for_today() {
   //
   datagroup_today_flag = false;
   active_datagroup_number= 255;                // means no active datagroup
   //
   // Now find the data_block that is in use for this day. Must hold 0xAA 
   //  and year, month and date must be equal to current time.
   // Flags are read from ram to speedup things
   //
//   printf("datagroup_number = %d\n", datagroup_number);
//   printf("current_year = %d\n", current_year);
//   printf("current_month = %d\n", current_month);
//   printf("current_mday = %d\n", current_mday);
   for (uint8_t i = 0; i<datagroup_number; i++) {    // for all 128 datagroups
//      printf("i = %d\n", i);
//      printf("headers[%d].datablock_flags = %d\n", i, headers[i].datablock_flags);
      if (headers[i].datablock_flags == 0xAA) {
//         printf("datablock_flag of 0xAA found\n");
//         printf("headers[%d].datablock_year  = %d\n", i, headers[i].datablock_year);
//         printf("headers[%d].datablock_month = %d\n", i, headers[i].datablock_month);
//         printf("headers[%d].datablock_date  = %d\n", i, headers[i].datablock_date);
         if ((headers[i].datablock_year  == current_year) && 
             (headers[i].datablock_month == current_month) && 
             (headers[i].datablock_date  == current_mday)){ 
            // a flag in use is found and the year/month/date equals the current year/month/date 
            active_datagroup_number= i;
            datagroup_today_flag = true;
            printf("Datagroup number that will be used = %d\n", i);  
            break;
         }       
      } 
   }   
}

//-------------------------------------------------------------------
// Find unused datagroup an create date for this datagroup
//-------------------------------------------------------------------
void flash_make_new_header(void) {
   //
   // There seems to be no datagroup with the current date, create one
   // This can only be done on the flag that is 0xFF. let's find one
   // if no flag can be found, maybe we need to cleanup the pointer groups
   // Searching is done over the headergroup in RAM
   
   uint32_t datagroup_base_address = 0;       // address in flash
   datagroup_today_flag = false;     // no free datagroup found
   
   for (uint8_t i = 0; i<datagroup_number; i++) {
      if (headers[i].datablock_flags == FLASH_ERASED_FLAG) {                                     // free datagroup
         //
         // set this pointer active in flash and in the internal header array
         //
         datagroup_base_address = i * datagroup_size;                               // calc start address of datagroup
         //
         flash.writeByte (datagroup_base_address + 0, FLASH_USED_FLAG, false);        //write with no check
         flash.writeShort(datagroup_base_address + 1, current_year,  false);
         flash.writeByte (datagroup_base_address + 3, current_month, false);
         flash.writeByte (datagroup_base_address + 4, current_mday,  false);
         //
         // Write also in internal array
         //
         headers[i].datablock_flags = FLASH_USED_FLAG;                                         // set "in use"
         headers[i].datablock_year  = current_year;
         headers[i].datablock_month = current_month;
         headers[i].datablock_date  = current_mday;
         //
         // set values for active datagroup
         //
         datagroup_today_flag = true;               // free datagroup found
         active_datagroup_number = i;
         printf("active datagroup number in use = %d\n", i); 
         dataset_write_address = (i * datagroup_size) + dataset_size;
         printf("flash_write_address for today = %08x\n", dataset_write_address);   // number of active datagroup
         break; 
      }    
   }
   //
   // Memory full or not
   //
   if (datagroup_today_flag){                      // free datagroup is found
      flash_memory_full = false;
   }
   else {
      flash_memory_full = true;
   }     
}


//-------------------------------------------------------------------
// Check if datagroup has the date of today
//-------------------------------------------------------------------
bool flash_check_current_datablock (uint8_t current_mday){

   if (headers[active_datagroup_number].datablock_date == current_mday) {
      return true;
   }
   else {
      return false;   
   }
}


//-------------------------------------------------------------------
// Write electrical values to flash memory
// The function is only called if data must be written in flash
//------------------------------------------------------------------
void flash_write_dataset () {
 
   //
   // Store values in struct to have a faster write,struct is written in 1 go
   //
   write_electrical_samples_to.hour_minute       = (current_hour << 8) + current_minute;  // int16
   write_electrical_samples_to.current_power     = data.currentpower    ;                 // int16
   write_electrical_samples_to.total_energy      = data.totalEnergy;                      // float
   if (data.totalEnergy == 0) {
//      write_electrical_samples_to.status         = ((data.status == "MPP") ? 1 : 0) + 10;        // int16
      write_electrical_samples_to.status         = 10;        // int16
   } 
   else {
      write_electrical_samples_to.status            = (data.status == "MPP") ? 1 : 0;        // int16   
   }   
   write_electrical_samples_to.daily_energy      = data.dailyEnergy;                      // float
//   write_electrical_samples_to.status            = (data.status == "MPP") ? 1 : 0;        // int16
   write_electrical_samples_to.string1_voltage   = data.str1Voltage;                      // int16
   write_electrical_samples_to.string1_current   = data.str1Current;                      // float
   write_electrical_samples_to.string2_voltage   = data.str2Voltage;    
   write_electrical_samples_to.string2_current   = data.str2Current;    
   write_electrical_samples_to.string3_voltage   = 0;    
   write_electrical_samples_to.string3_current   = 0.0;    
   write_electrical_samples_to.L1_voltage        = data.l1Voltage  ;    
   write_electrical_samples_to.L1_power          = data.l1Power    ;    
   write_electrical_samples_to.L2_voltage        = data.l2Voltage  ;    
   write_electrical_samples_to.L2_power          = data.l2Power    ;    
   write_electrical_samples_to.L3_voltage        = data.l3Voltage  ;    
   write_electrical_samples_to.L3_power          = data.l3Power    ;    
   //
//   printf("write_total_energy = %d\n", int(write_electrical_samples_to.total_energy));
//   printf("active_datagroup_address = %x\n", active_datagroup_address);
//   printf("Dataset written on address = %08x \n",dataset_write_address);

   // write values to flash
   flash.writeAnything(dataset_write_address, write_electrical_samples_to, false);                  
   dataset_write_address = dataset_write_address + dataset_size;  // go to location for next sample
}

//-------------------------------------------------------------------
// Calc base address for csv read
//-------------------------------------------------------------------
void flash_calc_base_address_csv (uint32_t int_csv_value){

   base_address_csv = (int_csv_value * datagroup_size) + dataset_size;  // skip flag and date block
}

//-------------------------------------------------------------------
// Calc base address for json read
//-------------------------------------------------------------------
void flash_calc_base_address_json (uint32_t int_json_value){

   base_address_json = (int_json_value * datagroup_size) + dataset_size;  // skip flag and date block
}

//-------------------------------------------------------------------
// Read values from flash memory
// Converts values to strings
//------------------------------------------------------------------
void flash_read_dataset (uint32_t flash_read_address) {
//   printf("Flash read address = %08x \n",flash_read_address);
   // read values to flash
   flash.readAnything(flash_read_address, read_electrical_samples_from, false);                  // read values from 1 minute point.
   //
   // Store values in struct to have a faster read
   //
//   printf("read_electrical_samples_in.elec_hour_minute = %x\n", read_electrical_samples_in.elec_hour_minute);
   int_hour_from_flash   = read_electrical_samples_from.hour_minute >> 8;
   int_minute_from_flash = read_electrical_samples_from.hour_minute & 0x00FF;
   //
   // Check if these locations have been written
   //
   if ((int_hour_from_flash != 255) && (int_minute_from_flash != 255)) {  // has some written value
      itoa(int_hour_from_flash,   chr_hour_from_flash,   10);             // 10 is base
      itoa(int_minute_from_flash, chr_minute_from_flash, 10);
      //
      itoa(read_electrical_samples_from.current_power,  chr_current_power_from_flash, 10);
      //
      dtostrf(read_electrical_samples_from.total_energy, 8, 0, chr_total_energy_from_flash); // total 8 digits
//      printf ("read float total.energy = %.2f kWh\n", read_electrical_samples_from.total_energy);   
//      printf ("read char total.energy = %s kWh\n", chr_total_energy_from_flash);      
      itoa   (read_electrical_samples_from.total_energy, chr_total_energy_from_flash, 10);
      dtostrf(read_electrical_samples_from.daily_energy, 8, 2, chr_daily_energy_from_flash); // total 6 digits and 2 digits behind the "."
      if (read_electrical_samples_from.total_energy == 0) {
         itoa((read_electrical_samples_from.status + 20), chr_status_from_flash, 10);
      }
      else {
         itoa(read_electrical_samples_from.status, chr_status_from_flash, 10);
      }
      //
      itoa(read_electrical_samples_from.string1_voltage, chr_string1_voltage_from_flash, 10);
      dtostrf(read_electrical_samples_from.string1_current, 6, 2, chr_string1_current_from_flash);
      itoa(read_electrical_samples_from.string2_voltage, chr_string2_voltage_from_flash, 10);
      dtostrf(read_electrical_samples_from.string2_current, 6, 2, chr_string2_current_from_flash);
      itoa(read_electrical_samples_from.string3_voltage, chr_string3_voltage_from_flash, 10);
      dtostrf(read_electrical_samples_from.string3_current, 6, 2, chr_string3_current_from_flash);
      //
      itoa(read_electrical_samples_from.L1_voltage, chr_l1_voltage_from_flash, 10);  
      itoa(read_electrical_samples_from.L1_power, chr_l1_power_from_flash, 10);  
      itoa(read_electrical_samples_from.L2_voltage, chr_l2_voltage_from_flash, 10);  
      itoa(read_electrical_samples_from.L2_power, chr_l2_power_from_flash, 10);  
      itoa(read_electrical_samples_from.L3_voltage, chr_l3_voltage_from_flash, 10);  
      itoa(read_electrical_samples_from.L3_power, chr_l3_power_from_flash, 10); 
      
//      printf("read_total_energy = %d\n", int(read_electrical_samples_from.total_energy));

   }   
   //
/*   printf("hour_from_flash = %s\n",   chr_hour_from_flash);
   printf("minute_from_flash = %s\n", chr_minute_from_flash);
   printf("current_power = %s\n",     chr_current_power_from_flash);
   printf("total_energy = %s\n",      chr_total_energy_from_flash);
   printf("daily_energy = %s\n",      chr_daily_energy_from_flash);
   printf("string1_voltage = %s\n",   chr_string1_voltage_from_flash);
   printf("string1_current = %s\n",   chr_string1_current_from_flash);
   printf("string2_voltage = %s\n",   chr_string2_voltage_from_flash);
   printf("string2_current = %s\n",   chr_string2_current_from_flash);
   printf("string3_voltage = %s\n",   chr_string3_voltage_from_flash);
   printf("string3_current = %s\n",   chr_string3_current_from_flash);
   printf("L1_voltage = %s\n",        chr_l1_voltage_from_flash);
   printf("L1_power = %s\n",          chr_l1_power_from_flash);
   printf("L2_voltage = %s\n",        chr_l2_voltage_from_flash);
   printf("L2_power = %s\n",          chr_l2_power_from_flash);
   printf("L3_voltage = %s\n",        chr_l3_voltage_from_flash);
   printf("L3_power = %s\n",          chr_l3_power_from_flash);
   printf("status_from_flash = %s\n", chr_status_from_flash);
*/   
}
     

//-------------------------------------------------------------------
// Erase the full flash chip. The KostalPiko Reader will restart after erase is finished
//-------------------------------------------------------------------
void flash_erase_chip(){
    
   printf("Start full erase of memory chip\n");
   flash_memory_full = false;
   flash.eraseChip();                                             // clear complete flash device
   printf("Full erase finished\n");
}


//-------------------------------------------------------------------
// Erase a data-group (2 blocks of 64 kbyte each)
//-------------------------------------------------------------------
void flash_erase_datagroup(uint32_t erase_datagroup_number) {
 
   printf("Starting to erase datagroup number = %d\n", erase_datagroup_number);
   //
   uint32_t erase_address = ((erase_datagroup_number * datagroup_size) + (datagroup_size/2));
//   printf("erase_address %08x\n", erase_address);
   flash.eraseBlock64K ((erase_datagroup_number * datagroup_size) + (datagroup_size/2));   // erase last of 2 64k blocks
   yield();
   erase_address = (erase_datagroup_number * datagroup_size);
//   printf("erase_address %08x\n", erase_address);
   flash.eraseBlock64K ((erase_datagroup_number * datagroup_size)                     );   // erase first 64k block
   yield();
   erase_flag = false;           
   flash_memory_full = false;                                       // remove erase flag
   headers[erase_datagroup_number].datablock_flags = 0xFF;                // also update header array
   headers[erase_datagroup_number].datablock_year  = 0xFF;
   headers[erase_datagroup_number].datablock_month = 0xFF;
   headers[erase_datagroup_number].datablock_date  = 0xFF;
   //
//   printf("Erasing of datagroup is finished\n");   
   
}   
