#include "ui/UiDemo.hpp"

#include <Arduino.h>
#include <cmath>

#include "display/BoardDisplay.hpp"
#include "log_core.h"
#include "modules/SystemInfo.hpp"

namespace {

/** @brief Logical width of the rotated display in pixels. */
constexpr int32_t kScreenWidth = 320;
/** @brief Logical height of the rotated display in pixels. */
constexpr int32_t kScreenHeight = 240;
/** @brief Frame interval used by the touch diagnostics screen. */
constexpr unsigned long kTouchUpdateIntervalMs = 33UL;
/** @brief Frame interval used by the animated graphics screen. */
constexpr unsigned long kGraphicUpdateIntervalMs = 16UL;
/** @brief Scene rotation interval for the graphics test in milliseconds. */
constexpr unsigned long kGraphicPhaseIntervalMs = 3000UL;
/** @brief Presentation rotation interval for the font test in milliseconds. */
constexpr unsigned long kFontPhaseIntervalMs = 3000UL;
/** @brief Number of graphics phases presented by the demo. */
constexpr uint8_t kGraphicPhaseCount = 5U;
/** @brief Number of font phases presented by the demo. */
constexpr uint8_t kFontPhaseCount = 4U;

/**
 * @brief Return a reusable LVGL color from a 24-bit RGB hex value.
 *
 * @param rgb 24-bit RGB color value in `0xRRGGBB` form.
 *
 * @return LVGL color converted for the active color format.
 */
lv_color_t uiColor(uint32_t rgb) {
    return lv_color_hex(rgb);
}

/**
 * @brief Apply a consistent background theme to a root LVGL screen.
 *
 * @param screen Screen object that should receive the base background styling.
 */
void styleScreen(lv_obj_t* screen) {
    lv_obj_set_style_bg_color(screen, uiColor(0x101820), 0);
    lv_obj_set_style_bg_grad_color(screen, uiColor(0x182B3A), 0);
    lv_obj_set_style_bg_grad_dir(screen, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_border_width(screen, 0, 0);
    lv_obj_set_style_pad_all(screen, 0, 0);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_OFF);
}

/**
 * @brief Apply a shared panel style used by content containers on the demo screens.
 *
 * @param container LVGL object that should visually behave like a content panel.
 */
void stylePanel(lv_obj_t* container) {
    lv_obj_set_style_bg_color(container, uiColor(0x1B2D3D), 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(container, uiColor(0x49708A), 0);
    lv_obj_set_style_border_width(container, 1, 0);
    lv_obj_set_style_radius(container, 10, 0);
    lv_obj_set_style_pad_all(container, 10, 0);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(container, LV_SCROLLBAR_MODE_OFF);
}

/**
 * @brief Return a short human-readable label for a graphics scene index.
 *
 * @param phase Graphics phase index that should be translated.
 *
 * @return Static string label that describes the scene.
 */
const char* graphicPhaseLabel(uint8_t phase) {
    switch (phase) {
        case 0:
            return "Solid Colors";
        case 1:
            return "Wave Bars";
        case 2:
            return "Particle Field";
        case 3:
            return "Nested Rectangles";
        case 4:
            return "Rainbow Sweep";
        default:
            return "Unknown";
    }
}

} // namespace

namespace ui {

/**
 * @brief Store the shared board display reference used for LVGL flush and touch callbacks.
 *
 * @param display Reference to the initialized board display driver.
 */
UiDemo::UiDemo(display::BoardDisplay& display) : display_(display) {}

/**
 * @brief Initialize LVGL, create all screens, and load the demo start screen.
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
 * The method creates a partial-refresh LVGL display using two RGB565 line buffers
 * and registers a touch input device that forwards CST816 coordinates into LVGL.
 */
void UiDemo::initializeLvgl() {
    if (!lv_is_initialized()) {
        lv_init();
    }

    lv_display_ = lv_display_create(kScreenWidth, kScreenHeight);
    lv_display_set_user_data(lv_display_, this);
    lv_display_set_color_format(lv_display_, LV_COLOR_FORMAT_RGB565);
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
}

/**
 * @brief Build the start screen with direct entry points for all demo areas.
 */
void UiDemo::createStartScreen() {
    start_screen_ = lv_obj_create(nullptr);
    styleScreen(start_screen_);
    createScreenTitle(start_screen_, "Demo Start");

    lv_obj_t* subtitle = lv_label_create(start_screen_);
    lv_label_set_text_fmt(subtitle, "%s", modules::SystemInfo::firmwareName());
    lv_obj_set_style_text_color(subtitle, uiColor(0xCFE8FF), 0);
    lv_obj_align(subtitle, LV_ALIGN_TOP_LEFT, 16, 34);

    lv_obj_t* panel = lv_obj_create(start_screen_);
    stylePanel(panel);
    lv_obj_set_size(panel, 288, 178);
    lv_obj_align(panel, LV_ALIGN_TOP_LEFT, 16, 52);

    createMenuButton(panel, "TOUCH-TEST", ScreenId::TouchTest, uiColor(0x1F8A70), true);
    createMenuButton(panel, "GRAPHIC-TEST", ScreenId::GraphicTest, uiColor(0x1D6FD6), true);
    createMenuButton(panel, "FONT-TEST", ScreenId::FontTest, uiColor(0x8B3DBA), true);
    createMenuButton(panel, "LVGL-WIDGETS", ScreenId::LvglWidgets, uiColor(0x9A5A12), true);
    createMenuButton(panel, "CAMERA-TEST", ScreenId::Start, uiColor(0x505050), false);

    lv_obj_t* hint = lv_label_create(start_screen_);
    lv_label_set_text(hint, "All active pages now run completely on LVGL.");
    lv_obj_set_style_text_color(hint, uiColor(0xFFD166), 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_LEFT, 16, -8);
}

/**
 * @brief Build the touch diagnostics screen with labels for the live touch state.
 */
void UiDemo::createTouchScreen() {
    touch_screen_ = lv_obj_create(nullptr);
    styleScreen(touch_screen_);
    createScreenTitle(touch_screen_, "Touch Test");
    createCancelButton(touch_screen_);

    lv_obj_t* panel = lv_obj_create(touch_screen_);
    stylePanel(panel);
    lv_obj_set_size(panel, 288, 160);
    lv_obj_align(panel, LV_ALIGN_TOP_LEFT, 16, 46);

    touch_event_value_label_ = createValueRow(panel, "Event");
    touch_x_value_label_ = createValueRow(panel, "X");
    touch_y_value_label_ = createValueRow(panel, "Y");
    touch_active_value_label_ = createValueRow(panel, "Active");

    lv_obj_t* hint = lv_label_create(touch_screen_);
    lv_label_set_text(hint, "Touch the panel to verify coordinates and event transitions.");
    lv_obj_set_width(hint, 250);
    lv_obj_set_style_text_color(hint, uiColor(0xCFE8FF), 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_LEFT, 16, -10);

    touch_fps_label_ = lv_label_create(touch_screen_);
    lv_label_set_text(touch_fps_label_, "FPS: 0");
    lv_obj_set_style_text_color(touch_fps_label_, uiColor(0xFFFFFF), 0);
    lv_obj_align(touch_fps_label_, LV_ALIGN_BOTTOM_RIGHT, -8, -10);
}

/**
 * @brief Build the graphics performance screen with multiple animated LVGL scenes.
 */
void UiDemo::createGraphicScreen() {
    graphic_screen_ = lv_obj_create(nullptr);
    styleScreen(graphic_screen_);
    createScreenTitle(graphic_screen_, "Graphic Test");
    createCancelButton(graphic_screen_);

    graphic_scene_label_ = lv_label_create(graphic_screen_);
    lv_label_set_text(graphic_scene_label_, "Scene: Solid Colors");
    lv_obj_set_style_text_color(graphic_scene_label_, uiColor(0xCFE8FF), 0);
    lv_obj_align(graphic_scene_label_, LV_ALIGN_TOP_LEFT, 16, 32);

    graphic_viewport_ = lv_obj_create(graphic_screen_);
    stylePanel(graphic_viewport_);
    lv_obj_set_size(graphic_viewport_, 288, 164);
    lv_obj_align(graphic_viewport_, LV_ALIGN_TOP_LEFT, 16, 52);
    lv_obj_set_style_clip_corner(graphic_viewport_, true, 0);

    graphic_scene_solid_ = lv_obj_create(graphic_viewport_);
    lv_obj_remove_style_all(graphic_scene_solid_);
    lv_obj_set_size(graphic_scene_solid_, 268, 142);
    lv_obj_align(graphic_scene_solid_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(graphic_scene_solid_, LV_OBJ_FLAG_SCROLLABLE);

    static const lv_color_t solid_colors[4] = {
        uiColor(0xD62828),
        uiColor(0x2A9D8F),
        uiColor(0x1565C0),
        uiColor(0xFFB703),
    };
    static const char* solid_labels[4] = {"RED", "GRN", "BLU", "YLW"};
    for (int index = 0; index < 4; ++index) {
        lv_obj_t* block = lv_obj_create(graphic_scene_solid_);
        lv_obj_set_size(block, 112, 46);
        lv_obj_set_pos(block, 10 + ((index % 2) * 136), 10 + ((index / 2) * 62));
        lv_obj_set_style_bg_color(block, solid_colors[index], 0);
        lv_obj_set_style_border_width(block, 0, 0);
        lv_obj_set_style_radius(block, 8, 0);
        lv_obj_clear_flag(block, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t* label = lv_label_create(graphic_scene_solid_);
        lv_label_set_text(label, solid_labels[index]);
        lv_obj_set_style_text_color(label, uiColor(0xF3F7FA), 0);
        lv_obj_align_to(label, block, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
    }

    graphic_scene_bars_ = lv_obj_create(graphic_viewport_);
    lv_obj_remove_style_all(graphic_scene_bars_);
    lv_obj_set_size(graphic_scene_bars_, 268, 142);
    lv_obj_align(graphic_scene_bars_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(graphic_scene_bars_, LV_OBJ_FLAG_SCROLLABLE);
    for (std::size_t index = 0; index < graphic_bars_.size(); ++index) {
        graphic_bars_[index] = lv_obj_create(graphic_scene_bars_);
        lv_obj_set_size(graphic_bars_[index], 14, 40);
        lv_obj_set_pos(graphic_bars_[index], 6 + static_cast<int32_t>(index) * 21, 90);
        lv_obj_set_style_radius(graphic_bars_[index], 4, 0);
        lv_obj_set_style_border_width(graphic_bars_[index], 0, 0);
        lv_obj_set_style_bg_color(graphic_bars_[index], uiColor(0x6BCB77), 0);
        lv_obj_clear_flag(graphic_bars_[index], LV_OBJ_FLAG_SCROLLABLE);
    }

    graphic_scene_particles_ = lv_obj_create(graphic_viewport_);
    lv_obj_remove_style_all(graphic_scene_particles_);
    lv_obj_set_size(graphic_scene_particles_, 268, 142);
    lv_obj_align(graphic_scene_particles_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(graphic_scene_particles_, LV_OBJ_FLAG_SCROLLABLE);
    for (std::size_t index = 0; index < graphic_particles_.size(); ++index) {
        graphic_particles_[index] = lv_obj_create(graphic_scene_particles_);
        lv_obj_set_size(graphic_particles_[index], 8, 8);
        lv_obj_set_style_radius(graphic_particles_[index], LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_border_width(graphic_particles_[index], 0, 0);
        lv_obj_set_style_bg_color(
            graphic_particles_[index],
            uiColor((index % 2U == 0U) ? 0xFF6B6B : 0x4D96FF),
            0);
        lv_obj_clear_flag(graphic_particles_[index], LV_OBJ_FLAG_SCROLLABLE);

        graphic_particle_states_[index].x = static_cast<int16_t>((index * 17U) % 240U);
        graphic_particle_states_[index].y = static_cast<int16_t>((index * 23U) % 110U);
        graphic_particle_states_[index].vx = static_cast<int8_t>((index % 3U) + 1U);
        graphic_particle_states_[index].vy = static_cast<int8_t>(((index + 1U) % 3U) + 1U);
    }

    graphic_scene_nested_ = lv_obj_create(graphic_viewport_);
    lv_obj_remove_style_all(graphic_scene_nested_);
    lv_obj_set_size(graphic_scene_nested_, 268, 142);
    lv_obj_align(graphic_scene_nested_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(graphic_scene_nested_, LV_OBJ_FLAG_SCROLLABLE);
    for (std::size_t index = 0; index < graphic_nested_boxes_.size(); ++index) {
        graphic_nested_boxes_[index] = lv_obj_create(graphic_scene_nested_);
        lv_obj_set_style_bg_opa(graphic_nested_boxes_[index], LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(graphic_nested_boxes_[index], 2, 0);
        lv_obj_set_style_radius(graphic_nested_boxes_[index], 2, 0);
        lv_obj_clear_flag(graphic_nested_boxes_[index], LV_OBJ_FLAG_SCROLLABLE);
    }

    graphic_scene_rainbow_ = lv_obj_create(graphic_viewport_);
    lv_obj_remove_style_all(graphic_scene_rainbow_);
    lv_obj_set_size(graphic_scene_rainbow_, 268, 142);
    lv_obj_align(graphic_scene_rainbow_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(graphic_scene_rainbow_, LV_OBJ_FLAG_SCROLLABLE);
    for (std::size_t index = 0; index < graphic_rainbow_strips_.size(); ++index) {
        graphic_rainbow_strips_[index] = lv_obj_create(graphic_scene_rainbow_);
        lv_obj_set_size(graphic_rainbow_strips_[index], 248, 14);
        lv_obj_set_pos(graphic_rainbow_strips_[index], 10, 8 + static_cast<int32_t>(index) * 16);
        lv_obj_set_style_radius(graphic_rainbow_strips_[index], 6, 0);
        lv_obj_set_style_border_width(graphic_rainbow_strips_[index], 0, 0);
        lv_obj_clear_flag(graphic_rainbow_strips_[index], LV_OBJ_FLAG_SCROLLABLE);
    }

    graphic_fps_label_ = lv_label_create(graphic_screen_);
    lv_label_set_text(graphic_fps_label_, "FPS: 0");
    lv_obj_set_style_text_color(graphic_fps_label_, uiColor(0xFFFFFF), 0);
    lv_obj_align(graphic_fps_label_, LV_ALIGN_BOTTOM_RIGHT, -8, -10);

    lv_obj_t* hint = lv_label_create(graphic_screen_);
    lv_label_set_text(hint, "The active scene changes every 3 seconds.");
    lv_obj_set_style_text_color(hint, uiColor(0xFFD166), 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_LEFT, 16, -10);

    applyGraphicPhase();
}

/**
 * @brief Build the font showcase screen with reusable labels for the rotating phases.
 */
void UiDemo::createFontScreen() {
    font_screen_ = lv_obj_create(nullptr);
    styleScreen(font_screen_);
    createScreenTitle(font_screen_, "Font Test");
    createCancelButton(font_screen_);

    font_title_label_ = lv_label_create(font_screen_);
    lv_label_set_text(font_title_label_, "Phase: Compact Info");
    lv_obj_set_style_text_color(font_title_label_, uiColor(0xCFE8FF), 0);
    lv_obj_align(font_title_label_, LV_ALIGN_TOP_LEFT, 16, 34);

    font_sample_label_ = lv_label_create(font_screen_);
    lv_obj_set_width(font_sample_label_, 280);
    lv_label_set_long_mode(font_sample_label_, LV_LABEL_LONG_WRAP);
    lv_label_set_text(font_sample_label_, "A brown fox jumps over the lazy dog.");
    lv_obj_set_style_text_align(font_sample_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(font_sample_label_, LV_ALIGN_CENTER, 0, -10);

    font_info_label_ = lv_label_create(font_screen_);
    lv_obj_set_width(font_info_label_, 280);
    lv_label_set_long_mode(font_info_label_, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(font_info_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(font_info_label_, uiColor(0xFFD166), 0);
    lv_obj_align(font_info_label_, LV_ALIGN_BOTTOM_MID, 0, -14);

    applyFontPhase();
}

/**
 * @brief Build the widget showcase screen with real LVGL widgets.
 */
void UiDemo::createWidgetScreen() {
    widget_screen_ = lv_obj_create(nullptr);
    styleScreen(widget_screen_);
    createScreenTitle(widget_screen_, "LVGL Widgets");
    createCancelButton(widget_screen_);

    lv_obj_t* info = lv_label_create(widget_screen_);
    lv_label_set_text(info, "This page shows real LVGL widgets without custom event handling.");
    lv_obj_set_width(info, 280);
    lv_label_set_long_mode(info, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(info, uiColor(0xCFE8FF), 0);
    lv_obj_align(info, LV_ALIGN_TOP_LEFT, 16, 34);

    lv_obj_t* left_panel = lv_obj_create(widget_screen_);
    stylePanel(left_panel);
    lv_obj_set_size(left_panel, 136, 142);
    lv_obj_align(left_panel, LV_ALIGN_TOP_LEFT, 16, 76);

    lv_obj_t* demo_button = lv_button_create(left_panel);
    lv_obj_set_size(demo_button, 100, 34);
    lv_obj_align(demo_button, LV_ALIGN_TOP_MID, 0, 6);
    lv_obj_t* demo_button_label = lv_label_create(demo_button);
    lv_label_set_text(demo_button_label, "Button");
    lv_obj_center(demo_button_label);

    lv_obj_t* spinner = lv_spinner_create(left_panel);
    lv_spinner_set_anim_params(spinner, 1000, 90);
    lv_obj_set_size(spinner, 44, 44);
    lv_obj_align(spinner, LV_ALIGN_CENTER, 0, 4);

    lv_obj_t* a_switch = lv_switch_create(left_panel);
    lv_obj_align(a_switch, LV_ALIGN_BOTTOM_MID, 0, -10);

    lv_obj_t* right_panel = lv_obj_create(widget_screen_);
    stylePanel(right_panel);
    lv_obj_set_size(right_panel, 136, 142);
    lv_obj_align(right_panel, LV_ALIGN_TOP_RIGHT, -16, 76);

    lv_obj_t* slider = lv_slider_create(right_panel);
    lv_obj_set_width(slider, 96);
    lv_slider_set_value(slider, 65, LV_ANIM_OFF);
    lv_obj_align(slider, LV_ALIGN_TOP_MID, 0, 12);

    lv_obj_t* bar = lv_bar_create(right_panel);
    lv_obj_set_size(bar, 96, 14);
    lv_bar_set_value(bar, 72, LV_ANIM_OFF);
    lv_obj_align(bar, LV_ALIGN_TOP_MID, 0, 54);

    lv_obj_t* textarea = lv_textarea_create(right_panel);
    lv_obj_set_size(textarea, 104, 44);
    lv_textarea_set_text(textarea, "Text area");
    lv_obj_align(textarea, LV_ALIGN_BOTTOM_MID, 0, -10);
}

/**
 * @brief Create a screen title label with a consistent visual style.
 *
 * @param parent Screen that should own the title label.
 * @param title Title text shown at the top-left corner.
 *
 * @return Pointer to the created title label.
 */
lv_obj_t* UiDemo::createScreenTitle(lv_obj_t* parent, const char* title) {
    lv_obj_t* label = lv_label_create(parent);
    lv_label_set_text(label, title);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label, uiColor(0x6AD5FF), 0);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 16, 8);
    return label;
}

/**
 * @brief Create the shared cancel button used on every non-start screen.
 *
 * @param parent Screen that should host the cancel button.
 */
void UiDemo::createCancelButton(lv_obj_t* parent) {
    lv_obj_t* button = lv_button_create(parent);
    lv_obj_set_size(button, 68, 28);
    lv_obj_align(button, LV_ALIGN_TOP_RIGHT, -8, 6);
    lv_obj_set_style_bg_color(button, uiColor(0x7F1D1D), 0);
    lv_obj_set_style_border_width(button, 0, 0);
    lv_obj_set_style_radius(button, 8, 0);
    lv_obj_set_user_data(button, reinterpret_cast<void*>(static_cast<uintptr_t>(ScreenId::Start)));
    lv_obj_add_event_cb(button, navigationEventCallback, LV_EVENT_CLICKED, this);

    lv_obj_t* label = lv_label_create(button);
    lv_label_set_text(label, "Cancel");
    lv_obj_set_style_text_color(label, uiColor(0xFFFFFF), 0);
    lv_obj_center(label);
}

/**
 * @brief Create one screen navigation button on the start screen.
 *
 * @param parent Container that hosts the navigation buttons.
 * @param label Button text.
 * @param target Screen to activate on click.
 * @param accent_color Background color used by the button.
 * @param enabled `true` when the button should be clickable.
 *
 * @return Pointer to the created button object.
 */
lv_obj_t* UiDemo::createMenuButton(
    lv_obj_t* parent,
    const char* label,
    ScreenId target,
    lv_color_t accent_color,
    bool enabled) {
    const uint32_t child_count = lv_obj_get_child_count(parent);

    lv_obj_t* button = lv_button_create(parent);
    lv_obj_set_size(button, 264, 26);
    lv_obj_set_pos(button, 12, 12 + static_cast<int32_t>(child_count / 1U) * 30);
    lv_obj_set_style_bg_color(button, accent_color, 0);
    lv_obj_set_style_border_width(button, 0, 0);
    lv_obj_set_style_radius(button, 8, 0);

    lv_obj_t* text = lv_label_create(button);
    lv_label_set_text(text, label);
    lv_obj_set_style_text_color(text, uiColor(0xFFFFFF), 0);
    lv_obj_center(text);

    if (enabled) {
        lv_obj_set_user_data(button, reinterpret_cast<void*>(static_cast<uintptr_t>(target)));
        lv_obj_add_event_cb(button, navigationEventCallback, LV_EVENT_CLICKED, this);
    } else {
        lv_obj_add_state(button, LV_STATE_DISABLED);
        lv_obj_remove_flag(button, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_color(button, uiColor(0x505050), 0);
        lv_obj_set_style_text_color(text, uiColor(0xD0D0D0), 0);
    }

    return button;
}

/**
 * @brief Create one caption/value row used by the touch diagnostics screen.
 *
 * @param parent Container that should own the row.
 * @param caption Static caption shown on the left side of the row.
 *
 * @return Pointer to the value label on the right side of the row.
 */
lv_obj_t* UiDemo::createValueRow(lv_obj_t* parent, const char* caption) {
    const uint32_t existing_rows = lv_obj_get_child_count(parent) / 2U;
    const int32_t row_y = 10 + static_cast<int32_t>(existing_rows) * 32;

    lv_obj_t* caption_label = lv_label_create(parent);
    lv_label_set_text(caption_label, caption);
    lv_obj_set_style_text_color(caption_label, uiColor(0x9CC7D8), 0);
    lv_obj_set_pos(caption_label, 8, row_y);

    lv_obj_t* value_label = lv_label_create(parent);
    lv_label_set_text(value_label, "-");
    lv_obj_set_style_text_color(value_label, uiColor(0xFFFFFF), 0);
    lv_obj_set_pos(value_label, 112, row_y);
    return value_label;
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
        case ScreenId::Start:
        case ScreenId::LvglWidgets:
            break;
    }
}

/**
 * @brief Refresh the touch diagnostics values at a stable low-overhead cadence.
 */
void UiDemo::updateTouchScreen() {
    const unsigned long now = millis();
    if (touch_last_sample_at_ != 0 && (now - touch_last_sample_at_) < kTouchUpdateIntervalMs) {
        return;
    }

    updateFpsValue(touch_fps_, touch_last_sample_at_, now);
    lv_label_set_text(touch_event_value_label_, touchEventLabel(touch_event_state_));
    lv_label_set_text_fmt(touch_x_value_label_, "%u", last_touch_x_);
    lv_label_set_text_fmt(touch_y_value_label_, "%u", last_touch_y_);
    lv_label_set_text(touch_active_value_label_, is_touch_active_ ? "yes" : "no");
    lv_label_set_text_fmt(touch_fps_label_, "FPS: %u", touch_fps_);
}

/**
 * @brief Refresh the active graphics scene and rotate scenes every three seconds.
 */
void UiDemo::updateGraphicScreen() {
    const unsigned long now = millis();
    if (graphic_last_update_at_ != 0 && (now - graphic_last_update_at_) < kGraphicUpdateIntervalMs) {
        rotateGraphicPhaseIfNeeded();
        return;
    }

    updateFpsValue(graphic_fps_, graphic_last_update_at_, now);
    rotateGraphicPhaseIfNeeded();

    switch (graphic_phase_) {
        case 0:
            break;
        case 1:
            updateGraphicBars(now);
            break;
        case 2:
            updateGraphicParticles();
            break;
        case 3:
            updateGraphicNested(now);
            break;
        case 4:
            updateGraphicRainbow(now);
            break;
        default:
            break;
    }

    lv_label_set_text_fmt(graphic_fps_label_, "FPS: %u", graphic_fps_);
}

/**
 * @brief Rotate the graphics scene after the configured three-second interval elapsed.
 */
void UiDemo::rotateGraphicPhaseIfNeeded() {
    const unsigned long now = millis();
    if ((now - graphic_last_phase_at_) < kGraphicPhaseIntervalMs) {
        return;
    }

    graphic_phase_ = static_cast<uint8_t>((graphic_phase_ + 1U) % kGraphicPhaseCount);
    graphic_last_phase_at_ = now;
    applyGraphicPhase();
}

/**
 * @brief Show exactly one graphics scene container and update the scene label text.
 */
void UiDemo::applyGraphicPhase() {
    const bool show_solid = graphic_phase_ == 0U;
    const bool show_bars = graphic_phase_ == 1U;
    const bool show_particles = graphic_phase_ == 2U;
    const bool show_nested = graphic_phase_ == 3U;
    const bool show_rainbow = graphic_phase_ == 4U;

    if (show_solid) {
        lv_obj_clear_flag(graphic_scene_solid_, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(graphic_scene_solid_, LV_OBJ_FLAG_HIDDEN);
    }

    if (show_bars) {
        lv_obj_clear_flag(graphic_scene_bars_, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(graphic_scene_bars_, LV_OBJ_FLAG_HIDDEN);
    }

    if (show_particles) {
        lv_obj_clear_flag(graphic_scene_particles_, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(graphic_scene_particles_, LV_OBJ_FLAG_HIDDEN);
    }

    if (show_nested) {
        lv_obj_clear_flag(graphic_scene_nested_, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(graphic_scene_nested_, LV_OBJ_FLAG_HIDDEN);
    }

    if (show_rainbow) {
        lv_obj_clear_flag(graphic_scene_rainbow_, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(graphic_scene_rainbow_, LV_OBJ_FLAG_HIDDEN);
    }

    lv_label_set_text_fmt(graphic_scene_label_, "Scene: %s", graphicPhaseLabel(graphic_phase_));
}

/**
 * @brief Update the animated wave-bar scene.
 *
 * @param now Current millisecond timestamp used to derive the bar animation phase.
 */
void UiDemo::updateGraphicBars(unsigned long now) {
    for (std::size_t index = 0; index < graphic_bars_.size(); ++index) {
        const float phase = static_cast<float>((now / 12UL) + (index * 15U));
        const int32_t height = 18 + static_cast<int32_t>(42.0f * (0.5f + 0.5f * sinf(phase * 0.08f)));
        const int32_t y = 116 - height;
        lv_obj_set_size(graphic_bars_[index], 14, height);
        lv_obj_set_pos(graphic_bars_[index], 6 + static_cast<int32_t>(index) * 21, y);

        const uint32_t hue_shift = static_cast<uint32_t>((phase * 9.0f));
        lv_obj_set_style_bg_color(
            graphic_bars_[index],
            uiColor(0x2040FFU + ((hue_shift * 73U) & 0x00FFFFU)),
            0);
    }
}

/**
 * @brief Update the moving particle field by integrating the stored particle velocities.
 */
void UiDemo::updateGraphicParticles() {
    for (std::size_t index = 0; index < graphic_particles_.size(); ++index) {
        ParticleState& state = graphic_particle_states_[index];

        state.x = static_cast<int16_t>(state.x + state.vx);
        state.y = static_cast<int16_t>(state.y + state.vy);

        if (state.x <= 0 || state.x >= 260) {
            state.vx = static_cast<int8_t>(-state.vx);
            state.x = static_cast<int16_t>(constrain(state.x, 0, 260));
        }

        if (state.y <= 0 || state.y >= 134) {
            state.vy = static_cast<int8_t>(-state.vy);
            state.y = static_cast<int16_t>(constrain(state.y, 0, 134));
        }

        lv_obj_set_pos(graphic_particles_[index], state.x, state.y);
    }
}

/**
 * @brief Update the nested rectangle animation from the outside towards the center.
 *
 * @param now Current millisecond timestamp used to derive the shrinking progress.
 */
void UiDemo::updateGraphicNested(unsigned long now) {
    const float progress = static_cast<float>(now % kGraphicPhaseIntervalMs) / static_cast<float>(kGraphicPhaseIntervalMs);
    static const uint32_t colors[kNestedBoxCount] = {
        0xE63946, 0xFF7F11, 0xFFD166, 0x06D6A0, 0x118AB2, 0x8338EC,
    };

    for (std::size_t index = 0; index < graphic_nested_boxes_.size(); ++index) {
        const int32_t base_width = 248 - static_cast<int32_t>(index) * 30;
        const int32_t base_height = 122 - static_cast<int32_t>(index) * 18;
        const float local_scale = 1.0f - progress * (0.10f + static_cast<float>(index) * 0.06f);
        const int32_t width = static_cast<int32_t>(base_width * local_scale);
        const int32_t height = static_cast<int32_t>(base_height * local_scale);
        lv_obj_set_size(graphic_nested_boxes_[index], width, height);
        lv_obj_align(graphic_nested_boxes_[index], LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_border_color(graphic_nested_boxes_[index], uiColor(colors[index]), 0);
    }
}

/**
 * @brief Update the rainbow strip scene by rotating the strip colors over time.
 *
 * @param now Current millisecond timestamp used to derive the hue rotation.
 */
void UiDemo::updateGraphicRainbow(unsigned long now) {
    static const uint32_t palette[kRainbowStripCount] = {
        0xE63946, 0xF77F00, 0xFDC500, 0x80ED99, 0x06D6A0, 0x118AB2, 0x3A86FF, 0x8338EC,
    };

    const uint32_t offset = static_cast<uint32_t>((now / 150UL) % kRainbowStripCount);
    for (std::size_t index = 0; index < graphic_rainbow_strips_.size(); ++index) {
        const std::size_t palette_index = (index + offset) % kRainbowStripCount;
        lv_obj_set_style_bg_color(graphic_rainbow_strips_[index], uiColor(palette[palette_index]), 0);
    }
}

/**
 * @brief Rotate the font demonstration after the configured three-second interval elapsed.
 */
void UiDemo::rotateFontPhaseIfNeeded() {
    const unsigned long now = millis();
    if ((now - font_last_phase_at_) < kFontPhaseIntervalMs) {
        return;
    }

    font_phase_ = static_cast<uint8_t>((font_phase_ + 1U) % kFontPhaseCount);
    font_last_phase_at_ = now;
    applyFontPhase();
}

/**
 * @brief Apply one of four text style presets to the font showcase screen.
 */
void UiDemo::applyFontPhase() {
    switch (font_phase_) {
        case 0:
            lv_label_set_text(font_title_label_, "Phase: Compact Info");
            lv_label_set_text(font_sample_label_, "A brown fox jumps over the lazy dog.");
            lv_label_set_text(font_info_label_, "Montserrat 12 with a warm accent color.");
            lv_obj_set_style_text_font(font_sample_label_, &lv_font_montserrat_12, 0);
            lv_obj_set_style_text_color(font_sample_label_, uiColor(0xFFD166), 0);
            break;

        case 1:
            lv_label_set_text(font_title_label_, "Phase: Balanced Body");
            lv_label_set_text(font_sample_label_, "A brown fox jumps over the lazy dog.");
            lv_label_set_text(font_info_label_, "Montserrat 16 for readable default body text.");
            lv_obj_set_style_text_font(font_sample_label_, &lv_font_montserrat_16, 0);
            lv_obj_set_style_text_color(font_sample_label_, uiColor(0xCFE8FF), 0);
            break;

        case 2:
            lv_label_set_text(font_title_label_, "Phase: Emphasized Heading");
            lv_label_set_text(font_sample_label_, "A brown fox jumps over the lazy dog.");
            lv_label_set_text(font_info_label_, "Montserrat 20 with a strong cyan emphasis.");
            lv_obj_set_style_text_font(font_sample_label_, &lv_font_montserrat_20, 0);
            lv_obj_set_style_text_color(font_sample_label_, uiColor(0x6AD5FF), 0);
            break;

        case 3:
        default:
            lv_label_set_text(font_title_label_, "Phase: Bold Showcase");
            lv_label_set_text(font_sample_label_, "A brown fox jumps over the lazy dog.");
            lv_label_set_text(font_info_label_, "Montserrat 24 for prominent hero copy.");
            lv_obj_set_style_text_font(font_sample_label_, &lv_font_montserrat_24, 0);
            lv_obj_set_style_text_color(font_sample_label_, uiColor(0xFF7B72), 0);
            break;
    }
}

/**
 * @brief Update an FPS estimate by smoothing the interval between successive updates.
 *
 * @param fps_value FPS output value that should be updated in place.
 * @param previous_timestamp Timestamp of the previous update, also updated in place.
 * @param now Current millisecond timestamp.
 */
void UiDemo::updateFpsValue(uint16_t& fps_value, unsigned long& previous_timestamp, unsigned long now) {
    if (previous_timestamp == 0) {
        previous_timestamp = now;
        fps_value = 0;
        return;
    }

    const unsigned long delta = now - previous_timestamp;
    previous_timestamp = now;
    if (delta == 0) {
        return;
    }

    const uint16_t instantaneous = static_cast<uint16_t>(1000UL / delta);
    if (fps_value == 0) {
        fps_value = instantaneous;
        return;
    }

    fps_value = static_cast<uint16_t>(((fps_value * 3U) + instantaneous) / 4U);
}

/**
 * @brief Convert a touch event state into a short diagnostic label.
 *
 * @param state Touch event state that should be translated.
 *
 * @return Static text representation of the given event state.
 */
const char* UiDemo::touchEventLabel(TouchEventState state) const {
    switch (state) {
        case TouchEventState::Idle:
            return "IDLE";
        case TouchEventState::Press:
            return "PRESS";
        case TouchEventState::Move:
            return "MOVE";
        case TouchEventState::Hold:
            return "HOLD";
        case TouchEventState::Release:
            return "RELEASE";
        default:
            return "UNKNOWN";
    }
}

/**
 * @brief Emit a log line that reflects the current touch diagnostics state.
 */
void UiDemo::logTouchState() const {
    INFO_TAG(
        "TOUCH",
        "event=%s x=%u y=%u active=%s\n",
        touchEventLabel(touch_event_state_),
        last_touch_x_,
        last_touch_y_,
        is_touch_active_ ? "yes" : "no");
}

/**
 * @brief Copy one invalidated LVGL area to the physical LovyanGFX display.
 *
 * @param display_handle LVGL display that requests the flush.
 * @param area Dirty area in LVGL logical coordinates.
 * @param pixel_map Pointer to RGB565 pixel data generated by LVGL.
 */
void UiDemo::displayFlushCallback(lv_display_t* display_handle, const lv_area_t* area, uint8_t* pixel_map) {
    UiDemo* instance = static_cast<UiDemo*>(lv_display_get_user_data(display_handle));
    const int32_t width = area->x2 - area->x1 + 1;
    const int32_t height = area->y2 - area->y1 + 1;

    instance->display_.pushImage(
        area->x1,
        area->y1,
        width,
        height,
        reinterpret_cast<const uint16_t*>(pixel_map));

    lv_display_flush_ready(display_handle);
}

/**
 * @brief Read one touch sample from the board touch controller and forward it into LVGL.
 *
 * @param input_handle LVGL input device requesting the current touch state.
 * @param data Output structure that receives the pointer state and coordinates.
 */
void UiDemo::touchReadCallback(lv_indev_t* input_handle, lv_indev_data_t* data) {
    UiDemo* instance = static_cast<UiDemo*>(lv_indev_get_user_data(input_handle));

    uint16_t x = instance->last_touch_x_;
    uint16_t y = instance->last_touch_y_;
    const bool touched = instance->display_.getTouch(&x, &y) > 0;
    const TouchEventState previous_state = instance->touch_event_state_;

    if (touched) {
        if (!instance->is_touch_active_) {
            instance->touch_event_state_ = TouchEventState::Press;
        } else if (x != instance->previous_touch_x_ || y != instance->previous_touch_y_) {
            instance->touch_event_state_ = TouchEventState::Move;
        } else {
            instance->touch_event_state_ = TouchEventState::Hold;
        }

        instance->last_touch_x_ = x;
        instance->last_touch_y_ = y;
        instance->previous_touch_x_ = x;
        instance->previous_touch_y_ = y;
        instance->is_touch_active_ = true;
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = x;
        data->point.y = y;
    } else {
        if (instance->is_touch_active_) {
            instance->touch_event_state_ = TouchEventState::Release;
        } else {
            instance->touch_event_state_ = TouchEventState::Idle;
        }

        instance->is_touch_active_ = false;
        data->state = LV_INDEV_STATE_RELEASED;
        data->point.x = instance->last_touch_x_;
        data->point.y = instance->last_touch_y_;
    }

    if (instance->touch_event_state_ != previous_state) {
        instance->logTouchState();
    }
}

/**
 * @brief React to LVGL button clicks by loading the screen encoded in the button user data.
 *
 * @param event LVGL event object generated by a screen navigation button.
 */
void UiDemo::navigationEventCallback(lv_event_t* event) {
    if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
        return;
    }

    UiDemo* instance = static_cast<UiDemo*>(lv_event_get_user_data(event));
    lv_obj_t* button = static_cast<lv_obj_t*>(lv_event_get_target(event));
    const ScreenId target = static_cast<ScreenId>(reinterpret_cast<uintptr_t>(lv_obj_get_user_data(button)));
    instance->activateScreen(target);
}

} // namespace ui
