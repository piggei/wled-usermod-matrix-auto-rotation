# v0.1.1 release checklist

## Final baseline

`0.1.1` is promoted directly from the hardware-qualified `0.1.1-rc.1`. Finalization changes version/release wording and documentation only; runtime rotation, sensor, calibration, recovery, raster and I²C behavior are unchanged.

## Package / code

- [x] Version metadata promoted to `0.1.1`
- [x] QMI8658 direct-register backend retained unchanged from qualified RC1
- [x] Waveshare ESP32-S3 RGB Matrix preset retained unchanged (SDA 47 / SCL 48)
- [x] No new third-party runtime dependency introduced
- [x] No TODO / FIXME / HACK markers in runtime sources
- [x] `.gitignore` keeps `*Zone.Identifier`
- [x] Package ZIP integrity verified

## Hardware qualification

- [x] Matrix Portal S3 + LIS3DH detection and XYZ
- [x] Matrix Portal S3 + LIS3DH automatic 0° / 90° / 180° / 270° rotation
- [x] ESP32-C3 + ICM-20689 detection and XYZ
- [x] ESP32-C3 + ICM-20689 automatic rotation
- [x] Custom I²C on ESP32-C3
- [x] Shared I²C coexistence with PAJ7620
- [x] Setup Rotation
- [x] Sensor Mounting
- [x] Invert Rotation
- [x] Auto Calibrate
- [x] Runtime sensor disconnect detection and automatic reconnect
- [x] Waveshare ESP32-S3 RGB Matrix + QMI8658 detection and XYZ
- [x] Waveshare ESP32-S3 RGB Matrix + QMI8658 automatic rotation
- [x] Waveshare ESP32-S3 RGB Matrix + QMI8658 Auto Calibrate
- [x] RC1 smoke test on Waveshare 64×64 HUB75 target
- [x] RC1 QMI8658 health snapshot: 1146 reads / 0 errors / 0 disconnects / 0 reconnects

## Documentation

- [x] README identifies `0.1.1` as stable
- [x] QMI8658 / Waveshare compatibility matrix marked hardware-tested
- [x] Current GUI screenshot present
- [x] README demo GIF present
- [x] PlatformIO integration documented
- [x] Validation plan updated for 0.1.1
- [x] Stable release notes added
- [x] Documentation/version consistency audit completed
- [x] Roadmap keeps Sensor Auto Detect in the 0.2.0 scope

## Non-blocking follow-up work

- [ ] Dedicated hardware qualification on a confirmed genuine MPU-6050
- [ ] Dedicated real-hardware regression of the 90° / 270° rectangular-matrix guard
- [ ] Sensor Auto Detect (0.2.0 candidate)
- [ ] Optional explicit address selection in Shared mode (0.2.0 candidate)
