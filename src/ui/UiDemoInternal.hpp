#pragma once

#include <array>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <utility>

#include <Arduino.h>
#include <lvgl.h>

namespace ui::detail {

/** @brief Logical width of the rotated display in pixels. */
inline constexpr int32_t kScreenWidth = 320;
/** @brief Logical height of the rotated display in pixels. */
inline constexpr int32_t kScreenHeight = 240;
/** @brief Frame interval used by the touch diagnostics screen. */
inline constexpr unsigned long kTouchUpdateIntervalMs = 33UL;
/** @brief Frame interval used by the animated graphics screen. */
inline constexpr unsigned long kGraphicUpdateIntervalMs = 16UL;
/** @brief Scene rotation interval for most graphics phases in milliseconds. */
inline constexpr unsigned long kGraphicPhaseIntervalMs = 3000UL;
/** @brief Extended interval for the sprite scene so the marker can cross the full width. */
inline constexpr unsigned long kGraphicSpritePhaseIntervalMs = 5000UL;
/** @brief Presentation rotation interval for the font test in milliseconds. */
inline constexpr unsigned long kFontPhaseIntervalMs = 3000UL;
/** @brief Update interval used by the accelerometer demo pages. */
inline constexpr unsigned long kAccelUpdateIntervalMs = 33UL;
/** @brief Blink interval used when the accelerometer game marker reaches the target. */
inline constexpr unsigned long kAccelBlinkIntervalMs = 120UL;
/** @brief Number of graphics phases presented by the demo. */
inline constexpr uint8_t kGraphicPhaseCount = 6U;
/** @brief Number of font phases presented by the demo. */
inline constexpr uint8_t kFontPhaseCount = 4U;
/** @brief Base screen background color used across the whole demo. */
inline constexpr uint32_t kScreenBackgroundColor = 0x000000U;
/** @brief Secondary dark gray used for subtle solid accents without color tinting. */
inline constexpr uint32_t kScreenGradientColor = 0x101010U;
/** @brief Dark solid panel color that separates content from the screen background. */
inline constexpr uint32_t kPanelBackgroundColor = 0x161616U;
/** @brief Border color that keeps dark panels visually separated from the background. */
inline constexpr uint32_t kPanelBorderColor = 0x6F879CU;
/** @brief High-contrast title color used for screen headers. */
inline constexpr uint32_t kTitleTextColor = 0xF2F7FFU;
/** @brief High-contrast primary text color used for most labels. */
inline constexpr uint32_t kPrimaryTextColor = 0xEAF2FFU;
/** @brief Slightly dimmed text color used for secondary information. */
inline constexpr uint32_t kSecondaryTextColor = 0xC4D3E6U;
/** @brief Warm accent text color used for hints and state descriptors. */
inline constexpr uint32_t kHintTextColor = 0xFFD38AU;
/** @brief Bright border/text accent used for active menu buttons. */
inline constexpr uint32_t kButtonTextColor = 0xF8FBFFU;
/** @brief Dark red cancel button background with strong contrast. */
inline constexpr uint32_t kCancelButtonColor = 0x7A2020U;
/** @brief Background color used inside the sprite-sequence canvas. */
inline constexpr uint32_t kSpriteCanvasBackgroundColor = 0x0B1220U;
/** @brief Accent color used for the sprite trail and star field. */
inline constexpr uint32_t kSpriteCanvasAccentColor = 0x24405FU;
/** @brief Main fill color of the animated sprite body. */
inline constexpr uint32_t kSpritePrimaryColor = 0x59D36FU;
/** @brief Secondary fill color of the animated sprite body. */
inline constexpr uint32_t kSpriteSecondaryColor = 0xF4C542U;
/** @brief Highlight color used for the sprite face and sparkles. */
inline constexpr uint32_t kSpriteHighlightColor = 0xF2F7FFU;
/** @brief Number of pixel-art frames stored for the sprite animation. */
inline constexpr uint8_t kSpriteFrameCount = 4U;

/** @brief Pixel-art row masks for the first sprite frame. */
inline constexpr std::array<uint16_t, 16> kSpriteFrame0 = {
    0b0000001111000000,
    0b0000011111100000,
    0b0000111111110000,
    0b0001111111111000,
    0b0011111111111100,
    0b0011111111111100,
    0b0111111111111110,
    0b0111111101111110,
    0b0111111111111110,
    0b0011111111111100,
    0b0011111111111100,
    0b0001111100111000,
    0b0000111000010000,
    0b0001110000111000,
    0b0011100001111100,
    0b0001000000010000,
};

/** @brief Pixel-art row masks for the second sprite frame. */
inline constexpr std::array<uint16_t, 16> kSpriteFrame1 = {
    0b0000001111000000,
    0b0000011111100000,
    0b0000111111110000,
    0b0001111111111000,
    0b0011111111111100,
    0b0011111111111100,
    0b0111111111111110,
    0b0111111101111110,
    0b0111111111111110,
    0b0011111111111100,
    0b0011111111111100,
    0b0001111100111000,
    0b0000011001110000,
    0b0000110001111000,
    0b0001110011110000,
    0b0010000010000000,
};

/** @brief Pixel-art row masks for the third sprite frame. */
inline constexpr std::array<uint16_t, 16> kSpriteFrame2 = {
    0b0000001111000000,
    0b0000011111100000,
    0b0000111111110000,
    0b0001111111111000,
    0b0011111111111100,
    0b0011111111111100,
    0b0111111111111110,
    0b0111111101111110,
    0b0111111111111110,
    0b0011111111111100,
    0b0011111111111100,
    0b0001111100111000,
    0b0000111000010000,
    0b0011100001111100,
    0b0001110000111000,
    0b0000010000001000,
};

/** @brief Pixel-art row masks for the fourth sprite frame. */
inline constexpr std::array<uint16_t, 16> kSpriteFrame3 = {
    0b0000001111000000,
    0b0000011111100000,
    0b0000111111110000,
    0b0001111111111000,
    0b0011111111111100,
    0b0011111111111100,
    0b0111111111111110,
    0b0111111101111110,
    0b0111111111111110,
    0b0011111111111100,
    0b0011111111111100,
    0b0001111100111000,
    0b0000011001110000,
    0b0001110011110000,
    0b0000110001111000,
    0b0000001000010000,
};

/** @brief Collection of all sprite frames used by the frame-sequence scene. */
inline constexpr std::array<std::array<uint16_t, 16>, kSpriteFrameCount> kSpriteFrames = {
    kSpriteFrame0,
    kSpriteFrame1,
    kSpriteFrame2,
    kSpriteFrame3,
};

/** @brief Static star coordinates rendered behind the moving sprite. */
inline constexpr std::array<std::pair<int16_t, int16_t>, 10> kSpriteStars = {{
    {8, 10}, {26, 18}, {44, 8}, {74, 14}, {102, 20},
    {18, 64}, {40, 78}, {70, 66}, {92, 54}, {114, 74},
}};

/**
 * @brief Return a reusable LVGL color from a 24-bit RGB hex value.
 *
 * @param rgb 24-bit RGB color value in `0xRRGGBB` form.
 *
 * @return LVGL color converted for the active color format.
 */
inline lv_color_t uiColor(uint32_t rgb) { return lv_color_hex(rgb); }

/**
 * @brief Apply a consistent background theme to a root LVGL screen.
 *
 * @param screen Screen object that should receive the base background styling.
 */
inline void styleScreen(lv_obj_t* screen) {
    lv_obj_set_style_bg_color(screen, uiColor(kScreenBackgroundColor), 0);
    lv_obj_set_style_bg_grad_color(screen, uiColor(kScreenGradientColor), 0);
    lv_obj_set_style_bg_grad_dir(screen, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
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
inline void stylePanel(lv_obj_t* container) {
    lv_obj_set_style_bg_color(container, uiColor(kPanelBackgroundColor), 0);
    lv_obj_set_style_bg_grad_dir(container, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(container, uiColor(kPanelBorderColor), 0);
    lv_obj_set_style_border_width(container, 2, 0);
    lv_obj_set_style_radius(container, 10, 0);
    lv_obj_set_style_pad_all(container, 10, 0);
    lv_obj_set_style_shadow_width(container, 0, 0);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(container, LV_SCROLLBAR_MODE_OFF);
}

/**
 * @brief Format one floating-point sensor value and apply it to an LVGL label.
 *
 * @param label Target LVGL label that should display the formatted value.
 * @param format `printf`-style format string for a single floating-point value.
 * @param value Floating-point value that should be rendered into the label.
 */
inline void setFormattedFloatLabel(lv_obj_t* label, const char* format, float value) {
    char buffer[32] = {};
    snprintf(buffer, sizeof(buffer), format, static_cast<double>(value));
    lv_label_set_text(label, buffer);
}

/**
 * @brief Return a short human-readable label for a graphics scene index.
 *
 * @param phase Graphics phase index that should be translated.
 *
 * @return Static string label that describes the scene.
 */
inline const char* graphicPhaseLabel(uint8_t phase) {
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
        case 5:
            return "Sprite Sequence";
        default:
            return "Unknown";
    }
}

/**
 * @brief Return the correct sprite frame for the requested frame index.
 *
 * @param frame_index Index of the frame within the looping animation.
 *
 * @return Reference to the 16-row pixel-art frame mask.
 */
inline const std::array<uint16_t, 16>& spriteFrame(uint8_t frame_index) {
    return kSpriteFrames[frame_index % kSpriteFrameCount];
}

/**
 * @brief Draw one scaled pixel-art sprite frame onto the target LVGL canvas.
 *
 * @param canvas Canvas object that receives the rendered sprite.
 * @param origin_x Left position of the sprite in canvas pixels.
 * @param origin_y Top position of the sprite in canvas pixels.
 * @param scale Integer scale factor applied to every sprite pixel.
 * @param frame Row-mask data that describes the current sprite frame.
 */
inline void drawSpriteFrame(
    lv_obj_t* canvas,
    int32_t origin_x,
    int32_t origin_y,
    int32_t scale,
    const std::array<uint16_t, 16>& frame) {
    for (std::size_t row = 0; row < frame.size(); ++row) {
        for (uint8_t column = 0; column < 16U; ++column) {
            const bool pixel_on = (frame[row] & (1U << (15U - column))) != 0U;
            if (!pixel_on) {
                continue;
            }

            lv_color_t pixel_color = uiColor(kSpritePrimaryColor);
            if (row < 4U) {
                pixel_color = uiColor(kSpriteSecondaryColor);
            } else if (row == 7U && (column == 5U || column == 10U)) {
                pixel_color = uiColor(kSpriteHighlightColor);
            } else if (row > 10U) {
                pixel_color = uiColor(kSpriteSecondaryColor);
            }

            for (int32_t dy = 0; dy < scale; ++dy) {
                for (int32_t dx = 0; dx < scale; ++dx) {
                    lv_canvas_set_px(
                        canvas,
                        origin_x + static_cast<int32_t>(column) * scale + dx,
                        origin_y + static_cast<int32_t>(row) * scale + dy,
                        pixel_color,
                        LV_OPA_COVER);
                }
            }
        }
    }
}

} // namespace ui::detail
