# WLED Matrix Auto Rotation v0.1.2

`v0.1.2` is the stable promotion of the hardware-tested `v0.1.2-rc.1` release candidate. Runtime behavior is unchanged from RC1 except for the version string.

## Highlights

This release hardens I2C ownership and lifecycle handling after the Community Usermods external review and subsequent real-hardware regression testing.

- Custom SDA/SCL reservations are atomic through WLED `PinManager`.
- MAR-owned buses release the controller before releasing their GPIO reservations.
- Shared mode remains strictly non-owning and safely waits when no external WLED `Wire` owner is available.
- Waveshare ESP32-S3 RGB Matrix SDA47/SCL48 is treated as a board-shared bus, preventing a second I2C controller from being attached to the onboard QMI8658/SHTC3 wires.
- Board-preset/shared-bus coexistence and single-controller transitions were hardened without changing rotation, calibration, recovery or raster algorithms.

## External review

The follow-up CodeRabbit external-source inspection of revision `1757390` confirmed that both previous Important findings were addressed in source and reported no new Important finding within its inspection scope.

## Hardware testing

### Waveshare ESP32-S3 RGB Matrix

Tested with the onboard QMI8658 (`0x6B`, `WHO_AM_I 0x05`) and SHTC3 on the board-shared SDA47/SCL48 bus. The following transition sequence passed without bus loss:

`Custom 47/48 -> Waveshare preset -> Shared -> Waveshare preset`

QMI8658 remained ready and SHTC3 continued returning valid temperature/humidity data.

### ESP32-C3

Tested with ICM-20689 (`0x68`, `WHO_AM_I 0x98`) on Custom SDA8/SCL9. The critical single-controller round trip passed without panic or reboot:

`Custom 8/9 owned -> Shared unavailable -> Custom 8/9 owned`

MAR safely waited when no external Shared `Wire` owner existed, then reacquired the Custom bus and returned the sensor to ready state.

### RC1 smoke tests

The final RC1 smoke tests passed on both hardware paths above.

## Non-blocking follow-up

The project still tracks additional regression coverage for pin-conflict rejection, failed-start rollback, arbitrary Custom pin reassignment, MatrixPortal ownership/reuse, genuine MPU-6050 hardware testing and real rectangular-matrix guard testing. Short blocking delays during sensor initialization/recovery remain lower-priority cleanup work.
