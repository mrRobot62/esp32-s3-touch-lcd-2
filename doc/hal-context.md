# HAL Context

## Purpose

`hal::HalContext` is the project's low-level runtime entry point for board
services that should not live in the UI layer.

The current implementation focuses on four responsibilities:

- bringing up the shared I2C bus
- scanning and reporting I2C devices
- configuring and sampling the onboard QMI8658 IMU
- preparing optional Wi-Fi and UDP services

The class is intentionally small. It should remain the place where hardware
details are normalized before they are consumed by higher-level modules or UI
code.

## Source Files

- `include/hal/HalContext.hpp`
- `src/hal/HalContext.cpp`

## Initialization Sequence

`HalContext::begin()` runs the current startup sequence in a deterministic
order:

1. `setupI2c()`
2. `scanI2cDevices()`
3. `setupImu()`
4. `setupWifi()`
5. `setupUdp()`

This order matters because the IMU depends on the I2C bus, and UDP setup should
only continue after Wi-Fi status is known.

In the current firmware, the application initializes the display first and the
HAL second. That ordering is intentional because the touch controller and the
IMU share the same physical I2C lines on this board.

## Shared I2C Bus

The board uses one shared I2C bus for touch and IMU:

- `SDA = GPIO48`
- `SCL = GPIO47`
- bus speed = `400 kHz`

This is implemented in `setupI2c()` through the global `Wire` instance:

```cpp
Wire.begin(48, 47, 400000UL);
Wire.setTimeOut(20);
```

### Why this matters

- The touch controller and the IMU are not isolated on separate I2C buses.
- Any future I2C peripheral added to this board must be checked against the
  same shared bus usage.
- If the display/touch stack is changed later, the initialization order should
  be revalidated because both subsystems interact with the same lines.

## I2C Scan

`scanI2cDevices()` iterates through the 7-bit address range `0x08 .. 0x77` and
caches every address that acknowledges.

The scan result is exposed through:

- `scanI2cDevices()`
- `i2cDeviceAddresses()`

This is useful for:

- bring-up diagnostics
- validating hardware presence
- confirming whether touch and IMU respond after reset

## IMU Integration

The HAL owns the entire QMI8658 integration.

### Detection

`setupImu()` probes both common QMI8658 addresses:

- `0x6A`
- `0x6B`

It reads:

- `WHO_AM_I` register `0x00`
- optional revision register `0x01`

The expected `WHO_AM_I` value is currently `0x05`.

### Configuration

After detection, the HAL writes the control registers needed for demo use:

- `CTRL1 = 0x60`
- `CTRL2 = 0x23`
- `CTRL3 = 0x43`
- `CTRL7 = 0x03`

This enables accelerometer and gyroscope sampling with a demo-friendly profile.
If a future application needs different ranges or output data rates, this is
the first place to adjust.

### Runtime Sampling

`readImuSample(ImuSample& sample)`:

- waits for the QMI8658 data-ready state
- reads the 12-byte sensor block starting at register `0x35`
- converts raw values into engineering units
- applies low-pass filtering and deadband
- returns the filtered result

### Filtering

The HAL currently stabilizes the IMU data before it reaches the UI:

- accelerometer low-pass alpha: `0.18`
- gyroscope low-pass alpha: `0.12`
- accelerometer deadband around `0 g`: `0.018 g`
- gyroscope deadband around `0 dps`: `0.8 dps`
- accelerometer `Z` axis deadband around gravity: `1.0 g`

This filtering lives in the HAL by design so every consumer gets consistent
behavior without duplicating smoothing logic.

## Wi-Fi Setup

`setupWifi()` reads credentials from:

- `include/wifi_secrets.h`

That file is intentionally local and should not be committed with real
credentials. The project uses a placeholder example file for repository-safe
defaults.

If the placeholder credentials are still present, Wi-Fi setup is skipped
deliberately so a fresh checkout remains buildable and flashable.

## UDP Setup

`setupUdp()` reads endpoint configuration from:

- `include/udp_config.h`

The method stores:

- remote server IP
- remote server port
- local UDP bind port

UDP is only opened when Wi-Fi is connected. If Wi-Fi is unavailable, the target
configuration is still kept, but the socket is not activated.

## Public API Summary

Important methods:

- `begin()`
- `setupI2c()`
- `scanI2cDevices()`
- `setupImu()`
- `readImuSample(ImuSample&)`
- `setupWifi()`
- `setupUdp()`
- `isImuReady()`
- `imuAddress()`
- `isWifiConnected()`
- `udpServerIp()`
- `udpServerPort()`
- `udp()`

## Extension Guidelines

When adding new hardware services to `HalContext`, keep these rules:

- Put board-specific details here, not in the UI.
- Reuse the existing shared I2C bus unless the hardware truly requires another one.
- Keep startup order explicit.
- Keep logs meaningful enough for bring-up and field diagnostics.
- Prefer returning stable, processed values instead of leaking raw hardware noise
  into higher layers.

## Typical Future Extensions

The class is a good place for later additions such as:

- GPIO helpers
- battery measurement
- Bluetooth bring-up
- SD card helpers
- power-management hooks
- reusable sensor calibration state
