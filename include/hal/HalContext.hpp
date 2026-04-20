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
     * @brief Initialize all low-level peripherals needed by the application.
     *
     * The method initializes I2C first, performs an initial bus scan, then
     * configures Wi-Fi and UDP services in a deterministic order.
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
    std::vector<uint8_t> i2c_device_addresses_;
    WiFiUDP udp_;
    IPAddress udp_server_ip_;
    uint16_t udp_server_port_ = 0;
    bool wifi_connected_ = false;
};

} // namespace hal
