# v0.1.1 release checklist

## RC1 baseline

`0.1.1-rc.1` is promoted directly from the hardware-qualified `0.1.1-dev-b002` path. The RC promotion must not change runtime rotation, sensor, calibration, recovery, raster or I²C behavior.

## Package / code

- [x] Version metadata promoted to `0.1.1-rc.1`
- [x] QMI8658 direct-register backend retained unchanged from qualified b002
- [x] Waveshare ESP32-S3 RGB Matrix preset retained unchanged (SDA 47 / SCL 48)
- [x] No new third-party runtime dependency introduced
- [x] No TODO / FIXME / HACK markers in runtime sources
- [x] `.gitignore` keeps `*Zone.Identifier`
- [x] Package ZIP integrity verified

## Hardware qualification carried into RC1

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

## Documentation

- [x] README identifies `0.1.1-rc.1` as a release candidate
- [x] QMI8658 / Waveshare compatibility matrix marked hardware-tested
- [x] Current GUI screenshot present
- [x] README demo GIF present
- [x] PlatformIO integration documented
- [x] Validation plan updated for RC1
- [x] Roadmap keeps Sensor Auto Detect in the 0.2.0 scope

## Non-blocking follow-up work

- [ ] Dedicated hardware qualification on a confirmed genuine MPU-6050
- [ ] Dedicated real-hardware regression of the 90° / 270° rectangular-matrix guard
- [ ] Sensor Auto Detect (0.2.0 candidate)
- [ ] Optional explicit address selection in Shared mode (0.2.0 candidate)

## Final 0.1.1 gate

Before promoting RC1 to `0.1.1`:

- [ ] Flash and smoke-test RC1 on the currently available qualified hardware
- [ ] Confirm QMI8658 / Waveshare path remains operational
- [ ] Confirm at least one pre-0.1.1 sensor path remains operational
- [ ] Confirm no documentation/version mismatch remains
- [ ] Promote only version/release wording; do not add new features during finalization
