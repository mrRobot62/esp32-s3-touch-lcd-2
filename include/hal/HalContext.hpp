#pragma once

#include <IPAddress.h>
#include <WiFiUdp.h>

#include <vector>

namespace hal {

/**
 * @brief Entry point for low-level hardware services used by the firmware.
 *
 * The class is intentionally small in the first implementation step. Future
 * revisions can extend it with GPIO, I2C, Wi-Fi, Bluetooth, and similar board
 * services while keeping a clear separation from the UI layer.
 */
class HalContext {
  public:
    /**
     * @brief Snapshot of one QMI8658 IMU measurement.
     */
    struct ImuSample {
        /** @brief Acceleration on the X axis in g. */
        float accel_x_g = 0.0f;
        /** @brief Acceleration on the Y axis in g. */
        float accel_y_g = 0.0f;
        /** @brief Acceleration on the Z axis in g. */
        float accel_z_g = 0.0f;
        /** @brief Angular rate on the X axis in degrees per second. */
        float gyro_x_dps = 0.0f;
        /** @brief Angular rate on the Y axis in degrees per second. */
        float gyro_y_dps = 0.0f;
        /** @brief Angular rate on the Z axis in degrees per second. */
        float gyro_z_dps = 0.0f;
    };

    /**
     * @brief Initialize all low-level peripherals needed by the application.
     *
     * The method initializes I2C first, performs an initial bus scan, then
     * configures the onboard IMU, and finally starts Wi-Fi and UDP services in
     * a deterministic order.
     */
    void begin();

    /**
     * @brief Initialize the primary I2C bus used by the board peripherals.
     *
     * This method prepares the bus pins used by the touch controller and any
     * future I2C peripherals managed through the HAL layer.
     */
    void setupI2c();

    /**
     * @brief Scan the I2C bus and cache all responding device addresses.
     *
     * @return List of 7-bit I2C addresses that acknowledged on the current bus scan.
     */
    std::vector<uint8_t> scanI2cDevices();

    /**
     * @brief Initialize the onboard QMI8658 accelerometer and gyroscope.
     *
     * The method probes the supported I2C addresses, validates the device ID,
     * and configures a practical demo profile for accelerometer and gyroscope
     * streaming.
     *
     * @return `true` if the IMU was detected and configured successfully.
     */
    bool setupImu();

    /**
     * @brief Read one fresh IMU sample from the configured QMI8658.
     *
     * @param sample Output structure that receives the converted sensor values.
     *
     * @return `true` if a complete IMU sample was read successfully.
     */
    bool readImuSample(ImuSample& sample);

    /**
     * @brief Configure the Wi-Fi station connection from the local secrets file.
     *
     * The method skips the connection attempt when placeholder credentials are
     * still in use so the firmware remains safe to flash on a fresh checkout.
     *
     * @return `true` if the station connected successfully, otherwise `false`.
     */
    bool setupWifi();

    /**
     * @brief Initialize the UDP client endpoint using the local UDP configuration.
     *
     * The method opens the local UDP socket and stores the remote server
     * endpoint for future telemetry or log transmission.
     *
     * @return `true` if the UDP socket was opened successfully, otherwise `false`.
     */
    bool setupUdp();

    /**
     * @brief Return the list of cached I2C device addresses from the last scan.
     *
     * @return Constant reference to the last scan result.
     */
    const std::vector<uint8_t>& i2cDeviceAddresses() const;

    /**
     * @brief Report whether the onboard IMU is ready for sampling.
     *
     * @return `true` when the QMI8658 was configured successfully.
     */
    bool isImuReady() const;

    /**
     * @brief Return the I2C address currently used for the onboard IMU.
     *
     * @return 7-bit I2C address of the configured IMU, or `0` when unavailable.
     */
    uint8_t imuAddress() const;

    /**
     * @brief Report whether Wi-Fi is currently connected.
     *
     * @return `true` when the station has an active connection, otherwise `false`.
     */
    bool isWifiConnected() const;

    /**
     * @brief Return the configured remote UDP server address.
     *
     * @return Target IPv4 address used for outgoing UDP traffic.
     */
    IPAddress udpServerIp() const;

    /**
     * @brief Return the configured remote UDP server port.
     *
     * @return Target UDP port used for outgoing traffic.
     */
    uint16_t udpServerPort() const;

    /**
     * @brief Provide access to the shared UDP socket instance.
     *
     * @return Reference to the internally managed UDP socket.
     */
    WiFiUDP& udp();

  private:
    /**
     * @brief Last filtered IMU sample used for low-pass stabilization.
     *
     * The HAL smooths the raw QMI8658 data before it reaches the UI. Storing
     * the previous filtered sample here keeps the filter state local to the
     * hardware layer and avoids duplicated smoothing logic in every consumer.
     */
    ImuSample filtered_imu_sample_;
    /**
     * @brief Track whether the IMU filter state already contains a valid sample.
     *
     * The first successful measurement is forwarded without blending so the
     * UI receives immediate values after entering an accelerometer screen.
     */
    bool has_filtered_imu_sample_ = false;
    std::vector<uint8_t> i2c_device_addresses_;
    uint8_t imu_address_ = 0;
    bool imu_ready_ = false;
    WiFiUDP udp_;
    IPAddress udp_server_ip_;
    uint16_t udp_server_port_ = 0;
    bool wifi_connected_ = false;
    bool udp_ready_ = false;
};

} // namespace hal
