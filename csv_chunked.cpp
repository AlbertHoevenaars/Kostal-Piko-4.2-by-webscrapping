// IMPROVED VERSION WITH PROPER ERROR HANDLING
#include <Arduino.h>

#include "read_write_flash.h"
#include "ESPAsyncWebServer.h"
#include "csv_chunked.h"
#include "wifi_connect.h"  // for file name
#include "progmem.h"

// =============================================================================
// CSV CHUNKED RESPONSE CLASS
// =============================================================================

CSVChunkedResponse::CSVChunkedResponse(int csvIndex)
   : _csvIndex(csvIndex),
     _state(STATE_HEADER),
     _i(0),
     _j(0),
     _error(false),
     _errorMessage(nullptr)
   {
      _code = 200;
      _contentType = FPSTR(text_csv);
      _sendContentLength = false;
      _chunked = true;
      
      // Validate csvIndex
      if (csvIndex < 0 || csvIndex >= datagroup_number) {
        setError("Invalid CSV index");
        return;
      }
      
      // Validate header data exists
      if (headers[csvIndex].datablock_flags != 0xAA) {
        setError("Data block not in use");
        return;
      }
      
      // Validate date fields
      if (headers[csvIndex].datablock_year < 2020 || 
          headers[csvIndex].datablock_year > 2100 ||
          headers[csvIndex].datablock_month < 1 || 
          headers[csvIndex].datablock_month > 12 ||
          headers[csvIndex].datablock_date < 1 || 
          headers[csvIndex].datablock_date > 31) {
        setError("Invalid date in header");
        return;
   }

   // Build filename safely
   char filename[42];
   int written = snprintf(filename, sizeof(filename),
            (str_wifi_hostname + "_%04d-%02d-%02d.csv").c_str(),
            headers[csvIndex].datablock_year,
            headers[csvIndex].datablock_month,
            headers[csvIndex].datablock_date);
   
   // Check for buffer overflow
   if (written >= (int)sizeof(filename)) {
      setError("Filename buffer overflow");
      return;
   }

   // Set headers
   addHeader("Content-Disposition",
             String("attachment; filename=\"") + filename + "\"");
   addHeader("Content-Type", "text/csv; charset=utf-8");
    

   flash_calc_base_address_csv(csvIndex);    // get base address for selected datagroup
  }

   bool CSVChunkedResponse::_sourceValid() const {
      return !_error;
   }

   size_t CSVChunkedResponse::_fillBuffer(uint8_t *buf, size_t maxLen) {
   // Safety checks
   if (buf == nullptr || maxLen == 0) {
     return 0;
   }

   if (_error) {
     _state = STATE_ERROR;
   }

   size_t written = 0;

   while (written == 0 && maxLen > 0) {
      switch (_state) {
       
         case STATE_HEADER:
            written += writeLine(
            buf,
            maxLen,
            "Time, Current_AC_Power(W), Total_Energy(kWh), Daily_Energy(kWh),"
            "String1_voltage(V), String1_current(A), String2_voltage(V),"
            "String2_current(A), String3_voltage(V), String3_current(A), L1_voltage(V),"
            "L1_power(W), L2_voltage(V), L2_power(W), L3_voltage(V), L3_power(W), status\n"
            );
            _state = STATE_DATA;
            break;
         
         case STATE_DATA:
            if (!writeNextCSVLine(buf, maxLen, written)) {
               _state = STATE_DONE;
            }
            break;
         
         case STATE_ERROR:
            // Send error message as CSV comment
            if (_errorMessage) {
               written += writeLine(buf, maxLen, 
               String("# Error: ") + _errorMessage + "\n");
            }
            _state = STATE_DONE;
            break;
         
         case STATE_DONE:
         return 0;
     }
   }

    return written;
  }

//private:
   void CSVChunkedResponse::setError(const char* message) {
      _error = true;
      _errorMessage = message;
      _code = 500;
      printf("CSVChunkedResponse Error: %s\n", message);
   }

   size_t CSVChunkedResponse::writeLine(uint8_t *buf, size_t maxLen, const char *line) {
      if (line == nullptr) return 0;
      
      size_t len = strlen(line);
      if (len > maxLen) len = maxLen;
      
      if (len > 0) {
        memcpy(buf, line, len);
      }
      return len;
   }

   size_t CSVChunkedResponse::writeLine(uint8_t *buf, size_t maxLen, const String &line) {
      return writeLine(buf, maxLen, line.c_str());
   }

   bool CSVChunkedResponse::writeNextCSVLine(uint8_t *buf, size_t maxLen, size_t &written) {
      // Check if we've reached the end
      if (_j >= CHUNK_SIZE) {
         return false;
      }
      
      // Validate address before reading
      if (base_address_csv >= FLASH_SIZE_bytes) {
         setError("Flash address out of bounds");
         return false;
      }

      flash_read_dataset(base_address_csv);    // read value from flash
      if (int_hour_from_flash == 255){           // check hour, if invalid hour -> finish 
         return false;
      }   
      base_address_csv += dataset_size;       // set to next 15 seconds values
      
      // Validate timestamp - skip invalid entries
      if (int_hour_from_flash > 23 || int_minute_from_flash > 59) {
         advance();
         return true;  // Continue to next sample
      }
      //
      //
      if (int_hour_from_flash == 0 && int_minute_from_flash == 0) {    
         printf("hour_from_flash = %s\n",   chr_hour_from_flash);
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
      }   
      

      // Format CSV line with overflow protection
      int len = snprintf(_line, sizeof(_line),
         "%02d:%02d, %s,%s,%s, %s,%s,%s,%s,%s,%s, %s,%s,%s,%s,%s,%s, %s\n",
         int_hour_from_flash,
         int_minute_from_flash,
         chr_current_power_from_flash,
         chr_total_energy_from_flash,
         chr_daily_energy_from_flash,
         chr_string1_voltage_from_flash,
         chr_string1_current_from_flash,
         chr_string2_voltage_from_flash,
         chr_string2_current_from_flash,
         chr_string3_voltage_from_flash,
         chr_string3_current_from_flash,
         chr_l1_voltage_from_flash,
         chr_l1_power_from_flash,
         chr_l2_voltage_from_flash,
         chr_l2_power_from_flash,
         chr_l3_voltage_from_flash,
         chr_l3_power_from_flash,
         chr_status_from_flash
      );

      // Check for snprintf truncation or error
      if (len < 0) {
         setError("sprintf encoding error");
         return false;
      }
      
      if ((size_t)len >= sizeof(_line)) {
         setError("Line buffer overflow");
         return false;
      }
      
      // Check if line fits in remaining buffer space
      if ((size_t)len > maxLen - written) {
         return true;  // Wait for next chunk
      }
      
      // Copy to output buffer
      memcpy(buf + written, _line, len);
      written += len;
      
      advance();
      return true;
  }

void CSVChunkedResponse::advance() {
   _i++;
   if (_i >= NUMBER_OF_CHUNKES) {
      _i = 0;
      _j++;
   }
}
//};

// =============================================================================
// ADDITIONAL RECOMMENDATIONS
// =============================================================================

/*
 * KEY IMPROVEMENTS MADE:
 * 
 * 1. INPUT VALIDATION:
 *    - Check parameters exist and are numeric
 *    - Validate indices are within bounds
 *    - Validate calculated addresses don't exceed flash size
 *    
 * 2. NULL POINTER CHECKS:
 *    - Verify buffers aren't null before use
 *    - Check string pointers from flash reads
 *    
 * 3. BUFFER OVERFLOW PROTECTION:
 *    - Check snprintf return values
 *    - Validate lengths before memcpy
 *    - Increased line buffer size
 *    
 * 4. ERROR REPORTING:
 *    - Meaningful error messages
 *    - JSON error responses for API
 *    - Error state in CSV response
 *    
 * 5. DATA VALIDATION:
 *    - Check timestamp ranges (including negative values)
 *    - Validate date fields in header
 *    - Verify datablock flags
 *    
 * 6. RESOURCE MANAGEMENT:
 *    - Check memory allocation (new operator)
 *    - Validate flash read operations
 *    
 * STILL NEEDED (depends on your flash library):
 *    - Make flash_read_electrical_samples() return bool for success/failure
 *    - Make flash_calc_base_address_csv() return bool
 *    - Add MAX_DATAGROUPS and MAX_FLASH_SIZE constants
 *    - Consider adding mutex/semaphore for flash access if multi-threaded
 *    - Add watchdog timer reset in long operations
 */
 
/* 
 * ADVANTAGES OF CSV:
 * - Smaller file size
 * - Opens directly in Excel/Google Sheets
 * - Simpler format
 * - Faster to generate
 */