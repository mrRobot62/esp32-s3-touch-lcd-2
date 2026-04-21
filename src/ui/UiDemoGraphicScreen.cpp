#include "ui/UiDemo.hpp"

#include "UiDemoInternal.hpp"

namespace ui {

/**
 * @brief Build the graphics performance screen with multiple animated LVGL scenes.
 */
void UiDemo::createGraphicScreen() {
    graphic_screen_ = lv_obj_create(nullptr);
    detail::styleScreen(graphic_screen_);
    createScreenTitle(graphic_screen_, "Graphic Test");
    createCancelButton(graphic_screen_);

    graphic_scene_label_ = lv_label_create(graphic_screen_);
    lv_label_set_text(graphic_scene_label_, "Scene: Solid Colors");
    lv_obj_set_style_text_color(graphic_scene_label_, detail::uiColor(detail::kSecondaryTextColor), 0);
    lv_obj_align(graphic_scene_label_, LV_ALIGN_TOP_LEFT, 16, 32);

    graphic_viewport_ = lv_obj_create(graphic_screen_);
    detail::stylePanel(graphic_viewport_);
    lv_obj_set_size(graphic_viewport_, 288, 164);
    lv_obj_align(graphic_viewport_, LV_ALIGN_TOP_LEFT, 16, 52);
    lv_obj_set_style_clip_corner(graphic_viewport_, true, 0);

    graphic_scene_solid_ = lv_obj_create(graphic_viewport_);
    lv_obj_remove_style_all(graphic_scene_solid_);
    lv_obj_set_size(graphic_scene_solid_, 268, 142);
    lv_obj_align(graphic_scene_solid_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(graphic_scene_solid_, LV_OBJ_FLAG_SCROLLABLE);

    static const lv_color_t solid_colors[4] = {
        detail::uiColor(0xD62828),
        detail::uiColor(0x2A9D8F),
        detail::uiColor(0x1565C0),
        detail::uiColor(0xFFB703),
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
        lv_obj_set_style_text_color(label, detail::uiColor(detail::kPrimaryTextColor), 0);
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
        lv_obj_set_style_bg_color(graphic_bars_[index], detail::uiColor(0x6BCB77), 0);
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
            detail::uiColor((index % 2U == 0U) ? 0xFF6B6B : 0x4D96FF),
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

    graphic_scene_sprite_ = lv_obj_create(graphic_viewport_);
    lv_obj_remove_style_all(graphic_scene_sprite_);
    lv_obj_set_size(graphic_scene_sprite_, 268, 142);
    lv_obj_align(graphic_scene_sprite_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(graphic_scene_sprite_, LV_OBJ_FLAG_SCROLLABLE);

    graphic_sprite_canvas_ = lv_canvas_create(graphic_scene_sprite_);
    lv_canvas_set_buffer(
        graphic_sprite_canvas_,
        graphic_sprite_canvas_buffer_.data(),
        static_cast<int32_t>(kSpriteCanvasWidth),
        static_cast<int32_t>(kSpriteCanvasHeight),
        LV_COLOR_FORMAT_RGB565);
    lv_canvas_fill_bg(graphic_sprite_canvas_, detail::uiColor(detail::kSpriteCanvasBackgroundColor), LV_OPA_COVER);
    lv_obj_center(graphic_sprite_canvas_);
    lv_obj_set_style_border_width(graphic_sprite_canvas_, 0, 0);
    lv_obj_set_style_bg_opa(graphic_sprite_canvas_, LV_OPA_TRANSP, 0);

    graphic_fps_label_ = lv_label_create(graphic_screen_);
    lv_label_set_text(graphic_fps_label_, "FPS: 0");
    lv_obj_set_style_text_color(graphic_fps_label_, detail::uiColor(detail::kPrimaryTextColor), 0);
    lv_obj_align(graphic_fps_label_, LV_ALIGN_BOTTOM_RIGHT, -8, -10);

    lv_obj_t* hint = lv_label_create(graphic_screen_);
    lv_label_set_text(hint, "The active scene changes every 3 seconds.");
    lv_obj_set_style_text_color(hint, detail::uiColor(detail::kHintTextColor), 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_LEFT, 16, -10);

    applyGraphicPhase();
}

/**
 * @brief Refresh the active graphics scene and rotate scenes every three seconds.
 */
void UiDemo::updateGraphicScreen() {
    const unsigned long now = millis();
    if (graphic_last_update_at_ != 0 && (now - graphic_last_update_at_) < detail::kGraphicUpdateIntervalMs) {
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
        case 5:
            updateGraphicSpriteSequence(now);
            break;
        default:
            break;
    }

    lv_label_set_text_fmt(graphic_fps_label_, "FPS: %u", graphic_fps_);
}

/**
 * @brief Rotate the graphics scene after the configured interval elapsed.
 */
void UiDemo::rotateGraphicPhaseIfNeeded() {
    const unsigned long now = millis();
    const unsigned long phase_interval =
        (graphic_phase_ == 5U) ? detail::kGraphicSpritePhaseIntervalMs : detail::kGraphicPhaseIntervalMs;
    if ((now - graphic_last_phase_at_) < phase_interval) {
        return;
    }

    graphic_phase_ = static_cast<uint8_t>((graphic_phase_ + 1U) % detail::kGraphicPhaseCount);
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
    const bool show_sprite = graphic_phase_ == 5U;

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

    if (show_sprite) {
        lv_obj_clear_flag(graphic_scene_sprite_, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(graphic_scene_sprite_, LV_OBJ_FLAG_HIDDEN);
    }

    lv_label_set_text_fmt(graphic_scene_label_, "Scene: %s", detail::graphicPhaseLabel(graphic_phase_));
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

        const uint32_t hue_shift = static_cast<uint32_t>(phase * 9.0f);
        lv_obj_set_style_bg_color(
            graphic_bars_[index],
            detail::uiColor(0x2040FFU + ((hue_shift * 73U) & 0x00FFFFU)),
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
    const float progress =
        static_cast<float>(now % detail::kGraphicPhaseIntervalMs) / static_cast<float>(detail::kGraphicPhaseIntervalMs);
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
        lv_obj_set_style_border_color(graphic_nested_boxes_[index], detail::uiColor(colors[index]), 0);
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
        lv_obj_set_style_bg_color(graphic_rainbow_strips_[index], detail::uiColor(palette[palette_index]), 0);
    }
}

/**
 * @brief Update the native LVGL sprite-sequence canvas with frame-by-frame animation.
 *
 * @param now Current millisecond timestamp used to derive motion, frame index, and sparkles.
 */
void UiDemo::updateGraphicSpriteSequence(unsigned long now) {
    lv_canvas_fill_bg(graphic_sprite_canvas_, detail::uiColor(detail::kSpriteCanvasBackgroundColor), LV_OPA_COVER);

    for (std::size_t index = 0; index < detail::kSpriteStars.size(); ++index) {
        const int32_t twinkle = static_cast<int32_t>((now / 120UL + index * 3U) % 4U);
        const lv_color_t star_color =
            (twinkle < 2) ? detail::uiColor(detail::kSpriteHighlightColor) : detail::uiColor(detail::kSpriteCanvasAccentColor);
        lv_canvas_set_px(
            graphic_sprite_canvas_,
            detail::kSpriteStars[index].first,
            detail::kSpriteStars[index].second,
            star_color,
            LV_OPA_COVER);
        if ((twinkle % 2) == 0) {
            lv_canvas_set_px(
                graphic_sprite_canvas_,
                detail::kSpriteStars[index].first + 1,
                detail::kSpriteStars[index].second,
                star_color,
                LV_OPA_COVER);
        }
    }

    const unsigned long elapsed_in_scene = now - graphic_last_phase_at_;
    const float progress = static_cast<float>(elapsed_in_scene % detail::kGraphicSpritePhaseIntervalMs) /
                           static_cast<float>(detail::kGraphicSpritePhaseIntervalMs);
    const uint8_t frame_index = static_cast<uint8_t>((now / 120UL) % detail::kSpriteFrameCount);
    const int32_t sprite_x = static_cast<int32_t>(progress * static_cast<float>(kSpriteCanvasWidth - 64U));
    const int32_t bobbing = static_cast<int32_t>(6.0f * sinf(static_cast<float>(now) * 0.012f));

    for (int32_t trail = 0; trail < 3; ++trail) {
        const int32_t trail_x = sprite_x - 14 - trail * 10;
        const int32_t trail_y = 54 + bobbing + trail * 3;
        if (trail_x < 0) {
            continue;
        }

        for (int32_t offset = 0; offset < 3 - trail; ++offset) {
            lv_canvas_set_px(
                graphic_sprite_canvas_,
                trail_x - offset,
                trail_y + offset,
                detail::uiColor(detail::kSpriteCanvasAccentColor),
                LV_OPA_COVER);
        }
    }

    detail::drawSpriteFrame(graphic_sprite_canvas_, sprite_x, 18 + bobbing, 4, detail::spriteFrame(frame_index));
}

} // namespace ui
