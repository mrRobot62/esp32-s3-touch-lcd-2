#include "ui/UiDemo.hpp"

#include <Arduino.h>

#include "display/BoardDisplay.hpp"
#include "modules/SystemInfo.hpp"

namespace {

/** @brief Default background color used by all current demo screens. */
constexpr uint16_t kBackgroundColor = TFT_BLACK;

} // namespace

namespace ui {

/**
 * @brief Store the shared display reference used by the demo renderer.
 *
 * @param display Reference to the board display that receives all draw calls.
 */
UiDemo::UiDemo(display::BoardDisplay& display) : display_(display) {}

/**
 * @brief Initialize the demo state and show the first validation screen.
 *
 * This method clears the display, sets a consistent text origin, and renders
 * the initial step so a developer can immediately verify display bring-up.
 */
void UiDemo::begin() {
    display_.fillScreen(kBackgroundColor);
    display_.setTextDatum(top_left);
    renderStep(0);
    last_step_at_ = millis();
    step_ = 0;
}

/**
 * @brief Advance to the next demo screen once one second has elapsed.
 *
 * The time-based state machine is intentionally simple so the first firmware
 * step remains predictable and easy to adapt while the UI architecture evolves.
 */
void UiDemo::update() {
    const auto now = millis();
    if ((now - last_step_at_) < 1000UL) {
        return;
    }

    step_ = (step_ + 1) % 4;
    renderStep(step_);
    last_step_at_ = now;
}

/**
 * @brief Draw one of the predefined bring-up screens.
 *
 * @param step Zero-based index that selects which demo screen to render.
 */
void UiDemo::renderStep(int step) {
    display_.startWrite();
    display_.fillScreen(kBackgroundColor);

    switch (step) {
        case 0:
            display_.setTextColor(TFT_CYAN, kBackgroundColor);
            display_.setFont(&fonts::Font2);
            display_.drawString(modules::SystemInfo::firmwareName(), 12, 12);

            display_.setTextColor(TFT_WHITE, kBackgroundColor);
            display_.setFont(&fonts::Font4);
            display_.drawString("Skeleton Step 1", 12, 48);

            display_.setTextColor(TFT_YELLOW, kBackgroundColor);
            display_.setFont(&fonts::Font2);
            display_.drawString("Display init ok", 12, 92);
            display_.drawString("UI demo cycle active", 12, 116);
            break;

        case 1:
            display_.fillRect(0, 0, 80, 320, TFT_RED);
            display_.fillRect(80, 0, 80, 320, TFT_GREEN);
            display_.fillRect(160, 0, 80, 320, TFT_BLUE);
            display_.setTextColor(TFT_WHITE, TFT_BLUE);
            display_.setFont(&fonts::Font2);
            display_.drawString("RGB bars", 172, 12);
            break;

        case 2:
            display_.fillRoundRect(12, 16, 296, 60, 12, TFT_DARKGREEN);
            display_.drawRoundRect(12, 96, 296, 60, 12, TFT_ORANGE);
            display_.fillCircle(56, 200, 30, TFT_MAGENTA);
            display_.drawCircle(140, 200, 30, TFT_CYAN);
            display_.fillTriangle(210, 230, 250, 160, 290, 230, TFT_YELLOW);
            display_.setTextColor(TFT_WHITE, kBackgroundColor);
            display_.setFont(&fonts::Font2);
            display_.drawString("Primitive preview", 20, 34);
            break;

        default:
            display_.setTextColor(TFT_WHITE, kBackgroundColor);
            display_.setFont(&fonts::Font2);
            display_.drawString("Folders prepared:", 12, 12);
            display_.drawString("hal / modules / ui / display", 12, 40);
            display_.drawString("Next step: LVGL widgets", 12, 68);
            break;
    }

    display_.endWrite();
}

} // namespace ui
