# WLED Matrix Auto Rotation v0.1.1-rc.1

`0.1.1-rc.1` is the release-candidate promotion of the hardware-qualified 0.1.1 development path.

## New in 0.1.1

- Added direct-register **QMI8658** accelerometer support.
- Added the **Waveshare ESP32-S3 RGB Matrix** I²C preset using onboard SDA GPIO47 and SCL GPIO48.
- Added QMI8658 probing at `0x6A` / `0x6B` with identity validation through `WHO_AM_I = 0x05`.
- Configured QMI8658 acceleration for ±4 g, 125 Hz and LPF mode 0; the gyroscope remains disabled.
- Added QMI8658 addresses to Custom-I²C address selection.
- Extended diagnostics for the Waveshare preset and QMI8658 device identity.

## Hardware validation

The Waveshare ESP32-S3 RGB Matrix + onboard QMI8658 path has been verified on real hardware, including:

- sensor detection;
- live XYZ acceleration;
- automatic 0° / 90° / 180° / 270° rotation;
- Sensor Mounting / Invert Rotation behavior;
- Auto Calibrate.

Existing 0.1.0-qualified LIS3DH and ICM-20689 paths remain the regression baseline.

## RC policy

No runtime algorithm change is introduced when promoting `0.1.1-dev-b002` to `0.1.1-rc.1`. The RC is intended only for final regression and documentation/version validation before the stable `0.1.1` release.

## Known non-blocking validation gaps

- Dedicated testing on a confirmed genuine MPU-6050 remains pending.
- The rectangular-matrix 90° / 270° guard remains statically validated but has not yet received a dedicated real-hardware test.
