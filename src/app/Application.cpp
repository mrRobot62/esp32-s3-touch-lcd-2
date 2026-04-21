#include "app/Application.hpp"

#include <Arduino.h>

#include "display/BoardDisplay.hpp"
#include "hal/HalContext.hpp"
#include "log_core.h"
#include "modules/SystemInfo.hpp"
#include "ui/UiDemo.hpp"

namespace {

/** @brief Maximum time in milliseconds to wait for the USB serial monitor to attach. */
constexpr unsigned long kSerialAttachTimeoutMs = 2500UL;
/** @brief Periodic application heartbeat interval in milliseconds. */
constexpr unsigned long kHeartbeatIntervalMs = 3000UL;

/**
 * @brief Give the USB CDC monitor a short time window to attach before early logs are sent.
 *
 * On ESP32-S3 native USB setups the first boot messages can be lost when the host
 * terminal opens slightly after reset. This wait is bounded so boot stays fast.
 */
void waitForSerialMonitor() {
    const unsigned long wait_started_at = millis();
    while (!Serial && (millis() - wait_started_at) < kSerialAttachTimeoutMs) {
        delay(10);
    }
}

/**
 * @brief Print the currently detected I2C devices to the serial console.
 *
 * @param addresses List of 7-bit I2C addresses returned by the HAL scan.
 */
void logI2cDevices(const std::vector<uint8_t> &addresses) {
    if (addresses.empty()) {
        WARN_TAG("APP", "I2C scan found no devices.\n");
        return;
    }

    INFO_TAG("APP", "I2C scan found %u device(s): ", static_cast<unsigned>(addresses.size()));
    for (size_t index = 0; index < addresses.size(); ++index) {
        RAW("0x%02X%s", addresses[index], (index + 1U < addresses.size()) ? ", " : "\n");
    }
}

/**
 * @brief Show a short full-screen color sequence to validate basic panel output.
 *
 * @param display Reference to the initialized display driver.
 */
void runDisplaySmokeTest(display::BoardDisplay &display) {
    INFO_TAG("APP", "Running display smoke test: RED.\n");
    display.fillScreen(TFT_RED);
    delay(250);
    INFO_TAG("APP", "Running display smoke test: GREEN.\n");
    display.fillScreen(TFT_GREEN);
    delay(250);
    INFO_TAG("APP", "Running display smoke test: BLUE.\n");
    display.fillScreen(TFT_BLUE);
    delay(250);
    INFO_TAG("APP", "Running display smoke test: WHITE.\n");
    display.fillScreen(TFT_WHITE);
    delay(250);
    INFO_TAG("APP", "Running display smoke test: BLACK.\n");
    display.fillScreen(TFT_BLACK);
}

/** @brief Shared low-level hardware context for the complete firmware lifetime. */
hal::HalContext hal_context;
/** @brief Shared board display instance reused by all demo screens. */
display::BoardDisplay board_display;
/** @brief Active demo presenter that renders the current validation screens. */
ui::UiDemo ui_demo(board_display);
/** @brief Timestamp of the last runtime heartbeat log. */
unsigned long last_heartbeat_at = 0;

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
    waitForSerialMonitor();
    delay(200);
    RAW("\n");
    RAW("=== serial online ===\n");
    INFO_TAG("APP", "%s booting.\n", modules::SystemInfo::firmwareName());

    board_display.forceBacklightOn();
    INFO_TAG("APP", "Display backlight forced on for bring-up diagnostics.\n");

    hal_context.begin();
    logI2cDevices(hal_context.i2cDeviceAddresses());
    INFO_TAG("APP", "WiFi connected: %s\n", hal_context.isWifiConnected() ? "yes" : "no");
    INFO_TAG(
        "APP",
        "UDP target configured as %s:%u\n",
        hal_context.udpServerIp().toString().c_str(),
        hal_context.udpServerPort());

    INFO_TAG("APP", "Initializing display controller.\n");
    board_display.init();
    INFO_TAG("APP", "Display controller init returned.\n");

    board_display.setRotation(1);
    INFO_TAG("APP", "Display rotation set to 1.\n");

    board_display.setBrightness(255);
    INFO_TAG("APP", "Display brightness set to maximum.\n");
    INFO_TAG("APP", "Display controller initialized, starting smoke test.\n");

    runDisplaySmokeTest(board_display);

    INFO_TAG("APP", "Starting UI demo.\n");
    ui_demo.begin();
    INFO_TAG("APP", "UI demo initialized.\n");
    last_heartbeat_at = millis();
}

/**
 * @brief Forward the Arduino main loop into the active UI demo.
 *
 * Keeping the loop thin makes it easy to add task scheduling, touch polling,
 * or background services later without rewriting the Arduino entry points.
 */
void Application::loop() {
    ui_demo.update();

    const unsigned long now = millis();
    if ((now - last_heartbeat_at) >= kHeartbeatIntervalMs) {
        INFO_TAG("APP", "heartbeat uptime_ms=%lu current_demo_running=yes\n", now);
        last_heartbeat_at = now;
    }
}

} // namespace app
