# Display Driver

## Purpose

This document explains how the current firmware drives the onboard LCD and how
the LovyanGFX and LVGL layers are connected.

## Source Files

- `include/display/BoardDisplay.hpp`
- `src/display/BoardDisplay.cpp`
- `include/ui/UiDemo.hpp`
- `src/ui/UiDemo.cpp`

## Layering

The display stack has two main layers:

### 1. Board-Level Driver

`display::BoardDisplay`

Responsibilities:

- configure the SPI bus
- configure the ST7789 panel
- configure the PWM backlight
- configure the CST816-family touch controller
- expose a single LovyanGFX device to the rest of the application

### 2. UI Rendering Layer

`ui::UiDemo`

Responsibilities:

- initialize LVGL
- create LVGL draw buffers
- flush LVGL pixels into the LovyanGFX display
- bind the touch callback into LVGL

This separation is intentional:

- board-specific electrical details stay in `BoardDisplay`
- screen logic stays in the UI layer

## SPI Wiring

Current display SPI configuration in `BoardDisplay.cpp`:

- `SCLK = GPIO39`
- `MOSI = GPIO38`
- `MISO = not used`
- `DC = GPIO42`
- `CS = GPIO45`
- SPI host = `SPI2_HOST`
- write frequency = `20 MHz`
- read frequency = `8 MHz`

The display is configured as write-focused:

- `pin_miso = -1`
- `readable = false`

That is deliberate because the current application does not need display readback,
and a simpler write path is more reliable for bring-up.

## Panel Geometry and Behavior

Current panel configuration:

- width = `240`
- height = `320`
- `offset_x = 0`
- `offset_y = 0`
- `offset_rotation = 0`
- `invert = true`
- `rgb_order = false`
- `bus_shared = true`

### Why these values matter

- `width` and `height` must match the physical panel
- offsets matter when a module uses a controller with hidden margins
- inversion affects whether colors look visually correct
- `rgb_order` matters if red and blue appear swapped

If colors or orientation are wrong, this block is one of the first places to
inspect.

## Backlight Control

The backlight is driven by PWM:

- pin = `GPIO1`
- PWM channel = `7`
- PWM frequency = `44100 Hz`

`BoardDisplay::forceBacklightOn()` exists as a diagnostic helper. It drives the
backlight GPIO high directly before the full panel initialization finishes.

This is useful during bring-up because it separates:

- backlight problems
- panel-controller problems

## Touch Binding at the Display Layer

The touch controller is bound directly to the panel inside `BoardDisplay`.

That means higher-level code can call:

```cpp
display_.getTouch(&x, &y)
```

without managing the I2C protocol itself.

## LVGL Display Setup

`UiDemo::initializeLvgl()` creates an LVGL display object with two partial draw
buffers.

Current buffer layout:

- two RGB565 buffers
- each sized for `320 x 20` pixels

This allows LVGL to render in chunks instead of requiring one full-screen
framebuffer.

## Color Format

The current LVGL display configuration uses:

```cpp
lv_display_set_color_format(lv_display_, LV_COLOR_FORMAT_RGB565_SWAPPED);
```

This is important.

Earlier testing showed that the unswapped RGB565 path produced incorrect color
appearance on this setup. The swapped format matches the panel/driver data path
used here.

If colors ever look wrong again after a library update, re-check this first.

## LVGL Flush Callback

LVGL renders into its draw buffers and then calls:

- `UiDemo::displayFlushCallback(...)`

The callback converts LVGL's dirty rectangle into a LovyanGFX `pushImage(...)`
operation:

```cpp
display_.pushImage(x, y, width, height, pixel_data);
```

That means:

- LVGL owns the draw scheduling
- LovyanGFX owns the final SPI transfer

## Why the Current Display Stack Works Well

This architecture keeps the responsibilities clean:

- LovyanGFX handles the hardware-specific display and touch driver details
- LVGL handles widgeting, invalidation, and partial redraws

That is a better long-term fit than drawing the entire UI manually on every
frame.

## Common Adjustment Points

If the display behaves incorrectly, check these areas:

### Wrong Colors

- `LV_COLOR_FORMAT_RGB565_SWAPPED`
- panel `rgb_order`
- panel `invert`

### Wrong Orientation

- panel rotation settings
- LVGL logical width/height
- touch rotation compensation

### Black Screen but Backlight On

- SPI pins
- `CS` and `DC`
- panel configuration block
- whether the panel is successfully initialized before the UI starts

### Visible Flicker

- LVGL invalidation behavior
- draw-buffer size
- animation update rates
- unnecessary full-screen redraws

## Recommended Future Improvements

- move theme styling into a dedicated UI theme module
- evaluate larger LVGL draw buffers if RAM budget allows
- add optional brightness control API through the HAL or display layer
- add a screen-capture/debug path if later needed for diagnostics
