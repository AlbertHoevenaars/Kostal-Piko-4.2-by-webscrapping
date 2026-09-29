#include <Arduino.h>

#include "read_write_flash.h"
#include "ESPAsyncWebServer.h"
#include "wifi_connect.h"
#include "json_chunked.h"

// =============================================================================
// CONSTRUCTOR
// =============================================================================

JSONChunkedResponse::JSONChunkedResponse(int jsonIndex)
    : _bufLen(0),
      _bufPos(0),
      _jsonIndex(jsonIndex),
      _state(STATE_START),
      _i(0),
      _j(0),
      _error(false),
      _errorMessage(nullptr),
      _firstSample(true),
      _pendingSecondPart(false)
{
    _code              = 200;
    _contentType       = "application/json";
    _sendContentLength = false;
    _chunked           = true;

    // Validate jsonIndex
    if (jsonIndex < 0 || jsonIndex >= datagroup_number) {
        setError("Invalid JSON index");
        return;
    }

    // Validate header data exists
    if (headers[jsonIndex].datablock_flags != 0xAA) {
        setError("Data block not in use");
        return;
    }

    // Validate date fields
    if (headers[jsonIndex].datablock_year  < 2020 ||
        headers[jsonIndex].datablock_year  > 2100 ||
        headers[jsonIndex].datablock_month < 1    ||
        headers[jsonIndex].datablock_month > 12   ||
        headers[jsonIndex].datablock_date  < 1    ||
        headers[jsonIndex].datablock_date  > 31)
    {
        setError("Invalid date in header");
        return;
    }

    // Build attachment filename
    char filename[42];
    int written = snprintf(filename, sizeof(filename),
                           (str_wifi_hostname + "_%04d-%02d-%02d.json").c_str(),
                           headers[jsonIndex].datablock_year,
                           headers[jsonIndex].datablock_month,
                           headers[jsonIndex].datablock_date);

    if (written >= (int)sizeof(filename)) {
        setError("Filename buffer overflow");
        return;
    }

    addHeader("Content-Disposition",
              String("attachment; filename=\"") + filename + "\"");
    addHeader("Content-Type", "application/json; charset=utf-8");

    // Calculate base flash address for the selected datagroup
    flash_calc_base_address_json(jsonIndex);
}

// =============================================================================
// AsyncAbstractResponse interface
// =============================================================================

bool JSONChunkedResponse::_sourceValid() const {
    return !_error;
}

// _fillBuffer is called repeatedly by ESPAsyncWebServer.
// Each call must return > 0 bytes until the response is complete,
// at which point it must return 0.
//
// Strategy:
//   1. If the internal buffer has pending bytes, drain them first.
//   2. Otherwise, run the state machine to generate the next chunk of data
//      into the internal buffer, then drain what we can.
//
size_t JSONChunkedResponse::_fillBuffer(uint8_t *buf, size_t maxLen) {

    if (buf == nullptr || maxLen == 0) return 0;

    if (_error) _state = STATE_ERROR;

    // --- Phase 1: drain any previously generated data ----------------------
    size_t drained = drainInternalBuf(buf, maxLen);
    if (drained > 0) return drained;

    // --- Phase 2: internal buffer empty – generate the next piece ----------
    // We loop until we have put something into _internalBuf (or hit DONE).
    while (_bufLen == 0) {

        switch (_state) {

            // -----------------------------------------------------------------
            case STATE_START:
                appendToInternalBuf("{\n", 2);
                _state = STATE_METADATA;
                break;

            // -----------------------------------------------------------------
            case STATE_METADATA: {
                int len = snprintf(_line, sizeof(_line),
                    "  \"date\": \"%04d-%02d-%02d\",\n"
                    "  \"format_version\": \"1.0\",\n",
                    headers[_jsonIndex].datablock_year,
                    headers[_jsonIndex].datablock_month,
                    headers[_jsonIndex].datablock_date);

                if (len < 0 || (size_t)len >= sizeof(_line)) {
                    setError("Metadata formatting error");
                    break;
                }
                appendToInternalBuf(_line, len);
                _state = STATE_DATA_START;
                break;
            }

            // -----------------------------------------------------------------
            case STATE_DATA_START: {
                const char *s = "  \"samples\": [\n";
                appendToInternalBuf(s, strlen(s));
                _state       = STATE_DATA;
                _firstSample = true;
                break;
            }

            // -----------------------------------------------------------------
            // STATE_DATA generates exactly ONE complete sample per iteration
            // so the internal buffer never needs to hold more than one sample.
            case STATE_DATA: {
                if (!_pendingSecondPart) {
                    // Try to read and format the first half of the next sample
                    if (!generateFirstPart()) {
                        // No more data – close the array
                        _state = STATE_DATA_END;
                        break;
                    }
                    // generateFirstPart() may have decided to skip this entry
                    // (invalid timestamp) – in that case _bufLen is still 0
                    // and we loop again naturally.
                    if (_bufLen > 0) {
                        _pendingSecondPart = true;
                    }
                } else {
                    // First half already written; append the second half
                    if (!generateSecondPart()) {
                        // Formatting error – abort
                        _state = STATE_DATA_END;
                        break;
                    }
                    _pendingSecondPart = false;
                    advance();
                }
                break;
            }

            // -----------------------------------------------------------------
            case STATE_DATA_END: {
                const char *s = "\n  ],\n";
                appendToInternalBuf(s, strlen(s));
                _state = STATE_END;
                break;
            }

            // -----------------------------------------------------------------
            case STATE_END: {
                int len = snprintf(_line, sizeof(_line),
                    "  \"total_samples\": %d\n}\n", (_j * NUMBER_OF_CHUNKES + _i));

                if (len < 0 || (size_t)len >= sizeof(_line)) {
                    setError("End formatting error");
                    break;
                }
                appendToInternalBuf(_line, len);
                _state = STATE_DONE;
                break;
            }

            // -----------------------------------------------------------------
            case STATE_ERROR: {
                if (_errorMessage) {
                    int len = snprintf(_line, sizeof(_line),
                        "{\"error\": \"%s\"}\n", _errorMessage);
                    if (len > 0 && (size_t)len < sizeof(_line)) {
                        appendToInternalBuf(_line, len);
                    }
                }
                _state = STATE_DONE;
                break;
            }

            // -----------------------------------------------------------------
            case STATE_DONE:
                return 0;   // Signal EOF to ESPAsyncWebServer
        }
    }

    // Drain whatever the state machine just produced
    return drainInternalBuf(buf, maxLen);
}

// =============================================================================
// PRIVATE HELPERS
// =============================================================================

void JSONChunkedResponse::setError(const char *message) {
    _error        = true;
    _errorMessage = message;
    _code         = 500;
    printf("JSONChunkedResponse Error: %s\n", message);
}

// --------------------------------------------------------------------------
// appendToInternalBuf
// Appends 'len' bytes of 'line' to the internal buffer.
// Returns false if there is not enough space (sets error).
// --------------------------------------------------------------------------
bool JSONChunkedResponse::appendToInternalBuf(const char *line, int len) {
    if (line == nullptr || len <= 0) return true;

    if (_bufLen + (size_t)len > INTERNAL_BUF_SIZE) {
        setError("Internal buffer overflow");
        return false;
    }
    memcpy(_internalBuf + _bufLen, line, len);
    _bufLen += len;
    return true;
}

// --------------------------------------------------------------------------
// drainInternalBuf
// Copies as many bytes as possible from _internalBuf into ESPAsync's buf.
// Resets pointers once the internal buffer is fully drained.
// --------------------------------------------------------------------------
size_t JSONChunkedResponse::drainInternalBuf(uint8_t *buf, size_t maxLen) {
    size_t available = _bufLen - _bufPos;
    if (available == 0) return 0;

    size_t toCopy = (available < maxLen) ? available : maxLen;
    memcpy(buf, _internalBuf + _bufPos, toCopy);
    _bufPos += toCopy;

    if (_bufPos >= _bufLen) {
        // Fully drained – reset
        _bufLen = 0;
        _bufPos = 0;
    }
    return toCopy;
}

// --------------------------------------------------------------------------
// generateFirstPart
// Reads the next dataset from flash and formats the first half of the JSON
// object into _internalBuf.
// Returns false when there is no more valid data to emit.
// --------------------------------------------------------------------------
bool JSONChunkedResponse::generateFirstPart() {

    // Check sample counter limit
    if (_j >= CHUNK_SIZE) return false;

    // Check flash address
    if (base_address_json >= FLASH_SIZE_bytes) return false;

    // Read from flash
    flash_read_dataset(base_address_json);

    // 0xFF in the hour field means the slot is empty – we're done
    if (int_hour_from_flash == 255) return false;

    // Advance flash pointer regardless of whether this entry is valid
    base_address_json += dataset_size;

    // Skip entries with invalid timestamps (but keep going)
    if (int_hour_from_flash   > 23 || int_hour_from_flash   < 0 ||
        int_minute_from_flash > 59 || int_minute_from_flash < 0)
    {
        advance();
        // Return true so the caller loops and tries the next entry,
        // but _bufLen remains 0 so the outer while() keeps iterating.
        return true;
    }

    // Build the comma/newline prefix
    const char *prefix = _firstSample ? "    " : ",\n    ";
    _firstSample = false;

    int len = snprintf(_line, sizeof(_line),
        "%s{\n"
        "      \"time\": \"%02d:%02d\",\n"
        "      \"current_power\": %s,\n"
        "      \"total_energy\": %s,\n"
        "      \"daily_energy\": %s,\n"
        "      \"string1_voltage\": %s,\n"
        "      \"string1_current\": %s,\n"
        "      \"string2_voltage\": %s,\n"
        "      \"string2_current\": %s,\n",
        prefix,
        int_hour_from_flash,
        int_minute_from_flash,
        chr_current_power_from_flash,
        chr_total_energy_from_flash,
        chr_daily_energy_from_flash,
        chr_string1_voltage_from_flash,
        chr_string1_current_from_flash,
        chr_string2_voltage_from_flash,
        chr_string2_current_from_flash);

    if (len < 0 || (size_t)len >= sizeof(_line)) return false;

    return appendToInternalBuf(_line, len);
}

// --------------------------------------------------------------------------
// generateSecondPart
// Formats the second half of the current JSON sample into _internalBuf.
// Must only be called after a successful generateFirstPart().
// Returns false on formatting error.
// --------------------------------------------------------------------------
bool JSONChunkedResponse::generateSecondPart() {

    int len = snprintf(_line, sizeof(_line),
        "      \"string3_voltage\": %s,\n"
        "      \"string3_current\": %s,\n"
        "      \"l1_voltage\": %s,\n"
        "      \"L1_power\": %s,\n"
        "      \"l2_voltage\": %s,\n"
        "      \"L2_power\": %s,\n"
        "      \"l3_voltage\": %s,\n"
        "      \"L3_power\": %s\n"
        "    }",
        chr_string3_voltage_from_flash,
        chr_string3_current_from_flash,
        chr_l1_voltage_from_flash,
        chr_l1_power_from_flash,
        chr_l2_voltage_from_flash,
        chr_l2_power_from_flash,
        chr_l3_voltage_from_flash,
        chr_l3_power_from_flash);

    if (len < 0 || (size_t)len >= sizeof(_line)) return false;

    return appendToInternalBuf(_line, len);
}

// --------------------------------------------------------------------------
// advance – increment sub-chunk and sample counters
// --------------------------------------------------------------------------
void JSONChunkedResponse::advance() {
    _i++;
    if (_i >= NUMBER_OF_CHUNKES) {
        _i = 0;
        _j++;
    }
}
