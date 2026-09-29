// =============================================================================
// CSVChunkedResponse.h
// Header file for CSV chunked response class for ESP32/ESP8266 web server
// Streams CSV data from flash memory without loading entire dataset into RAM
// =============================================================================

#ifndef _CSV_CHUNKED_H
#define _CSV_CHUNKED_H

//#include <Arduino.h>

//#include "ESPAsyncWebServer.h"


// =============================================================================
// CSVChunkedResponse CLASS
// =============================================================================

/**
 * Async chunked response class for streaming CSV data from flash memory
 * 
 * This class implements a state machine that:
 * 1. Writes CSV headers
 * 2. Streams data rows one at a time from flash
 * 3. Handles errors gracefully
 * 
 * The chunked approach allows exporting large datasets without exhausting RAM.
 * 
 * Usage:
 *   AsyncAbstractResponse *response = new CSVChunkedResponse(0);
 *   request->send(response);
 */

// =============================================================================
// CSVChunkedResponse CLASS
// =============================================================================

class CSVChunkedResponse : public AsyncAbstractResponse {
public:
  /**
   * Constructor - creates a new CSV chunked response
   * 
   * Validates the datagroup index, checks header data, builds the filename,
   * and calculates the flash base address. If any validation fails, the
   * response enters error state.
   * 
   * @param csvIndex Index of the datagroup to export (0 to datagroup_number-1)
   */
   CSVChunkedResponse(int csvIndex);

  /**
   * Check if the data source is valid and ready to send
   * Called by AsyncWebServer before starting transmission
   * 
   * @return true if valid and ready, false if error occurred during construction
   */
  bool _sourceValid() const override;

  /**
   * Fill buffer with CSV data
   * 
   * This method is called repeatedly by the web server to stream data.
   * It implements a state machine that progresses through:
   * - STATE_HEADER: Write CSV column headers
   * - STATE_DATA: Write data rows from flash
   * - STATE_ERROR: Write error message (if error occurred)
   * - STATE_DONE: Signal completion
   * 
   * @param buf Buffer to fill with data
   * @param maxLen Maximum bytes to write to buffer
   * @return Number of bytes written, or 0 when transmission is complete
   */
  size_t _fillBuffer(uint8_t *buf, size_t maxLen) override;

private:
  // =============================================================================
  // STATE MACHINE STATES
  // =============================================================================
  
  enum State {
    STATE_HEADER,     // Writing CSV column headers
    STATE_DATA,       // Writing CSV data rows
    STATE_ERROR,      // Error occurred - write error message
    STATE_DONE        // Transmission complete
  };

  // =============================================================================
  // MEMBER VARIABLES
  // =============================================================================
  
  int _csvIndex;                    // Datagroup index being exported
  uint8_t _state;                   // Current state in state machine
  int _i;                           // Step counter within sample
  int _j;                           // Sample counter (0 to SAMPLES_PER_STEP-1)
  char _line[512];                  // Buffer for formatting output lines
  bool _error;                      // Error flag - true if error occurred
  const char* _errorMessage;        // Pointer to error message string (if error)
  

  // =============================================================================
  // PRIVATE METHODS
  // =============================================================================

  /**
   * Set error state with message
   * 
   * Marks the response as errored, stores the error message,
   * sets HTTP status to 500, and logs to serial console.
   * 
   * @param message Error message to store and log
   */
  void setError(const char* message);

  /**
   * Write a C-string line to output buffer
   * 
   * Safely copies up to maxLen bytes from line to buffer.
   * Handles null pointers gracefully.
   * 
   * @param buf Output buffer
   * @param maxLen Maximum bytes available in buffer
   * @param line C-string to write (null-terminated)
   * @return Number of bytes written
   */
  size_t writeLine(uint8_t *buf, size_t maxLen, const char *line);

  /**
   * Write a String object line to output buffer
   * 
   * Overloaded version that accepts Arduino String objects.
   * 
   * @param buf Output buffer
   * @param maxLen Maximum bytes available in buffer
   * @param line Arduino String to write
   * @return Number of bytes written
   */
  size_t writeLine(uint8_t *buf, size_t maxLen, const String &line);

  /**
   * Write the next CSV data row to output buffer
   * 
   * Reads one electrical sample from flash, validates the data,
   * formats it as a CSV row, and writes to the buffer.
   * 
   * Error handling:
   * - Skips invalid timestamps (hour>23, minute>59, second>59, or negative values)
   * - Returns false and sets error on flash read failures
   * - Returns false and sets error on null pointer data
   * - Returns false and sets error on buffer overflow
   * 
   * @param buf Output buffer
   * @param maxLen Maximum bytes available in buffer
   * @param written Reference to track total bytes written in this call
   * @return true to continue processing, false when no more samples or error
   */
  bool writeNextCSVLine(uint8_t *buf, size_t maxLen, size_t &written);

  /**
   * Advance to next sample position
   * 
   * Increments the step counter (_i) and sample counter (_j)
   * appropriately based on the step size. When _i reaches 'step',
   * it resets to 0 and _j increments.
   */
  void advance();
};


// =============================================================================
// UTILITY FUNCTIONS (optional - for convenience)
// =============================================================================

/**
 * Validate datagroup index is within valid range
 * @param index Index to validate
 * @return true if valid (0 <= index < datagroup_number), false otherwise

inline bool isValidCSVIndex(int index) {
  return (index >= 0 && index < datagroup_number);
}
 */
/**
 * Check if datablock is in use
 * @param index Datagroup index
 * @return true if in use (flags == 0xAA), false otherwise

inline bool isCSVDatablockInUse(int index) {
  if (!isValidCSVIndex(index)) return false;
  return (headers[index].datablock_flags == 0xAA);
}
 */
// =============================================================================
// USAGE EXAMPLE DOCUMENTATION
// =============================================================================

/*
 * SETUP IN YOUR WEB SERVER CODE:
 * 
 * #include "CSVChunkedResponse.h"
 * 
 * void setupWebServer() {
 *   server.on("/download_csv", HTTP_GET, [](AsyncWebServerRequest *request) {
 *     
 *     if (!request->hasParam("index")) {
 *       request->send(400, "text/plain", "Missing index parameter");
 *       return;
 *     }
 *     
 *     int index = request->getParam("index")->value().toInt();
 *     
 *     if (!isValidCSVIndex(index)) {
 *       request->send(400, "text/plain", "Invalid index");
 *       return;
 *     }
 *     
 *     if (!isCSVDatablockInUse(index)) {
 *       request->send(404, "text/plain", "Data block not in use");
 *       return;
 *     }
 *     
 *     AsyncAbstractResponse *response = new CSVChunkedResponse(index);
 *     if (response == nullptr) {
 *       request->send(500, "text/plain", "Memory allocation failed");
 *       return;
 *     }
 *     
 *     request->send(response);
 *   });
 * 
 *   server.begin();
 * }
 * 
 * CLIENT USAGE:
 *   http://device-ip/download_csv?index=0
 * 
 * OUTPUT FORMAT:
 *   Time,Flags,Voltage(V),Current(A),Power(W),Energy(kWh),Powerfactor(cos PHI),Apperent Power(kVA),Reactive Power(kVAR)
 *   10:30:15,0x01,230.5,4.2,968.1,12.5,0.98,988.5,150.2
 *   10:30:16,0x01,230.6,4.1,965.2,12.5,0.98,985.3,148.9
 *   ...
 */

#endif // CSV_CHUNKED_RESPONSE_H