#pragma once

#include "ESPAsyncWebServer.h"

// =============================================================================
// JSON CHUNKED RESPONSE CLASS
// Streams flash data as a JSON file over HTTP chunked transfer encoding.
//
// Uses an internal buffer (INTERNAL_BUF_SIZE) so that a single JSON sample
// can always be formatted in full, regardless of how small the buffer that
// ESPAsyncWebServer hands to _fillBuffer() happens to be.
// _fillBuffer() drains the internal buffer into ESPAsync's buffer across
// as many calls as needed before generating the next piece of data.
// =============================================================================

class JSONChunkedResponse : public AsyncAbstractResponse {

public:
    explicit JSONChunkedResponse(int jsonIndex);

    bool   _sourceValid() const override;
    size_t _fillBuffer(uint8_t *buf, size_t maxLen) override;

private:
    // -------------------------------------------------------------------------
    // State machine
    // -------------------------------------------------------------------------
    enum State : uint8_t {
        STATE_START,        // Write opening brace
        STATE_METADATA,     // Write date / format_version fields
        STATE_DATA_START,   // Write start of samples array
        STATE_DATA,         // Write individual samples (bulk of the work)
        STATE_DATA_END,     // Write closing bracket of samples array
        STATE_END,          // Write total_samples + closing brace
        STATE_ERROR,        // Write error JSON and finish
        STATE_DONE          // Signal EOF to ESPAsync (return 0)
    };

    // -------------------------------------------------------------------------
    // Internal double-buffer
    // Sized to comfortably hold the largest possible single sample (both
    // first_part and second_part combined) with room to spare.
    // Increase if you add more fields.
    // -------------------------------------------------------------------------
    static constexpr size_t INTERNAL_BUF_SIZE = 4096;
    char   _internalBuf[INTERNAL_BUF_SIZE];
    size_t _bufLen;     // valid bytes currently in _internalBuf
    size_t _bufPos;     // how many bytes have already been drained

    // -------------------------------------------------------------------------
    // Per-sample generation state
    // -------------------------------------------------------------------------
    int        _jsonIndex;
    State      _state;
    int        _i;          // sub-chunk counter (0 .. NUMBER_OF_CHUNKES-1)
    int        _j;          // sample counter
    char       _line[1024]; // scratch pad for snprintf
    bool       _error;
    const char *_errorMessage;
    bool       _firstSample;
    bool       _pendingSecondPart; // true when first_part written, waiting for second_part

    // -------------------------------------------------------------------------
    // Helpers
    // -------------------------------------------------------------------------

    // Write error state and set HTTP 500
    void   setError(const char *message);

    // Append 'line' to _internalBuf; returns false on overflow
    bool   appendToInternalBuf(const char *line, int len);

    // Copy as many bytes as possible from _internalBuf into ESPAsync's buf.
    // Returns bytes copied; resets _bufLen/_bufPos when fully drained.
    size_t drainInternalBuf(uint8_t *buf, size_t maxLen);

    // Generate the first half of a sample record into _internalBuf.
    // Reads from flash, advances base_address_json.
    // Returns false when there is no more data (EOF or invalid address).
    bool   generateFirstPart();

    // Generate the second half of a sample record into _internalBuf.
    // Returns false on formatting error.
    bool   generateSecondPart();

    // Increment _i / _j counters
    void   advance();
};
