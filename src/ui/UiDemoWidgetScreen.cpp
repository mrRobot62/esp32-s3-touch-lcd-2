#include "ui/UiDemo.hpp"

#include "UiDemoInternal.hpp"

namespace ui {

/**
 * @brief Build the widget showcase screen with real LVGL widgets.
 */
void UiDemo::createWidgetScreen() {
    widget_screen_ = lv_obj_create(nullptr);
    detail::styleScreen(widget_screen_);
    createScreenTitle(widget_screen_, "LVGL Widgets");
    createCancelButton(widget_screen_);

    lv_obj_t* info = lv_label_create(widget_screen_);
    lv_label_set_text(info, "This page shows real LVGL widgets without custom event handling.");
    lv_obj_set_width(info, 280);
    lv_label_set_long_mode(info, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(info, detail::uiColor(detail::kSecondaryTextColor), 0);
    lv_obj_align(info, LV_ALIGN_TOP_LEFT, 16, 34);

    lv_obj_t* left_panel = lv_obj_create(widget_screen_);
    detail::stylePanel(left_panel);
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
    detail::stylePanel(right_panel);
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

} // namespace ui
