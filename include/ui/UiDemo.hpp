#pragma once

#include <array>
#include <cstdint>
#include <utility>

#include <lvgl.h>

#include "hal/HalContext.hpp"

namespace display {
class BoardDisplay;
}

namespace ui {

/**
 * @brief LVGL-based demo application that exposes the hardware validation screens.
 *
 * The class owns the LVGL display/input bindings for the shared board display and
 * builds a small screen-based demo application. The implementation keeps the
 * application logic, screen creation, touch diagnostics, and performance-oriented
 * animation updates in one place so future developers can extend the demo
 * without having to rediscover the LVGL bring-up details.
 */
class UiDemo {
  public:
    /**
     * @brief Construct the UI controller around the already initialized board display.
     *
     * @param display Reference to the shared board display driver used for LVGL flushes and touch reads.
     * @param hal Reference to the shared HAL context that provides IMU and network state.
     */
    UiDemo(display::BoardDisplay& display, hal::HalContext& hal);

    /**
     * @brief Initialize LVGL, bind the display and touch callbacks, and load the start screen.
     */
    void begin();

    /**
     * @brief Advance LVGL timing and process the currently active screen logic.
     *
     * Call this method from the Arduino main loop as often as possible so LVGL can
     * refresh the display and run its timers with predictable latency.
     */
    void update();

  private:
    /**
     * @brief Logical screens exposed by the demo application.
     */
    enum class ScreenId {
        Start,
        TouchTest,
        GraphicTest,
        FontTest,
        LvglWidgets,
        AccelSimple,
        AccelGame,
    };

    /**
     * @brief Touch event states shown on the diagnostics screen.
     */
    enum class TouchEventState {
        Idle,
        Press,
        Move,
        Hold,
        Release,
    };

    /**
     * @brief Number of animated bar objects used by the graphic performance screen.
     */
    static constexpr std::size_t kGraphicBarCount = 12U;

    /**
     * @brief Number of moving particle objects used by the graphic performance screen.
     */
    static constexpr std::size_t kGraphicParticleCount = 14U;

    /**
     * @brief Number of nested boxes used by the rectangle animation scene.
     */
    static constexpr std::size_t kNestedBoxCount = 6U;

    /**
     * @brief Number of rainbow strips used by the color sweep scene.
     */
    static constexpr std::size_t kRainbowStripCount = 8U;

    /**
     * @brief Width of the canvas used by the sprite-sequence scene.
     */
    static constexpr std::size_t kSpriteCanvasWidth = 248U;

    /**
     * @brief Height of the canvas used by the sprite-sequence scene.
     */
    static constexpr std::size_t kSpriteCanvasHeight = 120U;

    /**
     * @brief Simple particle state used by the animated graphics screen.
     */
    struct ParticleState {
        /** @brief Current X coordinate inside the graphics viewport. */
        int16_t x = 0;
        /** @brief Current Y coordinate inside the graphics viewport. */
        int16_t y = 0;
        /** @brief Signed X velocity in pixels per update step. */
        int8_t vx = 1;
        /** @brief Signed Y velocity in pixels per update step. */
        int8_t vy = 1;
    };

    /** @brief Shared hardware display used for LVGL flushes and touch access. */
    display::BoardDisplay& display_;
    /** @brief Shared HAL context used for IMU-backed demo pages. */
    hal::HalContext& hal_;
    /** @brief LVGL display handle bound to the LovyanGFX-backed panel. */
    lv_display_t* lv_display_ = nullptr;
    /** @brief LVGL input device handle bound to the touch controller. */
    lv_indev_t* lv_input_ = nullptr;
    /** @brief Timestamp of the last LVGL tick update in milliseconds. */
    unsigned long last_lv_tick_at_ = 0;
    /** @brief Currently visible logical screen. */
    ScreenId active_screen_ = ScreenId::Start;

    /** @brief Primary partial draw buffer for LVGL RGB565 rendering. */
    std::array<uint16_t, 320U * 20U> draw_buffer_a_{};
    /** @brief Secondary partial draw buffer for LVGL RGB565 rendering. */
    std::array<uint16_t, 320U * 20U> draw_buffer_b_{};

    /** @brief Latest raw touch X coordinate received from the panel. */
    uint16_t last_touch_x_ = 0;
    /** @brief Latest raw touch Y coordinate received from the panel. */
    uint16_t last_touch_y_ = 0;
    /** @brief Previous raw touch X coordinate used to derive move events. */
    uint16_t previous_touch_x_ = 0;
    /** @brief Previous raw touch Y coordinate used to derive move events. */
    uint16_t previous_touch_y_ = 0;
    /** @brief Whether the touch controller currently reports an active finger. */
    bool is_touch_active_ = false;
    /** @brief Most recent derived touch event state. */
    TouchEventState touch_event_state_ = TouchEventState::Idle;

    /** @brief Timestamp of the last touch diagnostics refresh. */
    unsigned long touch_last_sample_at_ = 0;
    /** @brief Smoothed diagnostics FPS value for the touch screen. */
    uint16_t touch_fps_ = 0;
    /** @brief Timestamp of the last graphics update used for FPS smoothing. */
    unsigned long graphic_last_update_at_ = 0;
    /** @brief Smoothed graphics FPS value for the performance screen. */
    uint16_t graphic_fps_ = 0;
    /** @brief Timestamp of the last 3-second graphics scene transition. */
    unsigned long graphic_last_phase_at_ = 0;
    /** @brief Timestamp of the last 3-second font phase transition. */
    unsigned long font_last_phase_at_ = 0;
    /** @brief Current graphics scene index. */
    uint8_t graphic_phase_ = 0;
    /** @brief Current font presentation phase index. */
    uint8_t font_phase_ = 0;

    /** @brief Start screen root object. */
    lv_obj_t* start_screen_ = nullptr;
    /** @brief Touch diagnostics screen root object. */
    lv_obj_t* touch_screen_ = nullptr;
    /** @brief Graphics performance screen root object. */
    lv_obj_t* graphic_screen_ = nullptr;
    /** @brief Font showcase screen root object. */
    lv_obj_t* font_screen_ = nullptr;
    /** @brief Widget showcase screen root object. */
    lv_obj_t* widget_screen_ = nullptr;
    /** @brief Accelerometer diagnostics screen root object. */
    lv_obj_t* accel_simple_screen_ = nullptr;
    /** @brief Accelerometer game screen root object. */
    lv_obj_t* accel_game_screen_ = nullptr;

    /** @brief Label that shows the latest touch event state. */
    lv_obj_t* touch_event_value_label_ = nullptr;
    /** @brief Label that shows the latest touch X coordinate. */
    lv_obj_t* touch_x_value_label_ = nullptr;
    /** @brief Label that shows the latest touch Y coordinate. */
    lv_obj_t* touch_y_value_label_ = nullptr;
    /** @brief Label that shows whether the panel is currently touched. */
    lv_obj_t* touch_active_value_label_ = nullptr;
    /** @brief Overlay label that shows the touch diagnostics FPS value. */
    lv_obj_t* touch_fps_label_ = nullptr;

    /** @brief Container that hosts the active graphics demo scene. */
    lv_obj_t* graphic_viewport_ = nullptr;
    /** @brief Label that shows the currently active graphics scene name. */
    lv_obj_t* graphic_scene_label_ = nullptr;
    /** @brief Overlay label that shows the graphics screen FPS value. */
    lv_obj_t* graphic_fps_label_ = nullptr;
    /** @brief Solid-color scene container. */
    lv_obj_t* graphic_scene_solid_ = nullptr;
    /** @brief Wave bar scene container. */
    lv_obj_t* graphic_scene_bars_ = nullptr;
    /** @brief Moving particle scene container. */
    lv_obj_t* graphic_scene_particles_ = nullptr;
    /** @brief Nested rectangle scene container. */
    lv_obj_t* graphic_scene_nested_ = nullptr;
    /** @brief Rainbow strip scene container. */
    lv_obj_t* graphic_scene_rainbow_ = nullptr;
    /** @brief Sprite-sequence scene container. */
    lv_obj_t* graphic_scene_sprite_ = nullptr;
    /** @brief Canvas that renders the native LVGL frame-sequence animation. */
    lv_obj_t* graphic_sprite_canvas_ = nullptr;
    /** @brief Animated bar objects used by the bar-wave scene. */
    std::array<lv_obj_t*, kGraphicBarCount> graphic_bars_{};
    /** @brief Animated particle objects used by the particle scene. */
    std::array<lv_obj_t*, kGraphicParticleCount> graphic_particles_{};
    /** @brief State backing the animated particle objects. */
    std::array<ParticleState, kGraphicParticleCount> graphic_particle_states_{};
    /** @brief Nested rectangle objects used by the shrinking box scene. */
    std::array<lv_obj_t*, kNestedBoxCount> graphic_nested_boxes_{};
    /** @brief Rainbow strip objects used by the rainbow scene. */
    std::array<lv_obj_t*, kRainbowStripCount> graphic_rainbow_strips_{};
    /** @brief RGB565 draw buffer backing the sprite-sequence canvas. */
    std::array<uint16_t, kSpriteCanvasWidth * kSpriteCanvasHeight> graphic_sprite_canvas_buffer_{};

    /** @brief Label that shows the current font test title. */
    lv_obj_t* font_title_label_ = nullptr;
    /** @brief Main sample label used to demonstrate font styles and colors. */
    lv_obj_t* font_sample_label_ = nullptr;
    /** @brief Secondary label that explains the current font phase. */
    lv_obj_t* font_info_label_ = nullptr;

    /** @brief Label that shows the latest accelerometer X value. */
    lv_obj_t* accel_simple_x_label_ = nullptr;
    /** @brief Label that shows the latest accelerometer Y value. */
    lv_obj_t* accel_simple_y_label_ = nullptr;
    /** @brief Label that shows the latest accelerometer Z value. */
    lv_obj_t* accel_simple_z_label_ = nullptr;
    /** @brief Label that shows the latest gyroscope X value. */
    lv_obj_t* accel_simple_gx_label_ = nullptr;
    /** @brief Label that shows the latest gyroscope Y value. */
    lv_obj_t* accel_simple_gy_label_ = nullptr;
    /** @brief Label that shows the latest gyroscope Z value. */
    lv_obj_t* accel_simple_gz_label_ = nullptr;
    /** @brief Label that reports the current IMU status on the simple screen. */
    lv_obj_t* accel_simple_status_label_ = nullptr;
    /** @brief Outer playfield object of the accelerometer game. */
    lv_obj_t* accel_game_field_ = nullptr;
    /** @brief Static target circle in the center of the accelerometer game. */
    lv_obj_t* accel_game_target_ = nullptr;
    /** @brief Moving circle driven by the accelerometer values. */
    lv_obj_t* accel_game_mover_ = nullptr;
    /** @brief Status label shown below the accelerometer game field. */
    lv_obj_t* accel_game_status_label_ = nullptr;
    /** @brief Most recent IMU sample used by the accelerometer screens. */
    hal::HalContext::ImuSample last_imu_sample_{};
    /** @brief Timestamp of the last accelerometer sample refresh. */
    unsigned long accel_last_sample_at_ = 0;
    /** @brief Remaining blink toggles of the game marker when centered. */
    uint8_t accel_game_blink_cycles_remaining_ = 0;
    /** @brief Timestamp of the last blink state transition of the game marker. */
    unsigned long accel_game_last_blink_at_ = 0;
    /** @brief Current visibility state of the moving game marker during blinking. */
    bool accel_game_marker_visible_ = true;

    /**
     * @brief Initialize LVGL core services and bind the display and touch drivers.
     */
    void initializeLvgl();

    /**
     * @brief Create all application screens and their persistent widgets.
     */
    void createScreens();

    /**
     * @brief Build the start screen with navigation buttons to the test pages.
     */
    void createStartScreen();

    /**
     * @brief Build the touch diagnostics screen and its live value labels.
     */
    void createTouchScreen();

    /**
     * @brief Build the graphics performance screen and its animated scenes.
     */
    void createGraphicScreen();

    /**
     * @brief Build the font showcase screen and its sample labels.
     */
    void createFontScreen();

    /**
     * @brief Build the static LVGL widget showcase screen.
     */
    void createWidgetScreen();

    /**
     * @brief Build the accelerometer diagnostics screen with live sensor values.
     */
    void createAccelSimpleScreen();

    /**
     * @brief Build the accelerometer mini-game screen with a movable marker.
     */
    void createAccelGameScreen();

    /**
     * @brief Create a reusable screen title label.
     *
     * @param parent Screen or container that owns the title label.
     * @param title Text to show in the upper-left header area.
     *
     * @return Pointer to the created title label so callers can update it later if needed.
     */
    lv_obj_t* createScreenTitle(lv_obj_t* parent, const char* title);

    /**
     * @brief Create the small cancel button that returns from a test screen to the start screen.
     *
     * @param parent Screen that should host the cancel button.
     */
    void createCancelButton(lv_obj_t* parent);

    /**
     * @brief Create one navigation button on the start screen.
     *
     * @param parent Container that hosts the button.
     * @param label Button text shown to the user.
     * @param target Screen that should be loaded when the button is clicked.
     * @param accent_color Accent color used to visually distinguish the button.
     * @param enabled `true` when the button is clickable, otherwise the button is rendered disabled.
     *
     * @return Pointer to the created LVGL button object.
     */
    lv_obj_t* createMenuButton(lv_obj_t* parent, const char* label, ScreenId target, lv_color_t accent_color, bool enabled);

    /**
     * @brief Create a two-column diagnostics row with a caption and a value label.
     *
     * @param parent Container that owns the row.
     * @param caption Static caption text shown on the left side.
     *
     * @return Pointer to the value label so the caller can update it during runtime.
     */
    lv_obj_t* createValueRow(lv_obj_t* parent, const char* caption);

    /**
     * @brief Load a logical screen and update timer state for the new context.
     *
     * @param screen Screen identifier to activate.
     */
    void activateScreen(ScreenId screen);

    /**
     * @brief Update the logic associated with the currently active LVGL screen.
     */
    void updateActiveScreen();

    /**
     * @brief Refresh the touch diagnostics labels and derive a live FPS estimate.
     */
    void updateTouchScreen();

    /**
     * @brief Refresh the graphics scene animation and derive a live FPS estimate.
     */
    void updateGraphicScreen();

    /**
     * @brief Advance to the next graphics scene every three seconds.
     */
    void rotateGraphicPhaseIfNeeded();

    /**
     * @brief Make exactly one graphics scene visible and refresh its title.
     */
    void applyGraphicPhase();

    /**
     * @brief Update the animated wave-bar scene.
     *
     * @param now Current millisecond timestamp used to derive animation progress.
     */
    void updateGraphicBars(unsigned long now);

    /**
     * @brief Update the animated particle scene with bouncing points.
     */
    void updateGraphicParticles();

    /**
     * @brief Update the nested-rectangle scene with shrinking colored boxes.
     *
     * @param now Current millisecond timestamp used to derive animation progress.
     */
    void updateGraphicNested(unsigned long now);

    /**
     * @brief Update the rainbow-strip scene with scrolling colors.
     *
     * @param now Current millisecond timestamp used to derive animation progress.
     */
    void updateGraphicRainbow(unsigned long now);

    /**
     * @brief Update the LVGL-native sprite-sequence scene on the canvas.
     *
     * @param now Current millisecond timestamp used to derive frame selection and motion.
     */
    void updateGraphicSpriteSequence(unsigned long now);

    /**
     * @brief Refresh the live IMU values shown on the accelerometer diagnostics screen.
     */
    void updateAccelSimpleScreen();

    /**
     * @brief Refresh the accelerometer mini-game and blink when the target is centered.
     */
    void updateAccelGameScreen();

    /**
     * @brief Read one fresh IMU sample into the cached UI state.
     *
     * @return `true` if a fresh IMU sample was read successfully.
     */
    bool refreshImuSample();

    /**
     * @brief Advance to the next font presentation every three seconds.
     */
    void rotateFontPhaseIfNeeded();

    /**
     * @brief Apply the visual style of the currently selected font phase.
     */
    void applyFontPhase();

    /**
     * @brief Update a smoothed FPS value from two subsequent timestamps.
     *
     * @param fps_value Reference to the FPS value to update in place.
     * @param previous_timestamp Reference to the previous sample timestamp used for delta calculation.
     * @param now Current millisecond timestamp.
     */
    void updateFpsValue(uint16_t& fps_value, unsigned long& previous_timestamp, unsigned long now);

    /**
     * @brief Convert a touch event state into a short user-facing label.
     *
     * @param state Touch event state to convert.
     *
     * @return Static string label for the given event state.
     */
    const char* touchEventLabel(TouchEventState state) const;

    /**
     * @brief Log the current touch state transition with coordinates and activity information.
     */
    void logTouchState() const;

    /**
     * @brief Flush one LVGL invalidated area onto the physical display.
     *
     * @param display_handle LVGL display that requested the flush.
     * @param area Dirty rectangle that should be copied to the panel.
     * @param pixel_map RGB565 pixel payload generated by LVGL for the dirty rectangle.
     */
    static void displayFlushCallback(lv_display_t* display_handle, const lv_area_t* area, uint8_t* pixel_map);

    /**
     * @brief Read the current touch state and feed it into LVGL's pointer input pipeline.
     *
     * @param input_handle LVGL input device that requests fresh pointer data.
     * @param data Output structure that receives the current pointer state and coordinates.
     */
    static void touchReadCallback(lv_indev_t* input_handle, lv_indev_data_t* data);

    /**
     * @brief Handle screen navigation button clicks.
     *
     * @param event LVGL event object that identifies the clicked button and its target screen.
     */
    static void navigationEventCallback(lv_event_t* event);
};

} // namespace ui
