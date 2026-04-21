#include "ui/UiDemo.hpp"

#include "UiDemoInternal.hpp"

namespace ui {

/**
 * @brief Build the font showcase screen with reusable labels for the rotating phases.
 */
void UiDemo::createFontScreen() {
    font_screen_ = lv_obj_create(nullptr);
    detail::styleScreen(font_screen_);
    createScreenTitle(font_screen_, "Font Test");
    createCancelButton(font_screen_);

    font_title_label_ = lv_label_create(font_screen_);
    lv_label_set_text(font_title_label_, "Phase: Compact Info");
    lv_obj_set_style_text_color(font_title_label_, detail::uiColor(detail::kSecondaryTextColor), 0);
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
    lv_obj_set_style_text_color(font_info_label_, detail::uiColor(detail::kHintTextColor), 0);
    lv_obj_align(font_info_label_, LV_ALIGN_BOTTOM_MID, 0, -14);

    applyFontPhase();
}

/**
 * @brief Rotate the font demonstration after the configured three-second interval elapsed.
 */
void UiDemo::rotateFontPhaseIfNeeded() {
    const unsigned long now = millis();
    if ((now - font_last_phase_at_) < detail::kFontPhaseIntervalMs) {
        return;
    }

    font_phase_ = static_cast<uint8_t>((font_phase_ + 1U) % detail::kFontPhaseCount);
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
            lv_obj_set_style_text_color(font_sample_label_, detail::uiColor(0xFFD166), 0);
            break;

        case 1:
            lv_label_set_text(font_title_label_, "Phase: Balanced Body");
            lv_label_set_text(font_sample_label_, "A brown fox jumps over the lazy dog.");
            lv_label_set_text(font_info_label_, "Montserrat 16 for readable default body text.");
            lv_obj_set_style_text_font(font_sample_label_, &lv_font_montserrat_16, 0);
            lv_obj_set_style_text_color(font_sample_label_, detail::uiColor(0xCFE8FF), 0);
            break;

        case 2:
            lv_label_set_text(font_title_label_, "Phase: Emphasized Heading");
            lv_label_set_text(font_sample_label_, "A brown fox jumps over the lazy dog.");
            lv_label_set_text(font_info_label_, "Montserrat 20 with a strong cyan emphasis.");
            lv_obj_set_style_text_font(font_sample_label_, &lv_font_montserrat_20, 0);
            lv_obj_set_style_text_color(font_sample_label_, detail::uiColor(0x6AD5FF), 0);
            break;

        case 3:
        default:
            lv_label_set_text(font_title_label_, "Phase: Bold Showcase");
            lv_label_set_text(font_sample_label_, "A brown fox jumps over the lazy dog.");
            lv_label_set_text(font_info_label_, "Montserrat 24 for prominent hero copy.");
            lv_obj_set_style_text_font(font_sample_label_, &lv_font_montserrat_24, 0);
            lv_obj_set_style_text_color(font_sample_label_, detail::uiColor(0xFF7B72), 0);
            break;
    }
}

} // namespace ui
