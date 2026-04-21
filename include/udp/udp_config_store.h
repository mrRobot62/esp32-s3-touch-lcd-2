#pragma once

#include <stdbool.h>
#include "udp/udp_config.h"

// Load config from Preferences/NVS.
// Returns true if valid config was found.
bool udp_cfg_load(UdpLogConfig& out);

// Save config to Preferences/NVS.
// Returns true on success.
bool udp_cfg_save(const UdpLogConfig& cfg);

// Clear persisted config.
bool udp_cfg_clear();
