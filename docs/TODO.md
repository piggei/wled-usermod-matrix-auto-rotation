# Roadmap / TODO

## Before 0.1.0 final

- Hardware-regression-test b012 runtime disconnect/reconnect recovery on LIS3DH and ICM-20689.
- Verify Shared I²C recovery while PAJ7620 remains active on the same bus.
- Re-run the 0/90/180/270, Setup Rotation and Invert Rotation regression matrix.
- Static audit and documentation consistency pass before the first release candidate.

## 0.2.0 candidates

- **Sensor Auto Detect** after the I²C bus/board/pins have already been configured. Keep bus selection and sensor detection as separate concepts.
- **Setup Rotation helper** available only when a sensor has already been detected and a valid/stable orientation exists. A static detect can determine the relative zero only.
- **Guided full calibration** if desired: first capture the zero position, then ask the user to rotate the matrix by 90 degrees in a known direction so the software can also determine rotation sense / Invert Rotation.
- Optional explicit sensor address selection in Shared mode for installations containing multiple compatible devices.
- Add new accelerometer backends as hardware becomes available, without changing the common orientation engine.

## Design constraints to preserve

- Do not use payload/effect state to determine orientation.
- Keep sensor drivers separate from the common orientation engine.
- Rotate only the final logical 2D raster.
- Do not add continuous pitch/roll, gyro integration, interpolation or other IMU complexity unless a concrete use case requires it.
- Preserve b011 as the last hardware-qualified baseline before runtime-recovery changes.
