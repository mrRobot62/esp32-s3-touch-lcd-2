#pragma once

#include <stddef.h>
#include <stdint.h>

// UDP logging is controlled by ONE flag in platformio.ini:
//   -DWIFI_LOGGING_ENABLE=1
//
// If disabled, all functions become no-ops and compile out.

namespace udp {

// runtime enable state (read-only API)
bool is_enabled();

// configure target (ip/port) used for UDP packets
void configure(const char *targetIp, uint16_t targetPort);

// start WiFi + UDP logging (roleLabel should be "HOST" or "CLIENT")
bool begin(const char *roleLabel);

// stop UDP logging (optional)
void end();

// send raw bytes (returns true if queued/sent)
bool send_bytes(const uint8_t *data, size_t len);

// convenience: send C-string (inline implementation must exist in header)
inline bool send_cstr(const char *s) {
    if (!s) {
        return false;
    }
    // strlen without pulling <cstring> into every TU
    size_t n = 0;
    while (s[n] != '\0') {
        ++n;
    }
    return send_bytes(reinterpret_cast<const uint8_t *>(s), n);
}

// optional diagnostics (safe to call even if disabled)
void diag_print();

} // namespace udp