# Roadmap / TODO

## Before 0.1.0 final

- Hardware-regression-test runtime disconnect/reconnect recovery on LIS3DH and ICM-20689.
- Complete the remaining `Auto Calibrate` regression on a normal-face sensor installation; the current inverted-face ESP32-C3 + ICM-20689 installation has passed and `Setup Rotation` remained untouched.
- Hardware-smoke-test the b015 two-column calibration layout on desktop and a narrow/mobile viewport.
- Verify Shared I²C recovery while PAJ7620 remains active on the same bus.
- Re-run the 0/90/180/270, Setup Rotation and Invert Rotation regression matrix.
- Static audit and documentation consistency pass before the first release candidate.

## 0.2.0 candidates

- **Sensor Auto Detect** after the I²C bus/board/pins have already been configured. Keep bus selection and sensor detection as separate concepts.
- Optional explicit sensor address selection in Shared mode for installations containing multiple compatible devices.
- Add new accelerometer backends as hardware becomes available, without changing the common orientation engine.

## Design constraints to preserve

- Do not use payload/effect state to determine orientation.
- Keep sensor drivers separate from the common orientation engine.
- Rotate only the final logical 2D raster.
- Do not add continuous pitch/roll, gyro integration, interpolation or other IMU complexity unless a concrete use case requires it.
- Preserve b011 as the last hardware-qualified baseline before runtime-recovery changes.
