#define LGFX_USE_V1
#include "display/BoardDisplay.hpp"

#include <Arduino.h>

#include "log_core.h"

namespace {

constexpr int kDisplayWidth = 240;
constexpr int kDisplayHeight = 320;
constexpr int kPinSclk = 39;
constexpr int kPinMosi = 38;
constexpr int kPinMiso = 40;
constexpr int kPinDc = 42;
constexpr int kPinCs = 45;
constexpr int kPinBacklight = 1;
constexpr int kPinTouchSda = 48;
constexpr int kPinTouchScl = 47;
constexpr uint8_t kTouchAddress = 0x15;

} // namespace

namespace display {

/**
 * @brief Prepare the complete LovyanGFX board configuration for this hardware.
 *
 * The constructor maps the board-specific SPI, backlight, and touch settings
 * into LovyanGFX configuration objects. Keeping the configuration grouped here
 * allows a developer to adjust pins, frequencies, orientation, or controller
 * behavior without touching the higher-level application code.
 */
BoardDisplay::BoardDisplay() {
    INFO_TAG("DISPLAY", "Constructing board display configuration.\n");

    // Configure the SPI bus used by the ST7789 display controller. These values
    // define the electrical connection and transfer timing between the ESP32-S3
    // and the display, so this is the first place to adapt when porting to a
    // different board revision or tuning display performance.
    {
        INFO_TAG(
            "DISPLAY",
            "Configuring SPI bus: host=%d, sclk=%d, mosi=%d, miso=%d, dc=%d, write=%d Hz.\n",
            SPI2_HOST,
            kPinSclk,
            kPinMosi,
            -1,
            kPinDc,
            20000000);
        auto cfg = bus_.config();
        cfg.spi_host = SPI2_HOST;
        cfg.spi_mode = 0;
        cfg.freq_write = 20000000;
        cfg.freq_read = 8000000;
        cfg.spi_3wire = false;
        cfg.use_lock = true;
        cfg.dma_channel = SPI_DMA_CH_AUTO;
        cfg.pin_sclk = kPinSclk;
        cfg.pin_mosi = kPinMosi;
        cfg.pin_miso = -1;
        cfg.pin_dc = kPinDc;
        bus_.config(cfg);
        panel_.setBus(&bus_);
    }

    // Configure the display panel geometry and controller behavior. Panel size,
    // color inversion, readable mode, and offsets belong here because they
    // describe how the ST7789 controller is wired to the physical LCD module.
    {
        INFO_TAG(
            "DISPLAY",
            "Configuring panel: cs=%d, width=%d, height=%d, invert=%s, rgb_order=%s.\n",
            kPinCs,
            kDisplayWidth,
            kDisplayHeight,
            "true",
            "false");
        auto cfg = panel_.config();
        cfg.pin_cs = kPinCs;
        cfg.pin_rst = -1;
        cfg.pin_busy = -1;
        cfg.panel_width = kDisplayWidth;
        cfg.panel_height = kDisplayHeight;
        cfg.offset_x = 0;
        cfg.offset_y = 0;
        cfg.offset_rotation = 0;
        cfg.readable = false;
        cfg.invert = true;
        cfg.rgb_order = false;
        cfg.dlen_16bit = false;
        cfg.bus_shared = true;
        panel_.config(cfg);
    }

    // Configure PWM-based backlight control. Adjust this block if the board
    // changes to another backlight pin, requires inverted logic, or needs a
    // different PWM channel/frequency for dimming behavior.
    {
        INFO_TAG(
            "DISPLAY",
            "Configuring backlight: pin=%d, pwm_channel=%d, freq=%d Hz.\n",
            kPinBacklight,
            7,
            44100);
        auto cfg = light_.config();
        cfg.pin_bl = kPinBacklight;
        cfg.invert = false;
        cfg.freq = 44100;
        cfg.pwm_channel = 7;
        light_.config(cfg);
        panel_.setLight(&light_);
    }

    // Configure the I2C touch controller and map its raw coordinate range to
    // the display space. This block is the primary place to change touch pins,
    // address, orientation compensation, or future interrupt integration.
    {
        INFO_TAG(
            "DISPLAY",
            "Configuring touch: addr=0x%02X, sda=%d, scl=%d, width=%d, height=%d.\n",
            kTouchAddress,
            kPinTouchSda,
            kPinTouchScl,
            kDisplayWidth,
            kDisplayHeight);
        auto cfg = touch_.config();
        cfg.x_min = 0;
        cfg.x_max = kDisplayWidth - 1;
        cfg.y_min = 0;
        cfg.y_max = kDisplayHeight - 1;
        cfg.i2c_port = 0;
        cfg.i2c_addr = kTouchAddress;
        cfg.pin_sda = kPinTouchSda;
        cfg.pin_scl = kPinTouchScl;
        cfg.pin_int = -1;
        cfg.bus_shared = false;
        cfg.offset_rotation = 0;
        cfg.freq = 400000;
        touch_.config(cfg);
        panel_.setTouch(&touch_);
    }

    // Bind the fully configured panel to the LovyanGFX device base class so the
    // rest of the application can use the object as a regular display instance.
    setPanel(&panel_);
    INFO_TAG("DISPLAY", "Board display configuration completed.\n");
}

/**
 * @brief Drive the backlight GPIO to a visible state before the panel is fully initialized.
 *
 * Some early bring-up situations are easier to diagnose when the backlight is
 * forced on independently from the LovyanGFX PWM setup. This makes it obvious
 * whether the issue is purely the panel light or the display controller itself.
 */
void BoardDisplay::forceBacklightOn() {
    pinMode(kPinBacklight, OUTPUT);
    digitalWrite(kPinBacklight, HIGH);
    INFO_TAG("DISPLAY", "Backlight GPIO forced HIGH on pin %d.\n", kPinBacklight);
}

} // namespace display
