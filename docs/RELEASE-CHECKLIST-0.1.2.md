# v0.1.2 release qualification checklist

## Current baseline

`0.1.2-dev-b006` is a documentation/qualification consolidation of the hardware-verified `0.1.2-dev-b005` runtime. Runtime behavior is unchanged except for the version string. Stable release promotion is not yet declared.

## External-review findings

- [x] Custom I2C SDA/SCL reservation uses WLED PinManager atomically.
- [x] MAR-owned single-controller `Wire` is ended before its pins are released.
- [x] Shared mode does not probe an ended MAR-owned `Wire`; when no external owner exists it waits safely.
- [x] Waveshare SDA47/SCL48 is treated as a board-shared bus and is never claimed/reinitialized by MAR.
- [ ] Lower-priority sensor-initialization `delay()` cleanup (future polish; not a functional blocker).

## Hardware qualification completed

- [x] Waveshare ESP32-S3 RGB Matrix: Custom 47/48 -> Waveshare preset -> Shared -> Waveshare preset.
- [x] Waveshare QMI8658 remains detected at `0x6B / WHO_AM_I 0x05`.
- [x] Waveshare SHTC3 remains healthy while MAR shares the board bus.
- [x] ESP32-C3: ICM-20689 Custom SDA8/SCL9 reports `MAR owned` and sensor ready.
- [x] ESP32-C3: Custom 8/9 -> Disabled teardown completes without panic.
- [x] ESP32-C3: Custom 8/9 -> Shared unavailable preserves uptime and waits safely for WLED Wire.
- [x] ESP32-C3: Shared unavailable -> Custom 8/9 reacquires the bus and returns ICM-20689 to ready.
- [x] No new panic/reboot observed during the qualified b005 transition round-trip.

## Pending regression coverage before final promotion

- [ ] Pin-conflict rejection against an already allocated Custom pair.
- [ ] Failed bus-start rollback leaves no stale PinManager reservations.
- [ ] Arbitrary Custom pin reassignment releases the old pair and reserves the new pair.
- [ ] MatrixPortal ownership/reuse regression after the 0.1.2 lifecycle changes.
- [ ] Genuine MPU-6050 hardware qualification (pre-existing non-blocking gap).
- [ ] Real rectangular-matrix 90/270 guard regression (pre-existing non-blocking gap).

## Documentation / package

- [x] README qualification matrix updated.
- [x] TEST-PLAN marks only actually executed hardware cases as PASS.
- [x] TODO preserves remaining work and lower-priority delay cleanup.
- [x] 0.1.1 historical release checklist remains untouched.
- [x] b006 runtime diff against b005 is version-string only.
