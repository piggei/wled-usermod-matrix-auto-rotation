# Roadmap / TODO

## 0.1.0 status

The available-hardware RC regression matrix passed and `0.1.0` was promoted to the first stable release without runtime code changes after RC1.

## Device qualification status

- Confirm the MPU-family backend on a known genuine MPU-6050 (`WHO_AM_I 0x68/0x69`).
- QMI8658 on Waveshare ESP32-S3 RGB Matrix (SDA 47 / SCL 48): **hardware-qualified and promoted to 0.1.1-rc.1**.

## 0.2.0 candidates

- **Sensor Auto Detect** after the I²C bus/board/pins have already been configured. Keep bus selection and sensor detection as separate concepts.
- Optional explicit sensor address selection in Shared mode for installations containing multiple compatible devices.
- Add new accelerometer backends without changing the common orientation engine.
- Consider a dedicated WLED PinManager owner/claim path for Custom I²C if an upstream-compatible usermod pin-owner mechanism is available.

## Design constraints to preserve

- Keep sensor drivers separate from the common orientation engine.
- Rotate only the final logical 2D raster.
- `Setup Rotation` remains a visual raster offset; sensor calibration must not repurpose it.
- Do not add continuous pitch/roll, gyro integration, interpolation or other IMU complexity unless a concrete use case requires it.
- Preserve Shared I²C as non-owning: it must not reinitialize, repin or retime WLED's bus.
