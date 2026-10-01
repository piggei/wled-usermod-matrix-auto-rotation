# WLED Matrix Auto Rotation v0.1.1

`0.1.1` adds hardware-qualified QMI8658 support and a dedicated Waveshare ESP32-S3 RGB Matrix I²C preset while preserving the rotation engine and behavior introduced in 0.1.0.

## New in 0.1.1

- Added direct-register **QMI8658** accelerometer support with no new third-party runtime dependency.
- Added the **Waveshare ESP32-S3 RGB Matrix** I²C preset using onboard SDA GPIO47 and SCL GPIO48.
- Added QMI8658 probing at `0x6A` / `0x6B` with identity validation through `WHO_AM_I = 0x05`.
- Configured QMI8658 acceleration for ±4 g, 125 Hz and LPF mode 0; the gyroscope remains disabled.
- Added QMI8658 addresses to Custom-I²C address selection.
- Extended diagnostics for the Waveshare preset and QMI8658 device identity.

## Hardware validation

The Waveshare ESP32-S3 RGB Matrix + onboard QMI8658 path has been verified on real hardware, including:

- sensor detection and live XYZ acceleration;
- automatic 0° / 90° / 180° / 270° rotation;
- Sensor Mounting and Invert Rotation behavior;
- Auto Calibrate;
- operation with a 64×64 HUB75 matrix.

Final release qualification snapshot:

- WLED `17.0.0-devV5`;
- Waveshare ESP32-S3 RGB Matrix target;
- HUB75 matrix `64x64`;
- QMI8658 detected at `0x6B`;
- `WHO_AM_I = 0x05`;
- SDA `47`, SCL `48`;
- 1146 successful reads;
- 0 read errors;
- 0 spurious disconnects/reconnects.

The existing 0.1.0-qualified LIS3DH and ICM-20689 paths remain unchanged and form the regression baseline.

## Release policy

The runtime code in `0.1.1` is unchanged from `0.1.1-rc.1` apart from version metadata. The stable promotion contains no new feature or algorithm change.

## Known non-blocking validation gaps

- Dedicated testing on a confirmed genuine MPU-6050 remains pending.
- The rectangular-matrix 90° / 270° guard is statically validated but has not yet received a dedicated real-hardware test.
