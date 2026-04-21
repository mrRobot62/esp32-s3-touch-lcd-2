#include "ui/UiDemo.hpp"

#include "UiDemoInternal.hpp"

namespace ui {

/**
 * @brief Build the accelerometer diagnostics screen with live motion values.
 */
void UiDemo::createAccelSimpleScreen() {
    accel_simple_screen_ = lv_obj_create(nullptr);
    detail::styleScreen(accel_simple_screen_);
    createScreenTitle(accel_simple_screen_, "Accel Simple");
    createCancelButton(accel_simple_screen_);

    lv_obj_t* next_button = lv_button_create(accel_simple_screen_);
    lv_obj_set_size(next_button, 64, 28);
    lv_obj_align(next_button, LV_ALIGN_TOP_RIGHT, -82, 6);
    lv_obj_set_style_bg_color(next_button, detail::uiColor(0x285E8C), 0);
    lv_obj_set_style_border_color(next_button, detail::uiColor(detail::kPrimaryTextColor), 0);
    lv_obj_set_style_border_width(next_button, 1, 0);
    lv_obj_set_style_radius(next_button, 8, 0);
    lv_obj_set_user_data(next_button, reinterpret_cast<void*>(static_cast<uintptr_t>(ScreenId::AccelGame)));
    lv_obj_add_event_cb(next_button, navigationEventCallback, LV_EVENT_CLICKED, this);
    lv_obj_add_event_cb(next_button, navigationEventCallback, LV_EVENT_SHORT_CLICKED, this);
    lv_obj_add_event_cb(next_button, navigationEventCallback, LV_EVENT_RELEASED, this);

    lv_obj_t* next_label = lv_label_create(next_button);
    lv_label_set_text(next_label, "Next");
    lv_obj_set_style_text_color(next_label, detail::uiColor(detail::kButtonTextColor), 0);
    lv_obj_center(next_label);

    lv_obj_t* panel = lv_obj_create(accel_simple_screen_);
    detail::stylePanel(panel);
    lv_obj_set_size(panel, 288, 160);
    lv_obj_align(panel, LV_ALIGN_TOP_LEFT, 16, 46);

    accel_simple_x_label_ = createValueRow(panel, "Accel X");
    accel_simple_y_label_ = createValueRow(panel, "Accel Y");
    accel_simple_z_label_ = createValueRow(panel, "Accel Z");
    accel_simple_gx_label_ = createValueRow(panel, "Gyro X");
    accel_simple_gy_label_ = createValueRow(panel, "Gyro Y");
    accel_simple_gz_label_ = createValueRow(panel, "Gyro Z");

    accel_simple_status_label_ = lv_label_create(accel_simple_screen_);
    lv_label_set_text(accel_simple_status_label_, "IMU status: waiting for samples");
    lv_obj_set_width(accel_simple_status_label_, 288);
    lv_label_set_long_mode(accel_simple_status_label_, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(accel_simple_status_label_, detail::uiColor(detail::kHintTextColor), 0);
    lv_obj_align(accel_simple_status_label_, LV_ALIGN_BOTTOM_LEFT, 16, -8);
}

/**
 * @brief Build the accelerometer mini-game screen with a centered target.
 */
void UiDemo::createAccelGameScreen() {
    accel_game_screen_ = lv_obj_create(nullptr);
    detail::styleScreen(accel_game_screen_);
    createScreenTitle(accel_game_screen_, "Accel Game");
    createCancelButton(accel_game_screen_);

    accel_game_field_ = lv_obj_create(accel_game_screen_);
    detail::stylePanel(accel_game_field_);
    lv_obj_set_size(accel_game_field_, 206, 150);
    lv_obj_align(accel_game_field_, LV_ALIGN_CENTER, 0, 6);
    lv_obj_set_style_clip_corner(accel_game_field_, true, 0);

    accel_game_target_ = lv_obj_create(accel_game_field_);
    lv_obj_set_size(accel_game_target_, 34, 34);
    lv_obj_center(accel_game_target_);
    lv_obj_set_style_radius(accel_game_target_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(accel_game_target_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_color(accel_game_target_, detail::uiColor(0xF4C542), 0);
    lv_obj_set_style_border_width(accel_game_target_, 3, 0);

    accel_game_mover_ = lv_obj_create(accel_game_field_);
    lv_obj_set_size(accel_game_mover_, 22, 22);
    lv_obj_center(accel_game_mover_);
    lv_obj_set_style_radius(accel_game_mover_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(accel_game_mover_, detail::uiColor(0x4D96FF), 0);
    lv_obj_set_style_border_width(accel_game_mover_, 0, 0);

    accel_game_status_label_ = lv_label_create(accel_game_screen_);
    lv_label_set_text(accel_game_status_label_, "Move the board until the blue marker reaches the center.");
    lv_obj_set_width(accel_game_status_label_, 288);
    lv_label_set_long_mode(accel_game_status_label_, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(accel_game_status_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(accel_game_status_label_, detail::uiColor(detail::kHintTextColor), 0);
    lv_obj_align(accel_game_status_label_, LV_ALIGN_BOTTOM_MID, 0, -8);
}

/**
 * @brief Refresh the accelerometer diagnostics screen with live motion values.
 */
void UiDemo::updateAccelSimpleScreen() {
    const unsigned long now = millis();
    if (accel_last_sample_at_ != 0 && (now - accel_last_sample_at_) < detail::kAccelUpdateIntervalMs) {
        return;
    }

    accel_last_sample_at_ = now;
    if (!refreshImuSample()) {
        if (hal_.isImuReady()) {
            lv_label_set_text(accel_simple_status_label_, "IMU status: sensor configured, but no fresh sample available");
        } else {
            lv_label_set_text(accel_simple_status_label_, "IMU status: sensor not initialized");
        }
        return;
    }

    detail::setFormattedFloatLabel(accel_simple_x_label_, "%.3f g", last_imu_sample_.accel_x_g);
    detail::setFormattedFloatLabel(accel_simple_y_label_, "%.3f g", last_imu_sample_.accel_y_g);
    detail::setFormattedFloatLabel(accel_simple_z_label_, "%.3f g", last_imu_sample_.accel_z_g);
    detail::setFormattedFloatLabel(accel_simple_gx_label_, "%.1f dps", last_imu_sample_.gyro_x_dps);
    detail::setFormattedFloatLabel(accel_simple_gy_label_, "%.1f dps", last_imu_sample_.gyro_y_dps);
    detail::setFormattedFloatLabel(accel_simple_gz_label_, "%.1f dps", last_imu_sample_.gyro_z_dps);
    lv_label_set_text(accel_simple_status_label_, "IMU status: live QMI8658 accelerometer and gyroscope data");
}

/**
 * @brief Refresh the accelerometer mini-game and blink when the target is reached.
 */
void UiDemo::updateAccelGameScreen() {
    const unsigned long now = millis();
    if (accel_last_sample_at_ == 0 || (now - accel_last_sample_at_) >= detail::kAccelUpdateIntervalMs) {
        accel_last_sample_at_ = now;
        if (refreshImuSample()) {
            const int32_t mover_width = static_cast<int32_t>(lv_obj_get_width(accel_game_mover_));
            const int32_t mover_height = static_cast<int32_t>(lv_obj_get_height(accel_game_mover_));
            const int32_t field_width = static_cast<int32_t>(lv_obj_get_width(accel_game_field_));
            const int32_t field_height = static_cast<int32_t>(lv_obj_get_height(accel_game_field_));
            const int32_t max_offset_x = (field_width - mover_width) / 2;
            const int32_t max_offset_y = (field_height - mover_height) / 2;
            const int32_t offset_x =
                static_cast<int32_t>(constrain(last_imu_sample_.accel_x_g * 78.0f, -max_offset_x, max_offset_x));
            const int32_t offset_y =
                static_cast<int32_t>(constrain(-last_imu_sample_.accel_y_g * 78.0f, -max_offset_y, max_offset_y));
            lv_obj_align(accel_game_mover_, LV_ALIGN_CENTER, offset_x, offset_y);

            const bool inside_target = (abs(offset_x) <= 8) && (abs(offset_y) <= 8);
            if (inside_target && accel_game_blink_cycles_remaining_ == 0) {
                accel_game_blink_cycles_remaining_ = 6;
                accel_game_last_blink_at_ = now;
                lv_label_set_text(accel_game_status_label_, "Centered. Hold it steady while the marker blinks.");
            } else if (!inside_target && accel_game_blink_cycles_remaining_ == 0) {
                lv_label_set_text(accel_game_status_label_, "Tilt the board until the blue marker reaches the yellow target.");
            }
        } else {
            if (hal_.isImuReady()) {
                lv_label_set_text(accel_game_status_label_, "IMU status: sensor configured, but no fresh sample available");
            } else {
                lv_label_set_text(accel_game_status_label_, "IMU status: sensor not initialized");
            }
        }
    }

    if (accel_game_blink_cycles_remaining_ > 0 && (now - accel_game_last_blink_at_) >= detail::kAccelBlinkIntervalMs) {
        accel_game_last_blink_at_ = now;
        accel_game_marker_visible_ = !accel_game_marker_visible_;
        if (accel_game_marker_visible_) {
            lv_obj_clear_flag(accel_game_mover_, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(accel_game_mover_, LV_OBJ_FLAG_HIDDEN);
        }

        --accel_game_blink_cycles_remaining_;
        if (accel_game_blink_cycles_remaining_ == 0) {
            accel_game_marker_visible_ = true;
            lv_obj_clear_flag(accel_game_mover_, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(accel_game_status_label_, "Blink complete. Move away and center the marker again.");
        }
    }
}

} // namespace ui
