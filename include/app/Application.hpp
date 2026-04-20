#pragma once

namespace app {

/**
 * @brief High-level application coordinator for the demo firmware.
 *
 * The application is responsible for bringing up the hardware abstraction layer,
 * initializing the display stack, and forwarding the Arduino runtime loop into
 * the currently active demo module.
 */
class Application {
  public:
    /**
     * @brief Perform one-time initialization for the firmware.
     *
     * This method prepares the HAL services, configures the display, and starts
     * the currently selected UI demo sequence.
     */
    void setup();

    /**
     * @brief Execute one iteration of the non-blocking application loop.
     *
     * This method is called repeatedly by the Arduino runtime and advances the
     * active demo without reinitializing any hardware.
     */
    void loop();
};

} // namespace app
