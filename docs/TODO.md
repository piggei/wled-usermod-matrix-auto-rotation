# Roadmap / TODO

## Stable release status

`0.1.1` is the current stable release. It carries forward the qualified 0.1.0 feature set and adds hardware-qualified QMI8658 support plus the Waveshare ESP32-S3 RGB Matrix I²C preset.

## Device qualification status

- Confirm the MPU-family backend on a known genuine MPU-6050 (`WHO_AM_I 0x68/0x69`).
- QMI8658 on Waveshare ESP32-S3 RGB Matrix (SDA 47 / SCL 48): **hardware-qualified and released in 0.1.1**.
- v0.1.2 I²C lifecycle hardening: **hardware-qualified in dev-b005** for Waveshare board-shared 47/48 transitions and ESP32-C3 MAR-owned Custom 8/9 -> Shared -> Custom round-trip; release promotion still pending.

## 0.2.0 candidates

- **Sensor Auto Detect** after the I²C bus/board/pins have already been configured. Keep bus selection and sensor detection as separate concepts.
- Optional explicit sensor address selection in Shared mode for installations containing multiple compatible devices.
- Add new accelerometer backends without changing the common orientation engine.
- Consider a dedicated WLED PinManager owner ID if upstream provides a registration path suitable for out-of-tree usermods; current builds use `PinOwner::UM_Unspecified` consistently.

## Design constraints to preserve

- Keep sensor drivers separate from the common orientation engine.
- Rotate only the final logical 2D raster.
- `Setup Rotation` remains a visual raster offset; sensor calibration must not repurpose it.
- Do not add continuous pitch/roll, gyro integration, interpolation or other IMU complexity unless a concrete use case requires it.
- Preserve Shared I²C as non-owning: it must not reinitialize, repin or retime WLED's bus.

## Runtime polish

- [ ] Replace sensor-initialization `delay()` calls with timestamp-driven non-blocking stages (external review lower-priority finding).
