#pragma once

#include <LovyanGFX.hpp>

namespace display {

/**
 * @brief Hardware-specific display driver for the Waveshare ESP32-S3 Touch LCD 2 board.
 *
 * This class encapsulates the LovyanGFX configuration for the onboard ST7789 display,
 * PWM-controlled backlight, and CST816 touch controller. Keep board-specific pin mapping
 * and controller settings here so higher-level code can stay independent from the wiring.
 */
class BoardDisplay : public lgfx::LGFX_Device {
  public:
    /**
     * @brief Construct and wire together the display, backlight, and touch controllers.
     *
     * The constructor only prepares the LovyanGFX configuration objects. Hardware
     * initialization still happens later when `init()` is called by the application.
     */
    BoardDisplay();

    /**
     * @brief Force the display backlight to a visible state for early bring-up diagnostics.
     *
     * This helper is intentionally separate from `init()` so the application can
     * enable the panel light even before the full display controller starts up.
     */
    void forceBacklightOn();

  private:
    /** @brief LovyanGFX panel instance for the ST7789 controller. */
    lgfx::Panel_ST7789 panel_;
    /** @brief SPI bus definition used by the display controller. */
    lgfx::Bus_SPI bus_;
    /** @brief PWM backlight controller bound to the panel. */
    lgfx::Light_PWM light_;
    /** @brief I2C touch controller bound to the panel. */
    lgfx::Touch_CST816S touch_;
};

} // namespace display
