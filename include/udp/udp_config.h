#pragma once

#include <stdint.h>
#include <stdbool.h>

// Runtime configuration for UDP logging.
// This struct is independent of UI and storage.
struct UdpLogConfig
{
    char ssid[33];      // max 32 + null
    char password[65];  // max 64 + null
    char targetIp[16];  // "255.255.255.255" + null
    uint16_t targetPort;

    bool isValid() const;
};

// Apply config to UDP logger runtime.
// This does NOT persist the config.
void udp_config_apply(const UdpLogConfig& cfg);

// Get currently active runtime config.
const UdpLogConfig& udp_config_current();

// Reset runtime config to defaults (not persisted).
void udp_config_reset();
