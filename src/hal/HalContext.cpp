#include "hal/HalContext.hpp"

#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>

#include "udp_config.h"
#include "wifi_secrets.h"

namespace {

constexpr uint8_t kI2cSdaPin = 48;
constexpr uint8_t kI2cSclPin = 47;
constexpr unsigned long kWifiConnectTimeoutMs = 10000UL;

/**
 * @brief Check whether the configured Wi-Fi credentials are still placeholders.
 *
 * @param ssid SSID string loaded from the local secrets header.
 * @param password Password string loaded from the local secrets header.
 *
 * @return `true` when the credentials still use placeholder values.
 */
bool hasPlaceholderWifiCredentials(const char* ssid, const char* password) {
    return strcmp(ssid, "YOUR_WIFI_SSID") == 0 || strcmp(password, "YOUR_WIFI_PASSWORD") == 0;
}

} // namespace

namespace hal {

/**
 * @brief Initialize all low-level board services used by the current firmware.
 *
 * The startup order is intentional: I2C first for onboard peripherals, then a
 * scan to expose the current bus state, followed by network services that may
 * depend on runtime configuration or later diagnostics output.
 */
void HalContext::begin() {
    setupI2c();
    scanI2cDevices();
    wifi_connected_ = setupWifi();
    setupUdp();
}

/**
 * @brief Initialize the primary I2C bus with the board-specific pin mapping.
 *
 * This method centralizes the I2C pin selection so a future hardware revision
 * only needs one update inside the HAL instead of multiple call sites.
 */
void HalContext::setupI2c() { Wire.begin(kI2cSdaPin, kI2cSclPin); }

/**
 * @brief Scan the complete 7-bit I2C address space for responding devices.
 *
 * The result is cached internally and also returned to the caller so later
 * modules can inspect the detected devices without triggering another scan.
 *
 * @return List of discovered 7-bit I2C device addresses.
 */
std::vector<uint8_t> HalContext::scanI2cDevices() {
    i2c_device_addresses_.clear();

    for (uint8_t address = 0x08; address <= 0x77; ++address) {
        Wire.beginTransmission(address);
        const uint8_t error = Wire.endTransmission();
        if (error == 0) {
            i2c_device_addresses_.push_back(address);
        }
    }

    return i2c_device_addresses_;
}

/**
 * @brief Connect the ESP32-S3 to the configured Wi-Fi network.
 *
 * The method reads credentials from `wifi_secrets.h`. If the header still
 * contains placeholder values, the connection is skipped intentionally so a
 * fresh clone remains compile- and flash-ready without exposing credentials.
 *
 * @return `true` if Wi-Fi connected successfully, otherwise `false`.
 */
bool HalContext::setupWifi() {
    if (hasPlaceholderWifiCredentials(WIFI_SSID, WIFI_PASSWORD)) {
        Serial.println("WiFi setup skipped because placeholder credentials are still configured.");
        WiFi.disconnect(true, true);
        WiFi.mode(WIFI_OFF);
        return false;
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    const unsigned long connect_started_at = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - connect_started_at) < kWifiConnectTimeoutMs) {
        delay(250);
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("WiFi connected. Local IP: %s\n", WiFi.localIP().toString().c_str());
        return true;
    }

    Serial.println("WiFi connection failed.");
    WiFi.disconnect(true, true);
    WiFi.mode(WIFI_OFF);
    return false;
}

/**
 * @brief Open the UDP socket and store the configured remote server endpoint.
 *
 * The local socket can be opened even without Wi-Fi, which keeps the firmware
 * setup deterministic. Actual network traffic will still depend on a working
 * Wi-Fi connection when future modules start sending packets.
 *
 * @return `true` if the local UDP socket was opened successfully, otherwise `false`.
 */
bool HalContext::setupUdp() {
    udp_server_ip_ = udp_config::kServerIp;
    udp_server_port_ = udp_config::kServerPort;

    const bool begin_ok = udp_.begin(udp_config::kLocalPort);
    if (begin_ok) {
        Serial.printf(
            "UDP ready. Local port: %u, remote target: %s:%u\n",
            udp_config::kLocalPort,
            udp_server_ip_.toString().c_str(),
            udp_server_port_);
    } else {
        Serial.println("UDP setup failed.");
    }

    return begin_ok;
}

/**
 * @brief Return the cached I2C scan result from the most recent bus scan.
 *
 * @return Constant reference to the list of detected I2C device addresses.
 */
const std::vector<uint8_t>& HalContext::i2cDeviceAddresses() const { return i2c_device_addresses_; }

/**
 * @brief Report the current Wi-Fi connection state cached by the HAL.
 *
 * @return `true` when Wi-Fi was connected during setup, otherwise `false`.
 */
bool HalContext::isWifiConnected() const { return wifi_connected_; }

/**
 * @brief Return the configured remote UDP server IP address.
 *
 * @return Target IPv4 address used for future UDP transmissions.
 */
IPAddress HalContext::udpServerIp() const { return udp_server_ip_; }

/**
 * @brief Return the configured remote UDP server port.
 *
 * @return Target UDP port used for future UDP transmissions.
 */
uint16_t HalContext::udpServerPort() const { return udp_server_port_; }

/**
 * @brief Expose the shared UDP socket for future modules.
 *
 * @return Reference to the internally managed `WiFiUDP` instance.
 */
WiFiUDP& HalContext::udp() { return udp_; }

} // namespace hal
