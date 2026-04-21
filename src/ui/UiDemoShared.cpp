#include "ui/UiDemo.hpp"

#include "UiDemoInternal.hpp"

namespace ui {

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
    lv_obj_set_style_text_color(label, detail::uiColor(detail::kTitleTextColor), 0);
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
    lv_obj_set_size(button, 92, 38);
    lv_obj_align(button, LV_ALIGN_TOP_RIGHT, -6, 4);
    lv_obj_set_style_bg_color(button, detail::uiColor(detail::kCancelButtonColor), 0);
    lv_obj_set_style_bg_opa(button, LV_OPA_80, 0);
    lv_obj_set_style_border_color(button, detail::uiColor(0xFFB4B4), 0);
    lv_obj_set_style_border_width(button, 1, 0);
    lv_obj_set_style_radius(button, 8, 0);
    lv_obj_move_foreground(button);
    lv_obj_set_user_data(button, reinterpret_cast<void*>(static_cast<uintptr_t>(ScreenId::Start)));
    lv_obj_add_event_cb(button, navigationEventCallback, LV_EVENT_SHORT_CLICKED, this);
    lv_obj_add_event_cb(button, navigationEventCallback, LV_EVENT_CLICKED, this);
    lv_obj_add_event_cb(button, navigationEventCallback, LV_EVENT_RELEASED, this);

    lv_obj_t* label = lv_label_create(button);
    lv_label_set_text(label, "Cancel");
    lv_obj_set_style_text_color(label, detail::uiColor(detail::kButtonTextColor), 0);
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
    const uint32_t button_index = lv_obj_get_child_count(parent);
    const uint32_t column = button_index % 2U;
    const uint32_t row = button_index / 2U;
    const int32_t button_width = 122;
    const int32_t button_height = 40;
    const int32_t button_x = 12 + static_cast<int32_t>(column) * 132;
    const int32_t button_y = 12 + static_cast<int32_t>(row) * 50;

    lv_obj_t* button = lv_button_create(parent);
    lv_obj_set_size(button, button_width, button_height);
    lv_obj_set_pos(button, button_x, button_y);
    lv_obj_set_style_bg_color(button, accent_color, 0);
    lv_obj_set_style_bg_grad_dir(button, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_bg_opa(button, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(button, detail::uiColor(detail::kPrimaryTextColor), 0);
    lv_obj_set_style_border_width(button, 1, 0);
    lv_obj_set_style_radius(button, 8, 0);

    lv_obj_t* text = lv_label_create(button);
    lv_label_set_text(text, label);
    lv_obj_set_style_text_color(text, detail::uiColor(detail::kButtonTextColor), 0);
    lv_obj_center(text);

    if (enabled) {
        lv_obj_set_user_data(button, reinterpret_cast<void*>(static_cast<uintptr_t>(target)));
        lv_obj_add_event_cb(button, navigationEventCallback, LV_EVENT_CLICKED, this);
        lv_obj_add_event_cb(button, navigationEventCallback, LV_EVENT_SHORT_CLICKED, this);
        lv_obj_add_event_cb(button, navigationEventCallback, LV_EVENT_RELEASED, this);
    } else {
        lv_obj_add_state(button, LV_STATE_DISABLED);
        lv_obj_remove_flag(button, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_color(button, detail::uiColor(0x30363D), 0);
        lv_obj_set_style_bg_grad_dir(button, LV_GRAD_DIR_NONE, 0);
        lv_obj_set_style_bg_opa(button, LV_OPA_COVER, 0);
        lv_obj_set_style_border_color(button, detail::uiColor(0x6C7682), 0);
        lv_obj_set_style_text_color(text, detail::uiColor(0xC5CCD5), 0);
    }

    return button;
}

/**
 * @brief Create one caption/value row used by diagnostics-oriented screens.
 *
 * @param parent Container that should own the row.
 * @param caption Static caption shown on the left side of the row.
 *
 * @return Pointer to the value label on the right side of the row.
 */
lv_obj_t* UiDemo::createValueRow(lv_obj_t* parent, const char* caption) {
    const uint32_t existing_rows = lv_obj_get_child_count(parent) / 2U;
    const int32_t row_y = 8 + static_cast<int32_t>(existing_rows) * 22;

    lv_obj_t* caption_label = lv_label_create(parent);
    lv_label_set_text(caption_label, caption);
    lv_obj_set_style_text_color(caption_label, detail::uiColor(detail::kSecondaryTextColor), 0);
    lv_obj_set_pos(caption_label, 8, row_y);

    lv_obj_t* value_label = lv_label_create(parent);
    lv_label_set_text(value_label, "-");
    lv_obj_set_style_text_color(value_label, detail::uiColor(detail::kPrimaryTextColor), 0);
    lv_obj_set_width(value_label, 152);
    lv_obj_set_style_text_align(value_label, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(value_label, 124, row_y);
    return value_label;
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

} // namespace ui
