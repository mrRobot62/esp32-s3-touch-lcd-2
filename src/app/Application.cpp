#include "app/Application.hpp"

#include <Arduino.h>

#include "display/BoardDisplay.hpp"
#include "hal/HalContext.hpp"
#include "modules/SystemInfo.hpp"
#include "ui/UiDemo.hpp"

namespace {

/**
 * @brief Print the currently detected I2C devices to the serial console.
 *
 * @param addresses List of 7-bit I2C addresses returned by the HAL scan.
 */
void logI2cDevices(const std::vector<uint8_t>& addresses) {
    if (addresses.empty()) {
        Serial.println("I2C scan found no devices.");
        return;
    }

    Serial.printf("I2C scan found %u device(s): ", static_cast<unsigned>(addresses.size()));
    for (size_t index = 0; index < addresses.size(); ++index) {
        Serial.printf("0x%02X%s", addresses[index], (index + 1U < addresses.size()) ? ", " : "\n");
    }
}

/** @brief Shared low-level hardware context for the complete firmware lifetime. */
hal::HalContext hal_context;
/** @brief Shared board display instance reused by all demo screens. */
display::BoardDisplay board_display;
/** @brief Active demo presenter that renders the current validation screens. */
ui::UiDemo ui_demo(board_display);

} // namespace

namespace app {

/**
 * @brief Bring up the hardware and start the initial demo sequence.
 *
 * The method configures serial logging, initializes the display hardware,
 * applies a default screen orientation and brightness, and finally starts
 * the UI demo. Adjust this sequence when more modules need explicit startup.
 */
void Application::setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println();
    Serial.printf("%s booting...\n", modules::SystemInfo::firmwareName());

    hal_context.begin();
    logI2cDevices(hal_context.i2cDeviceAddresses());
    Serial.printf("WiFi connected: %s\n", hal_context.isWifiConnected() ? "yes" : "no");
    Serial.printf(
        "UDP target configured as %s:%u\n",
        hal_context.udpServerIp().toString().c_str(),
        hal_context.udpServerPort());

    board_display.init();
    board_display.setRotation(1);
    board_display.setBrightness(200);

    ui_demo.begin();
}

/**
 * @brief Forward the Arduino main loop into the active UI demo.
 *
 * Keeping the loop thin makes it easy to add task scheduling, touch polling,
 * or background services later without rewriting the Arduino entry points.
 */
void Application::loop() { ui_demo.update(); }

} // namespace app
