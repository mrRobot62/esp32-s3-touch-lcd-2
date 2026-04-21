# Touch Input

## Purpose

This document explains how touch input is wired and processed in the current
firmware, from the board-level controller configuration up to the LVGL input
device.

## Source Files

- `include/display/BoardDisplay.hpp`
- `src/display/BoardDisplay.cpp`
- `include/ui/UiDemo.hpp`
- `src/ui/UiDemo.cpp`

## Hardware Path

The board uses a capacitive touch controller from the CST816 family on the
shared I2C bus.

Current configuration in `BoardDisplay.cpp`:

- `SDA = GPIO48`
- `SCL = GPIO47`
- I2C address = `0x15`
- bus speed = `400 kHz`
- interrupt pin = not used

The display wrapper currently uses LovyanGFX's `Touch_CST816S` integration.
Board references often mention `CST816D`. When updating LovyanGFX or porting to
another board revision, this controller-family mapping should be rechecked.

## Board-Level Touch Configuration

Touch setup happens inside `display::BoardDisplay::BoardDisplay()`.

The current touch block configures:

- X range: `0 .. 239`
- Y range: `0 .. 319`
- no interrupt pin
- no rotation compensation in the touch driver itself
- direct binding of the touch controller to the LovyanGFX panel object

Key point:

- Higher-level code does not talk to the CST816 controller directly.
- It talks to the `BoardDisplay` object through `getTouch(...)`.

## LVGL Integration

The UI layer binds touch input during `UiDemo::initializeLvgl()`.

The sequence is:

1. create LVGL display
2. create LVGL input device
3. bind the input device to the display
4. register `touchReadCallback(...)`

The relevant configuration is:

```cpp
lv_input_ = lv_indev_create();
lv_indev_set_user_data(lv_input_, this);
lv_indev_set_display(lv_input_, lv_display_);
lv_indev_set_type(lv_input_, LV_INDEV_TYPE_POINTER);
lv_indev_set_read_cb(lv_input_, touchReadCallback);
```

## Touch Read Flow

`UiDemo::touchReadCallback(...)` is the runtime bridge between LovyanGFX and
LVGL.

The callback:

1. reads coordinates through `display_.getTouch(&x, &y)`
2. determines whether the panel is currently touched
3. derives a higher-level event state
4. forwards the pointer state into LVGL
5. updates internal diagnostics state for the Touch Test page

## Derived Touch States

The UI derives a small event model for diagnostics:

- `Idle`
- `Press`
- `Move`
- `Hold`
- `Release`

These states are not part of LVGL itself. They are convenience states used by
the demo so developers can verify touch behavior quickly without attaching a
debugger.

### State Logic

- `Press`: first frame where a finger becomes active
- `Move`: active touch with changed coordinates
- `Hold`: active touch with unchanged coordinates
- `Release`: first frame after an active touch disappears
- `Idle`: no active touch and no recent release transition

## Why the Current Navigation Feels More Reliable

For this small capacitive panel, the firmware currently accepts several LVGL
button events for navigation:

- `LV_EVENT_CLICKED`
- `LV_EVENT_SHORT_CLICKED`
- `LV_EVENT_RELEASED`

This was done because the touch panel is small and the user interaction on the
physical device is less precise than a mouse click on desktop.

That more tolerant event handling is now used consistently across navigation
controls such as:

- start screen buttons
- cancel buttons
- next button on the accelerometer page

## Touch Diagnostics Screen

The `Touch Test` screen exists as a bring-up and regression tool.

It shows:

- current event state
- current X coordinate
- current Y coordinate
- whether touch is active
- a lightweight FPS estimate

This screen is valuable whenever:

- the touch controller seems dead
- coordinates appear mirrored or rotated
- hitboxes feel offset
- UI events feel unreliable

## Common Adjustment Points

If touch behavior is wrong, check these areas first:

### 1. Board-Level Controller Setup

File:

- `src/display/BoardDisplay.cpp`

Parameters to verify:

- I2C address
- SDA/SCL pins
- coordinate limits
- touch rotation offset

### 2. LVGL Input Callback

File:

- `src/ui/UiDemo.cpp`

Parameters to verify:

- the way `display_.getTouch(...)` is used
- whether last-known coordinates are kept on release
- event classification logic

### 3. UI Hitbox Sizes

If the controller works but buttons feel hard to press:

- enlarge the widgets
- avoid placing small controls too close to the glass edge
- accept multiple LVGL click-like events for navigation

## Limitations of the Current Design

The current integration is practical, but intentionally simple:

- no multitouch support
- no gesture recognition
- no interrupt-driven touch handling
- no pressure or contact-size data
- no persistent user calibration

That is acceptable for a demo and hardware validation project, but a polished
end-product may want more advanced input handling.

## Recommended Future Improvements

- optional coordinate calibration
- optional edge compensation for small buttons
- interrupt-driven wakeup for low-power modes
- reusable touch service below the current UI layer
- gesture detection for swipe-based navigation
