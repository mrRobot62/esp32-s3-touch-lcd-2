#include "hal/HalContext.hpp"

#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>

#include "log_core.h"
#include "udp_config.h"
#include "wifi_secrets.h"

namespace {

constexpr uint8_t kI2cSdaPin = 48;
constexpr uint8_t kI2cSclPin = 47;
constexpr uint32_t kI2cFrequencyHz = 400000UL;
constexpr unsigned long kWifiConnectTimeoutMs = 10000UL;
constexpr uint8_t kQmi8658WhoAmIRegister = 0x00;
constexpr uint8_t kQmi8658RevisionRegister = 0x01;
constexpr uint8_t kQmi8658Ctrl1Register = 0x02;
constexpr uint8_t kQmi8658Ctrl2Register = 0x03;
constexpr uint8_t kQmi8658Ctrl3Register = 0x04;
constexpr uint8_t kQmi8658Ctrl7Register = 0x08;
constexpr uint8_t kQmi8658Status0Register = 0x2E;
constexpr uint8_t kQmi8658AccelOutputRegister = 0x35;
constexpr uint8_t kQmi8658WhoAmIValue = 0x05;
constexpr uint8_t kQmi8658AddressLow = 0x6A;
constexpr uint8_t kQmi8658AddressHigh = 0x6B;
constexpr uint8_t kQmi8658Ctrl1Value = 0x60;
constexpr uint8_t kQmi8658Ctrl2Value = 0x23;
constexpr uint8_t kQmi8658Ctrl3Value = 0x43;
constexpr uint8_t kQmi8658Ctrl7Value = 0x03;
constexpr uint8_t kQmi8658DataReadyMask = 0x03;
constexpr uint8_t kQmi8658BootstrapReadAttempts = 8;
constexpr uint8_t kQmi8658ReadAttempts = 3;
constexpr unsigned long kQmi8658BootstrapDelayMs = 10UL;
constexpr unsigned long kQmi8658ReadRetryDelayMs = 2UL;
constexpr float kQmi8658AccelScale8g = 1.0f / 4096.0f;
constexpr float kQmi8658GyroScale512dps = 1.0f / 64.0f;
constexpr float kImuAccelLowPassAlpha = 0.18f;
constexpr float kImuGyroLowPassAlpha = 0.12f;
constexpr float kImuAccelDeadbandG = 0.018f;
constexpr float kImuGyroDeadbandDps = 0.8f;
constexpr float kImuGravityReferenceG = 1.0f;

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

/**
 * @brief Read one 8-bit register from an I2C peripheral.
 *
 * @param device_address 7-bit I2C address of the target device.
 * @param register_address Register to read from.
 * @param value Output byte that receives the register content.
 *
 * @return `true` if the register read completed successfully.
 */
bool readRegister8(TwoWire& wire, uint8_t device_address, uint8_t register_address, uint8_t& value) {
    wire.beginTransmission(device_address);
    wire.write(register_address);
    if (wire.endTransmission(false) != 0) {
        return false;
    }

    if (wire.requestFrom(device_address, static_cast<uint8_t>(1)) != 1) {
        return false;
    }

    value = wire.read();
    return true;
}

/**
 * @brief Read a consecutive block of bytes from an I2C peripheral.
 *
 * @param device_address 7-bit I2C address of the target device.
 * @param start_register First register address of the block read.
 * @param data Pointer to the output buffer.
 * @param length Number of bytes to read.
 *
 * @return `true` if the complete block was transferred successfully.
 */
bool readRegisters(TwoWire& wire, uint8_t device_address, uint8_t start_register, uint8_t* data, size_t length) {
    wire.beginTransmission(device_address);
    wire.write(start_register);
    if (wire.endTransmission(false) != 0) {
        return false;
    }

    const uint8_t received = wire.requestFrom(device_address, static_cast<uint8_t>(length));
    if (received != length) {
        return false;
    }

    for (size_t index = 0; index < length; ++index) {
        data[index] = wire.read();
    }
    return true;
}

/**
 * @brief Write one 8-bit register of an I2C peripheral.
 *
 * @param device_address 7-bit I2C address of the target device.
 * @param register_address Register to write.
 * @param value Value written into the register.
 *
 * @return `true` if the write transaction completed successfully.
 */
bool writeRegister8(TwoWire& wire, uint8_t device_address, uint8_t register_address, uint8_t value) {
    wire.beginTransmission(device_address);
    wire.write(register_address);
    wire.write(value);
    return wire.endTransmission() == 0;
}

/**
 * @brief Convert two adjacent little-endian bytes into a signed 16-bit integer.
 *
 * @param low Lower byte of the value.
 * @param high Upper byte of the value.
 *
 * @return Signed 16-bit measurement value.
 */
int16_t combineSigned16(uint8_t low, uint8_t high) {
    return static_cast<int16_t>((static_cast<uint16_t>(high) << 8U) | static_cast<uint16_t>(low));
}

/**
 * @brief Read the QMI8658 status register and report whether fresh accel/gyro data is ready.
 *
 * @param device_address 7-bit I2C address of the QMI8658.
 *
 * @return `true` when either accelerometer or gyroscope data is marked ready.
 */
bool isImuDataReady(uint8_t device_address) {
    uint8_t status = 0;
    if (!readRegister8(Wire, device_address, kQmi8658Status0Register, status)) {
        return false;
    }

    return (status & kQmi8658DataReadyMask) != 0;
}

/**
 * @brief Blend two sample values with a simple first-order low-pass filter.
 *
 * @param previous Previously filtered value.
 * @param current Newly measured raw value.
 * @param alpha Weight of the current value in the range `0.0 .. 1.0`.
 *
 * @return Filtered value between the previous and current sample.
 */
float lowPassValue(float previous, float current, float alpha) {
    return previous + ((current - previous) * alpha);
}

/**
 * @brief Clamp tiny signal variations around a reference point to zero motion.
 *
 * @param value Current filtered value.
 * @param reference Resting reference around which noise should be suppressed.
 * @param threshold Minimum delta from the reference before motion is reported.
 *
 * @return The reference value when the delta is below the threshold, otherwise the original value.
 */
float applyDeadband(float value, float reference, float threshold) {
    return fabsf(value - reference) < threshold ? reference : value;
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
    INFO_TAG("HAL", "Starting HAL initialization.\n");
    setupI2c();
    scanI2cDevices();
    imu_ready_ = setupImu();
    wifi_connected_ = setupWifi();
    udp_ready_ = setupUdp();
    INFO_TAG("HAL", "HAL initialization finished.\n");
}

/**
 * @brief Initialize the primary I2C bus with the board-specific pin mapping.
 *
 * This method centralizes the I2C pin selection so a future hardware revision
 * only needs one update inside the HAL instead of multiple call sites.
 */
void HalContext::setupI2c() {
    INFO_TAG("HAL", "Initializing I2C on SDA=%u, SCL=%u.\n", kI2cSdaPin, kI2cSclPin);
    Wire.begin(kI2cSdaPin, kI2cSclPin, kI2cFrequencyHz);
    Wire.setTimeOut(20);
}

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
    INFO_TAG("HAL", "Scanning I2C bus for responding devices.\n");

    for (uint8_t address = 0x08; address <= 0x77; ++address) {
        Wire.beginTransmission(address);
        const uint8_t error = Wire.endTransmission();
        if (error == 0) {
            i2c_device_addresses_.push_back(address);
            INFO_TAG("HAL", "I2C device acknowledged at 0x%02X.\n", address);
        }
    }

    INFO_TAG("HAL", "I2C scan complete, found %u device(s).\n", static_cast<unsigned>(i2c_device_addresses_.size()));

    return i2c_device_addresses_;
}

/**
 * @brief Probe and configure the onboard QMI8658 IMU for demo usage.
 *
 * The implementation currently uses a practical configuration for UI demos:
 * accelerometer at ±2 g / 125 Hz and gyroscope at ±512 dps / 112 Hz.
 *
 * @return `true` if the IMU was detected and configured successfully.
 */
bool HalContext::setupImu() {
    imu_address_ = 0;
    has_filtered_imu_sample_ = false;
    filtered_imu_sample_ = {};
    INFO_TAG(
        "HAL",
        "Probing QMI8658 on the shared touch/IMU I2C bus SDA=%u, SCL=%u.\n",
        kI2cSdaPin,
        kI2cSclPin);

    const uint8_t candidate_addresses[] = {kQmi8658AddressLow, kQmi8658AddressHigh};
    for (const uint8_t candidate_address : candidate_addresses) {
        uint8_t who_am_i = 0;
        if (!readRegister8(Wire, candidate_address, kQmi8658WhoAmIRegister, who_am_i)) {
            continue;
        }

        uint8_t revision = 0;
        const bool has_revision = readRegister8(Wire, candidate_address, kQmi8658RevisionRegister, revision);
        if (has_revision) {
            INFO_TAG(
                "HAL",
                "Shared-bus IMU probe at 0x%02X returned WHO_AM_I=0x%02X, REV=0x%02X.\n",
                candidate_address,
                who_am_i,
                revision);
        } else {
            INFO_TAG("HAL", "Shared-bus IMU probe at 0x%02X returned WHO_AM_I=0x%02X.\n", candidate_address, who_am_i);
        }

        if (who_am_i != kQmi8658WhoAmIValue) {
            WARN_TAG(
                "HAL",
                "QMI8658 probe on the shared bus at 0x%02X responded with unexpected WHO_AM_I=0x%02X.\n",
                candidate_address,
                who_am_i);
            continue;
        }

        imu_address_ = candidate_address;
        break;
    }

    if (imu_address_ == 0) {
        WARN_TAG("HAL", "QMI8658 IMU not detected on the shared touch/IMU I2C bus.\n");
        return false;
    }

    INFO_TAG(
        "HAL",
        "Configuring QMI8658 IMU at I2C address 0x%02X on shared SDA=%u, SCL=%u.\n",
        imu_address_,
        kI2cSdaPin,
        kI2cSclPin);

    if (!writeRegister8(Wire, imu_address_, kQmi8658Ctrl1Register, kQmi8658Ctrl1Value)) {
        ERR_TAG("HAL", "Failed to configure QMI8658 base control settings.\n");
        return false;
    }

    if (!writeRegister8(Wire, imu_address_, kQmi8658Ctrl2Register, kQmi8658Ctrl2Value)) {
        ERR_TAG("HAL", "Failed to configure QMI8658 accelerometer settings.\n");
        return false;
    }

    if (!writeRegister8(Wire, imu_address_, kQmi8658Ctrl3Register, kQmi8658Ctrl3Value)) {
        ERR_TAG("HAL", "Failed to configure QMI8658 gyroscope settings.\n");
        return false;
    }

    if (!writeRegister8(Wire, imu_address_, kQmi8658Ctrl7Register, kQmi8658Ctrl7Value)) {
        ERR_TAG("HAL", "Failed to enable QMI8658 accelerometer and gyroscope.\n");
        return false;
    }

    delay(kQmi8658BootstrapDelayMs);

    for (uint8_t attempt = 0; attempt < kQmi8658BootstrapReadAttempts; ++attempt) {
        uint8_t bootstrap_data[12] = {};
        if (isImuDataReady(imu_address_) &&
            readRegisters(Wire, imu_address_, kQmi8658AccelOutputRegister, bootstrap_data, sizeof(bootstrap_data))) {
            const int16_t accel_z_raw = combineSigned16(bootstrap_data[4], bootstrap_data[5]);
            INFO_TAG(
                "HAL",
                "QMI8658 bootstrap sample became available on attempt %u, accel_z_raw=%d.\n",
                static_cast<unsigned>(attempt + 1U),
                static_cast<int>(accel_z_raw));
            INFO_TAG("HAL", "QMI8658 IMU initialized successfully.\n");
            return true;
        }

        delay(kQmi8658BootstrapDelayMs);
    }

    WARN_TAG(
        "HAL",
        "QMI8658 accepted configuration at 0x%02X but no ready sample was observed during bootstrap.\n",
        imu_address_);
    return true;
}

/**
 * @brief Read and convert one QMI8658 sample into engineering units.
 *
 * @param sample Output structure that receives the converted accelerometer and gyroscope values.
 *
 * @return `true` if the complete sample read was successful.
 */
bool HalContext::readImuSample(ImuSample& sample) {
    if (!imu_ready_ || imu_address_ == 0) {
        return false;
    }

    uint8_t raw_data[12] = {};
    for (uint8_t attempt = 0; attempt < kQmi8658ReadAttempts; ++attempt) {
        const bool data_ready = isImuDataReady(imu_address_);
        if (data_ready &&
            readRegisters(Wire, imu_address_, kQmi8658AccelOutputRegister, raw_data, sizeof(raw_data))) {
            ImuSample raw_sample = {};
            const int16_t accel_x_raw = combineSigned16(raw_data[0], raw_data[1]);
            const int16_t accel_y_raw = combineSigned16(raw_data[2], raw_data[3]);
            const int16_t accel_z_raw = combineSigned16(raw_data[4], raw_data[5]);
            const int16_t gyro_x_raw = combineSigned16(raw_data[6], raw_data[7]);
            const int16_t gyro_y_raw = combineSigned16(raw_data[8], raw_data[9]);
            const int16_t gyro_z_raw = combineSigned16(raw_data[10], raw_data[11]);

            raw_sample.accel_x_g = static_cast<float>(accel_x_raw) * kQmi8658AccelScale8g;
            raw_sample.accel_y_g = static_cast<float>(accel_y_raw) * kQmi8658AccelScale8g;
            raw_sample.accel_z_g = static_cast<float>(accel_z_raw) * kQmi8658AccelScale8g;
            raw_sample.gyro_x_dps = static_cast<float>(gyro_x_raw) * kQmi8658GyroScale512dps;
            raw_sample.gyro_y_dps = static_cast<float>(gyro_y_raw) * kQmi8658GyroScale512dps;
            raw_sample.gyro_z_dps = static_cast<float>(gyro_z_raw) * kQmi8658GyroScale512dps;

            if (!has_filtered_imu_sample_) {
                filtered_imu_sample_ = raw_sample;
                has_filtered_imu_sample_ = true;
            } else {
                filtered_imu_sample_.accel_x_g =
                    lowPassValue(filtered_imu_sample_.accel_x_g, raw_sample.accel_x_g, kImuAccelLowPassAlpha);
                filtered_imu_sample_.accel_y_g =
                    lowPassValue(filtered_imu_sample_.accel_y_g, raw_sample.accel_y_g, kImuAccelLowPassAlpha);
                filtered_imu_sample_.accel_z_g =
                    lowPassValue(filtered_imu_sample_.accel_z_g, raw_sample.accel_z_g, kImuAccelLowPassAlpha);
                filtered_imu_sample_.gyro_x_dps =
                    lowPassValue(filtered_imu_sample_.gyro_x_dps, raw_sample.gyro_x_dps, kImuGyroLowPassAlpha);
                filtered_imu_sample_.gyro_y_dps =
                    lowPassValue(filtered_imu_sample_.gyro_y_dps, raw_sample.gyro_y_dps, kImuGyroLowPassAlpha);
                filtered_imu_sample_.gyro_z_dps =
                    lowPassValue(filtered_imu_sample_.gyro_z_dps, raw_sample.gyro_z_dps, kImuGyroLowPassAlpha);
            }

            filtered_imu_sample_.accel_x_g = applyDeadband(filtered_imu_sample_.accel_x_g, 0.0f, kImuAccelDeadbandG);
            filtered_imu_sample_.accel_y_g = applyDeadband(filtered_imu_sample_.accel_y_g, 0.0f, kImuAccelDeadbandG);
            filtered_imu_sample_.accel_z_g =
                applyDeadband(filtered_imu_sample_.accel_z_g, kImuGravityReferenceG, kImuAccelDeadbandG);
            filtered_imu_sample_.gyro_x_dps = applyDeadband(filtered_imu_sample_.gyro_x_dps, 0.0f, kImuGyroDeadbandDps);
            filtered_imu_sample_.gyro_y_dps = applyDeadband(filtered_imu_sample_.gyro_y_dps, 0.0f, kImuGyroDeadbandDps);
            filtered_imu_sample_.gyro_z_dps = applyDeadband(filtered_imu_sample_.gyro_z_dps, 0.0f, kImuGyroDeadbandDps);

            sample = filtered_imu_sample_;
            return true;
        }

        delay(kQmi8658ReadRetryDelayMs);
    }

    WARN_TAG("HAL", "No fresh QMI8658 sample available after %u read attempt(s).\n", kQmi8658ReadAttempts);
    return false;
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
        WARN_TAG("HAL", "WiFi setup skipped because placeholder credentials are still configured.\n");
        return false;
    }

    INFO_TAG("HAL", "Starting WiFi station mode for SSID '%s'.\n", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    const unsigned long connect_started_at = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - connect_started_at) < kWifiConnectTimeoutMs) {
        INFO_TAG("HAL", "Waiting for WiFi connection, status=%d.\n", static_cast<int>(WiFi.status()));
        delay(250);
    }

    if (WiFi.status() == WL_CONNECTED) {
        INFO_TAG("HAL", "WiFi connected. Local IP: %s\n", WiFi.localIP().toString().c_str());
        return true;
    }

    ERR_TAG("HAL", "WiFi connection failed, final status=%d.\n", static_cast<int>(WiFi.status()));
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

    if (!wifi_connected_) {
        WARN_TAG(
            "HAL",
            "UDP setup skipped because WiFi is not connected. Target remains %s:%u.\n",
            udp_server_ip_.toString().c_str(),
            udp_server_port_);
        return false;
    }

    INFO_TAG(
        "HAL",
        "Opening UDP socket on local port %u with remote target %s:%u.\n",
        udp_config::kLocalPort,
        udp_server_ip_.toString().c_str(),
        udp_server_port_);

    const bool begin_ok = udp_.begin(udp_config::kLocalPort);
    if (begin_ok) {
        INFO_TAG(
            "HAL",
            "UDP ready. Local port: %u, remote target: %s:%u\n",
            udp_config::kLocalPort,
            udp_server_ip_.toString().c_str(),
            udp_server_port_);
    } else {
        ERR_TAG("HAL", "UDP setup failed on local port %u.\n", udp_config::kLocalPort);
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
 * @brief Report whether the QMI8658 completed initialization successfully.
 *
 * @return `true` when the IMU is ready for sampling.
 */
bool HalContext::isImuReady() const { return imu_ready_; }

/**
 * @brief Return the detected I2C address of the onboard QMI8658.
 *
 * @return Configured 7-bit I2C address, or `0` when no IMU is available.
 */
uint8_t HalContext::imuAddress() const { return imu_address_; }

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
