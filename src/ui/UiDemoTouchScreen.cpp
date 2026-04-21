#include "ui/UiDemo.hpp"

#include "UiDemoInternal.hpp"

namespace ui {

/**
 * @brief Build the touch diagnostics screen with labels for the live touch state.
 */
void UiDemo::createTouchScreen() {
    touch_screen_ = lv_obj_create(nullptr);
    detail::styleScreen(touch_screen_);
    createScreenTitle(touch_screen_, "Touch Test");
    createCancelButton(touch_screen_);

    lv_obj_t* panel = lv_obj_create(touch_screen_);
    detail::stylePanel(panel);
    lv_obj_set_size(panel, 288, 160);
    lv_obj_align(panel, LV_ALIGN_TOP_LEFT, 16, 46);

    touch_event_value_label_ = createValueRow(panel, "Event");
    touch_x_value_label_ = createValueRow(panel, "X");
    touch_y_value_label_ = createValueRow(panel, "Y");
    touch_active_value_label_ = createValueRow(panel, "Active");

    lv_obj_t* hint = lv_label_create(touch_screen_);
    lv_label_set_text(hint, "Touch the panel to verify coordinates and event transitions.");
    lv_obj_set_width(hint, 250);
    lv_obj_set_style_text_color(hint, detail::uiColor(detail::kSecondaryTextColor), 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_LEFT, 16, -10);

    touch_fps_label_ = lv_label_create(touch_screen_);
    lv_label_set_text(touch_fps_label_, "FPS: 0");
    lv_obj_set_style_text_color(touch_fps_label_, detail::uiColor(detail::kPrimaryTextColor), 0);
    lv_obj_align(touch_fps_label_, LV_ALIGN_BOTTOM_RIGHT, -8, -10);
}

/**
 * @brief Refresh the touch diagnostics values at a stable low-overhead cadence.
 */
void UiDemo::updateTouchScreen() {
    const unsigned long now = millis();
    if (touch_last_sample_at_ != 0 && (now - touch_last_sample_at_) < detail::kTouchUpdateIntervalMs) {
        return;
    }

    updateFpsValue(touch_fps_, touch_last_sample_at_, now);
    lv_label_set_text(touch_event_value_label_, touchEventLabel(touch_event_state_));
    lv_label_set_text_fmt(touch_x_value_label_, "%u", last_touch_x_);
    lv_label_set_text_fmt(touch_y_value_label_, "%u", last_touch_y_);
    lv_label_set_text(touch_active_value_label_, is_touch_active_ ? "yes" : "no");
    lv_label_set_text_fmt(touch_fps_label_, "FPS: %u", touch_fps_);
}

} // namespace ui
