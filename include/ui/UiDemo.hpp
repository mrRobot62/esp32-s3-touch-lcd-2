#pragma once

namespace display {
class BoardDisplay;
}

namespace ui {

/**
 * @brief Minimal screen demo used to validate display bring-up and rendering flow.
 *
 * The class cycles through a small set of text and primitive drawing screens.
 * It serves as a safe first-step demo before integrating LVGL widgets and touch
 * interaction in later implementation steps.
 */
class UiDemo {
  public:
    /**
     * @brief Construct the demo renderer around the shared board display object.
     *
     * @param display Reference to the already configured board display driver.
     */
    explicit UiDemo(display::BoardDisplay& display);

    /**
     * @brief Reset the demo state and render the first screen immediately.
     *
     * This method should be called once after the display hardware has been
     * initialized and is ready to receive drawing commands.
     */
    void begin();

    /**
     * @brief Advance the demo state machine when the configured step interval expires.
     *
     * The method is intentionally non-blocking so it can be called in every Arduino
     * loop iteration without affecting future touch or UI integration.
     */
    void update();

  private:
    /** @brief Shared display driver used for all drawing operations. */
    display::BoardDisplay& display_;
    /** @brief Timestamp of the last rendered demo step in milliseconds. */
    unsigned long last_step_at_ = 0;
    /** @brief Index of the currently active demo step. */
    int step_ = -1;

    /**
     * @brief Render a specific demo screen.
     *
     * @param step Zero-based index of the demo screen to draw.
     */
    void renderStep(int step);
};

} // namespace ui
