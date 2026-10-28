# WLED Matrix Auto Rotation v0.1.2-rc.1

`v0.1.2-rc.1` is a release-candidate promotion of the hardware-tested `v0.1.2-dev-b006` baseline. Runtime behavior is unchanged except for the version string.

## What changed in 0.1.2

- Custom I2C SDA/SCL reservations are managed atomically through WLED `PinManager`.
- MAR-owned single-controller `Wire` buses are ended before their pins are released.
- Shared mode never treats MAR's own ended `Wire` instance as an externally owned shared bus.
- Waveshare SDA47/SCL48 is treated as a board-shared bus and is never reinitialized or claimed by MAR.
- Board/shared-bus transitions and Custom ownership transitions were hardened after real-hardware testing.

## Hardware-tested paths

- Waveshare ESP32-S3 RGB Matrix + QMI8658 + SHTC3: `Custom 47/48 -> Waveshare preset -> Shared -> Waveshare preset`, with both sensors remaining healthy.
- ESP32-C3 + ICM-20689: MAR-owned Custom SDA8/SCL9, `Custom -> Shared unavailable -> Custom`, with continuous uptime and no new panic/reboot.

## External review

A follow-up external inspection of revision `1757390` confirmed that both previous Important findings are addressed in source and reported no new Important finding within the inspected scope.

## Remaining non-blocking coverage gaps

- Pin-conflict rejection and failed-start rollback runtime tests.
- Arbitrary Custom pin reassignment runtime test.
- MatrixPortal ownership/reuse regression test.
- Auto Rotation-only disable lifecycle test.
- Delayed initialization of an external board-bus owner.
- Genuine MPU-6050 hardware testing.
- Real rectangular-matrix 90/270 guard regression.
- Lower-priority removal of short blocking sensor-initialization delays.
