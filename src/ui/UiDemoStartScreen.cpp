#include "ui/UiDemo.hpp"

#include "modules/SystemInfo.hpp"
#include "UiDemoInternal.hpp"

namespace ui {

/**
 * @brief Build the start screen with direct entry points for all demo areas.
 */
void UiDemo::createStartScreen() {
    start_screen_ = lv_obj_create(nullptr);
    detail::styleScreen(start_screen_);
    createScreenTitle(start_screen_, "Demo Start");

    lv_obj_t* subtitle = lv_label_create(start_screen_);
    lv_label_set_text_fmt(subtitle, "%s", modules::SystemInfo::firmwareName());
    lv_obj_set_style_text_color(subtitle, detail::uiColor(detail::kSecondaryTextColor), 0);
    lv_obj_align(subtitle, LV_ALIGN_TOP_LEFT, 16, 34);

    lv_obj_t* panel = lv_obj_create(start_screen_);
    detail::stylePanel(panel);
    lv_obj_set_size(panel, 288, 176);
    lv_obj_align(panel, LV_ALIGN_TOP_LEFT, 16, 52);

    createMenuButton(panel, "TOUCH-TEST", ScreenId::TouchTest, detail::uiColor(0x1F8A70), true);
    createMenuButton(panel, "GRAPHIC-TEST", ScreenId::GraphicTest, detail::uiColor(0x1D6FD6), true);
    createMenuButton(panel, "FONT-TEST", ScreenId::FontTest, detail::uiColor(0x8B3DBA), true);
    createMenuButton(panel, "LVGL-WIDGETS", ScreenId::LvglWidgets, detail::uiColor(0x9A5A12), true);
    createMenuButton(panel, "ACCEL-SIMPLE", ScreenId::AccelSimple, detail::uiColor(0x4B8B2C), true);
    createMenuButton(panel, "ACCEL-GAME", ScreenId::AccelGame, detail::uiColor(0xA5492A), true);

    lv_obj_t* hint = lv_label_create(start_screen_);
    lv_label_set_text(hint, "Touch, graphics, fonts, widgets, and IMU demos.");
    lv_obj_set_style_text_color(hint, detail::uiColor(detail::kHintTextColor), 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_LEFT, 16, -8);
}

} // namespace ui
