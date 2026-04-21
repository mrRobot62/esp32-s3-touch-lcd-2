# QMI8658 IMU

## Purpose

This document explains how the onboard QMI8658 6-axis IMU is integrated into
the current firmware through `hal::HalContext`.

The chip provides:

- 3-axis accelerometer
- 3-axis gyroscope

The current firmware uses it for:

- live value diagnostics on `ACCEL-SIMPLE`
- motion-driven marker control on `ACCEL-GAME`

## Source Files

- `include/hal/HalContext.hpp`
- `src/hal/HalContext.cpp`
- `include/ui/UiDemo.hpp`
- `src/ui/UiDemo.cpp`

## Bus Topology

The QMI8658 shares the same I2C bus as the touch controller:

- `SDA = GPIO48`
- `SCL = GPIO47`
- bus speed = `400 kHz`

This shared-bus detail is critical. The application currently initializes the
display first and the HAL second so the board's touch/display stack is already
in a known-good state before IMU bring-up happens.

## IMU Detection

`HalContext::setupImu()` probes the two common QMI8658 addresses:

- `0x6A`
- `0x6B`

The probe sequence reads:

- `WHO_AM_I` register `0x00`
- revision register `0x01` when available

The current firmware expects:

- `WHO_AM_I = 0x05`

If no expected response is found, the IMU remains unavailable and the UI shows
that state rather than crashing.

## Register Map Used by the Current Firmware

The current implementation explicitly uses the following registers:

- `0x00` `WHO_AM_I`
- `0x01` revision
- `0x02` `CTRL1`
- `0x03` `CTRL2`
- `0x04` `CTRL3`
- `0x08` `CTRL7`
- `0x2E` `STATUS0`
- `0x35` accelerometer output start register

The data block starting at `0x35` is read as 12 bytes:

- accel X low/high
- accel Y low/high
- accel Z low/high
- gyro X low/high
- gyro Y low/high
- gyro Z low/high

## Current Configuration Profile

After detection, the firmware writes:

- `CTRL1 = 0x60`
- `CTRL2 = 0x23`
- `CTRL3 = 0x43`
- `CTRL7 = 0x03`

This profile is not intended to be a universal final configuration. It is a
practical demo configuration that enables both accelerometer and gyroscope with
stable enough behavior for UI demos.

If you need different ranges, output rates, or power modes, update these values
first and then verify the scale factors used for conversion.

## Data-Ready Handling

Before reading a sample, the HAL checks `STATUS0`.

The current data-ready mask is:

- `0x03`

This means the firmware only consumes samples when the QMI8658 reports fresh
accelerometer or gyroscope data.

The runtime method retries a small number of times before reporting that no
fresh sample is available.

## Unit Conversion

Raw signed 16-bit values are converted into engineering units using the current
scale constants:

- accelerometer scale: `1 / 4096`
- gyroscope scale: `1 / 64`

The returned sample structure is:

```cpp
struct ImuSample {
    float accel_x_g;
    float accel_y_g;
    float accel_z_g;
    float gyro_x_dps;
    float gyro_y_dps;
    float gyro_z_dps;
};
```

## Runtime Filtering

The current firmware does not expose raw noisy IMU values directly to the UI.
It applies filtering inside the HAL.

### Low-Pass Filter

Current smoothing constants:

- accelerometer alpha: `0.18`
- gyroscope alpha: `0.12`

This reduces visible jitter and makes the values more usable for the demo
screens.

### Deadband

Current deadband constants:

- accelerometer around `0 g`: `0.018 g`
- gyroscope around `0 dps`: `0.8 dps`
- accelerometer `Z` around gravity `1.0 g`: `0.018 g`

This prevents tiny sensor noise from looking like real motion when the board is
lying still.

## Axis Interpretation

The HAL only returns filtered numeric values. The UI decides how to interpret
them visually.

For example, `ACCEL-GAME` currently maps:

- `accel_x_g` to horizontal movement
- inverted `accel_y_g` to vertical movement

The Y inversion was added because the intuitive screen direction did not match
the first raw implementation.

That means:

- sensor orientation problems can be corrected either in the UI mapping or in a
  dedicated orientation layer later
- the current HAL does not yet provide a board-orientation abstraction

## Bootstrap Behavior

`setupImu()` does not only configure the chip. It also waits briefly for a
first sample to become available.

This is useful because it confirms:

- the device answered on I2C
- configuration writes succeeded
- the sensor actually started producing data

If no ready sample appears during bootstrap, the firmware logs a warning but can
still continue. This is useful for bring-up because it avoids a hard failure
too early in the boot sequence.

## UI Consumers

### ACCEL-SIMPLE

This page shows the latest filtered values for:

- accel X/Y/Z in `g`
- gyro X/Y/Z in `dps`

It exists mainly for verification and tuning.

### ACCEL-GAME

This page uses filtered accelerometer data to move a marker inside a playfield.

The design goal is not scientific precision. It is an intuitive demonstration
that shows whether:

- sampling works
- motion mapping works
- values are stable enough for a small interaction demo

## Common Adjustment Points

If the IMU behavior feels wrong, check these in order:

1. bus wiring and shared-bus initialization order
2. detected I2C address
3. `WHO_AM_I` value
4. control register values
5. scale factors
6. low-pass and deadband constants
7. UI axis mapping

## Recommended Future Improvements

- explicit board-orientation abstraction
- calibration offsets stored in non-volatile storage
- selectable filter strength
- optional raw-value debug mode
- reusable sensor service for modules outside the UI
- interrupt-driven sample acquisition if later needed
