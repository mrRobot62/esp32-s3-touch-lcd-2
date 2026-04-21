#include "ui/UiDemo.hpp"

#include "display/BoardDisplay.hpp"
#include "log_core.h"
#include "UiDemoInternal.hpp"

namespace ui {

/**
 * @brief Store the shared board display and HAL references used by the UI runtime.
 *
 * @param display Reference to the initialized board display driver.
 * @param hal Reference to the initialized HAL context.
 */
UiDemo::UiDemo(display::BoardDisplay& display, hal::HalContext& hal) : display_(display), hal_(hal) {}

/**
 * @brief Initialize LVGL, build all screens, and load the start screen.
 */
void UiDemo::begin() {
    INFO_TAG("UI", "Initializing LVGL demo controller.\n");
    randomSeed(micros());
    initializeLvgl();
    createScreens();
    activateScreen(ScreenId::Start);
    last_lv_tick_at_ = millis();
    INFO_TAG("UI", "LVGL demo controller initialized.\n");
}

/**
 * @brief Advance LVGL timing and refresh the currently active screen logic.
 */
void UiDemo::update() {
    const unsigned long now = millis();
    const unsigned long elapsed = now - last_lv_tick_at_;
    last_lv_tick_at_ = now;

    if (elapsed > 0) {
        lv_tick_inc(static_cast<uint32_t>(elapsed));
    }

    updateActiveScreen();
    lv_timer_handler();
}

/**
 * @brief Initialize LVGL and bind it to the already configured LovyanGFX display.
 *
 * The method creates a partial-refresh LVGL display using two RGB565 line
 * buffers and registers a touch input device that forwards CST816 coordinates
 * into LVGL.
 */
void UiDemo::initializeLvgl() {
    if (!lv_is_initialized()) {
        lv_init();
    }

    lv_display_ = lv_display_create(detail::kScreenWidth, detail::kScreenHeight);
    lv_display_set_user_data(lv_display_, this);
    lv_display_set_color_format(lv_display_, LV_COLOR_FORMAT_RGB565_SWAPPED);
    lv_display_set_flush_cb(lv_display_, displayFlushCallback);
    lv_display_set_buffers(
        lv_display_,
        draw_buffer_a_.data(),
        draw_buffer_b_.data(),
        static_cast<uint32_t>(draw_buffer_a_.size() * sizeof(uint16_t)),
        LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_default(lv_display_);

    lv_input_ = lv_indev_create();
    lv_indev_set_user_data(lv_input_, this);
    lv_indev_set_display(lv_input_, lv_display_);
    lv_indev_set_type(lv_input_, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(lv_input_, touchReadCallback);
}

/**
 * @brief Create every persistent screen object used by the demo application.
 */
void UiDemo::createScreens() {
    createStartScreen();
    createTouchScreen();
    createGraphicScreen();
    createFontScreen();
    createWidgetScreen();
    createAccelSimpleScreen();
    createAccelGameScreen();
}

/**
 * @brief Load the selected screen and reset the screen-specific runtime state.
 *
 * @param screen Screen identifier to activate.
 */
void UiDemo::activateScreen(ScreenId screen) {
    active_screen_ = screen;

    switch (screen) {
        case ScreenId::Start:
            lv_screen_load(start_screen_);
            INFO_TAG("UI", "Loaded start screen.\n");
            break;
        case ScreenId::TouchTest:
            touch_fps_ = 0;
            touch_last_sample_at_ = 0;
            lv_screen_load(touch_screen_);
            INFO_TAG("UI", "Loaded touch diagnostics screen.\n");
            break;
        case ScreenId::GraphicTest:
            graphic_phase_ = 0;
            graphic_fps_ = 0;
            graphic_last_update_at_ = 0;
            graphic_last_phase_at_ = millis();
            applyGraphicPhase();
            lv_screen_load(graphic_screen_);
            INFO_TAG("UI", "Loaded graphics performance screen.\n");
            break;
        case ScreenId::FontTest:
            font_phase_ = 0;
            font_last_phase_at_ = millis();
            applyFontPhase();
            lv_screen_load(font_screen_);
            INFO_TAG("UI", "Loaded font showcase screen.\n");
            break;
        case ScreenId::LvglWidgets:
            lv_screen_load(widget_screen_);
            INFO_TAG("UI", "Loaded LVGL widget showcase screen.\n");
            break;
        case ScreenId::AccelSimple:
            accel_last_sample_at_ = 0;
            lv_screen_load(accel_simple_screen_);
            INFO_TAG("UI", "Loaded accelerometer diagnostics screen.\n");
            break;
        case ScreenId::AccelGame:
            accel_last_sample_at_ = 0;
            accel_game_blink_cycles_remaining_ = 0;
            accel_game_last_blink_at_ = 0;
            accel_game_marker_visible_ = true;
            lv_obj_clear_flag(accel_game_mover_, LV_OBJ_FLAG_HIDDEN);
            lv_obj_center(accel_game_mover_);
            lv_screen_load(accel_game_screen_);
            INFO_TAG("UI", "Loaded accelerometer game screen.\n");
            break;
    }
}

/**
 * @brief Update the logic that belongs to the currently visible screen.
 */
void UiDemo::updateActiveScreen() {
    switch (active_screen_) {
        case ScreenId::TouchTest:
            updateTouchScreen();
            break;
        case ScreenId::GraphicTest:
            updateGraphicScreen();
            break;
        case ScreenId::FontTest:
            rotateFontPhaseIfNeeded();
            break;
        case ScreenId::AccelSimple:
            updateAccelSimpleScreen();
            break;
        case ScreenId::AccelGame:
            updateAccelGameScreen();
            break;
        case ScreenId::Start:
        case ScreenId::LvglWidgets:
            break;
    }
}

/**
 * @brief Read one fresh IMU sample into the cached UI state.
 *
 * @return `true` if the IMU provided a valid sample.
 */
bool UiDemo::refreshImuSample() {
    if (!hal_.isImuReady()) {
        return false;
    }

    return hal_.readImuSample(last_imu_sample_);
}

} // namespace ui
