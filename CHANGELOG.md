# Changelog

## 0.1.2 - 2026-10-28

Stable release promoted from `0.1.2-rc.1` after release-candidate smoke tests passed on both tested hardware paths. Runtime behavior is unchanged from RC1 except for the version string.

- Finalize the I2C lifecycle hardening introduced during the 0.1.2 cycle.
- Preserve atomic PinManager reservation/rollback for MAR-owned Custom buses.
- Preserve safe single-controller teardown: MAR ends an owned global `Wire` before releasing its pins, and Shared mode does not probe an ended MAR-owned bus.
- Preserve the Waveshare SDA47/SCL48 board-shared contract so MAR never claims or reinitializes the onboard QMI8658/SHTC3 bus.
- Record external review confirmation that both previous Important findings are addressed in source and that no new Important finding was found within the inspected scope.
- Record RC1 smoke-test PASS on ESP32-C3 / ICM-20689 and Waveshare ESP32-S3 RGB Matrix / QMI8658.
- Keep remaining lifecycle coverage items and the short sensor-initialization `delay()` cleanup as documented non-blocking follow-up work.

## 0.1.2-rc.1 - 2026-10-28

- Promote the hardware-tested `0.1.2-dev-b006` baseline to release candidate without functional runtime changes other than the version string.
- Preserve the tested I2C lifecycle changes for MAR-owned Custom buses, Shared-mode non-ownership, and the Waveshare board-shared SDA47/SCL48 contract.
- Record external review confirmation that both previous Important findings are addressed in source and that no new Important finding was found within the inspected scope.
- Keep remaining coverage limits explicit: pin-conflict rejection, failed-start rollback, arbitrary Custom pin reassignment, MatrixPortal ownership/reuse, Auto Rotation-only disable, delayed external board-bus owner startup, genuine MPU-6050 hardware testing, and real rectangular-matrix guard testing.
- Keep the short sensor-initialization `delay()` cleanup as lower-priority future work.

## 0.1.2-dev-b006

- Documentation and test-record consolidation of the b005 runtime tested on hardware; no functional runtime changes other than the version string.
- Record Waveshare ESP32-S3 RGB Matrix lifecycle PASS for `Custom 47/48 -> Waveshare preset -> Shared -> Waveshare preset`, with QMI8658 and SHTC3 remaining healthy on the board-shared bus.
- Record ESP32-C3 single-controller lifecycle PASS for ICM-20689 on Custom SDA8/SCL9 through `Custom -> Shared unavailable -> Custom`, with continuous uptime and no new panic/reboot.
- Mark only actually executed lifecycle cases as PASS; conflict rejection, failed-bus-start rollback, arbitrary pin reassignment and MatrixPortal ownership regressions remain pending.
- Keep the external-review lower-priority sensor-initialization `delay()` cleanup as a future improvement.

## 0.1.2-dev-b005

- Fix ESP32-C3 single-controller transition from MAR-owned Custom I2C to Shared mode.
- Shared mode now validates that a global/external `Wire` owner actually exists before probing it.
- A `Wire` instance owned by MAR itself is never mistaken for an available Shared bus.
- MAR tears down its owned bus and releases PinManager pins, then waits safely if no WLED/global Shared bus is available.
- Shared startup/recovery retries remain non-owning and can attach later if WLED or another board/user module initializes global `Wire`.
- Hardware isolation test: Custom 8/9 -> Disabled confirmed that `Wire.end()` + PinManager release alone does not panic on ESP32-C3; the prior panic was caused by probing an ended `Wire` immediately after switching to Shared.

## 0.1.2-dev-b004

- Fix the remaining Waveshare initialization-order race exposed by b003 hardware testing.
- Treat the Waveshare onboard SDA47/SCL48 I2C bus as board-shared by contract instead of trying to infer ownership from `Wire.getClock()` or WLED pin bookkeeping.
- The Waveshare preset now always returns global `Wire` and never reserves 47/48, calls `Wire.begin()`, or starts a second controller.
- Custom SDA47/SCL48 follows the same board-shared rule while preserving the user-selected sensor address.
- Extend the fast startup retry window to every path currently using shared global `Wire`, including the Waveshare preset and Custom 47/48, so MAR can initialize after SHTC3/board code brings up the bus.
- Preserve the PinManager reservation and controller-before-pin-release fixes for genuinely MAR-owned Custom/MatrixPortal buses.
- Hardware regression motivating this build: b003 still reported `MAR owned` on 47/48 when MAR setup ran before the SHTC3 usermod, causing QMI8658 disconnects and null SHTC3 readings.

## 0.1.2-dev-b003

- Fix regression found during real Waveshare testing of b002: an already-active global `Wire` bus may exist even when WLED `i2c_sda/i2c_scl` bookkeeping is unset.
- Detect an already initialized global Arduino `Wire` controller through `Wire.getClock()` before attempting a MAR-owned bus.
- MatrixPortal (16/17) and Waveshare (47/48) presets reuse an active untracked global `Wire` bus rather than starting a second I2C controller on the same physical wires.
- Custom mode on those same known board-bus pairs also reuses active global `Wire`; arbitrary Custom pairs are rejected when global `Wire` is active but its pins cannot be identified safely.
- Preserve the b002 lifecycle fixes: MAR-owned single-controller buses are ended before pin release, Shared remains non-owning, and owned preset/Custom buses still reserve GPIO through PinManager.
- Hardware regression that motivated this build: b002 Custom 47/48 on Waveshare caused both QMI8658 and SHTC3 to fail after a few transactions because two I2C controllers were attached to the same wires.

- Close the external-review single-controller lifecycle finding: when MAR owns global `Wire`, call `Wire.end()` before releasing its SDA/SCL reservation.
- Harden single-controller Custom mode: reuse WLED global `Wire` when Custom pins exactly match WLED I2C, reject attempts to steal an active WLED bus on different pins, and retain explicit Custom sensor-address selection.
- Harden MatrixPortal and Waveshare board presets: reuse WLED global I2C when it is already configured on the preset pins; otherwise reserve the fixed preset pair through `PinManager` before starting an MAR-owned bus.
- Preserve Shared mode as strictly non-owning: no `begin()`, `end()`, pin reservation, repinning or clock changes.
- Expand the regression plan for pin-conflict rejection, bus-start rollback, disable/reconfiguration transitions, Custom-to-Shared transitions and preset ownership transitions.
- Keep the lower-priority sensor-initialization `delay()` cleanup as a separate future improvement.

## 0.1.2-dev-b001

- Reserve Custom I2C SDA/SCL pins atomically through WLED `PinManager` before starting the bus.
- Track the actually reserved Custom pin pair separately from the current configuration.
- Release the reserved pair when Custom-bus ownership ends and roll back the reservation if I2C initialization fails.
- Shared I2C ownership semantics are unchanged: Shared mode does not allocate, release, rebind, or configure WLED's global bus.
- Addresses the Important finding from the external Community Usermods review.

## 0.1.1 - 2026-10-01

Stable release promoted from `0.1.1-rc.1` after the Waveshare ESP32-S3 RGB Matrix + QMI8658 release-candidate smoke test passed without runtime changes.

- Finalized package/version metadata as `0.1.1`.
- Confirmed the Waveshare ESP32-S3 RGB Matrix preset on SDA47/SCL48 with the onboard QMI8658 detected at `0x6B` / `WHO_AM_I 0x05`.
- Confirmed automatic rotation and Auto Calibrate on a 64×64 HUB75 matrix.
- Recorded a testing snapshot of 1146 successful QMI8658 reads with 0 errors and no spurious disconnect/reconnect events.
- Completed documentation/version consistency audit and promoted RC wording to stable-release wording.
- Kept confirmed genuine MPU-6050 hardware testing and dedicated rectangular-matrix hardware testing as non-blocking follow-up work.
- Runtime rotation, sensor, calibration, raster, recovery and I²C algorithms are unchanged from `0.1.1-rc.1`.

## 0.1.1-rc.1 - 2026-10-01

Release-candidate promotion of the hardware-tested QMI8658 / Waveshare development path.

- Promoted package/version metadata from `0.1.1-dev-b002` to `0.1.1-rc.1`.
- Recorded Waveshare ESP32-S3 RGB Matrix + QMI8658 detection, XYZ sampling, automatic rotation and Auto Calibrate as hardware-tested RC coverage.
- Reworked the release checklist and validation wording for the 0.1.1 RC/final gate.
- Added dedicated RC1 release notes.
- Retained genuine MPU-6050 hardware testing and a dedicated rectangular-matrix hardware test as non-blocking follow-up work.
- No rotation, sensor, calibration, raster, recovery or I²C runtime algorithm changed relative to `0.1.1-dev-b002`.

## 0.1.1-dev-b002 - 2026-10-01

QMI8658 / Waveshare consolidation build after successful real-hardware testing.

- Marked Waveshare ESP32-S3 RGB Matrix + QMI8658 detection, XYZ sampling, automatic rotation and Auto Calibrate as hardware PASS.
- Updated the compatibility matrix, test plan and roadmap to reflect the completed hardware testing.
- Kept the QMI8658 direct-register backend, Waveshare SDA47/SCL48 preset and all common rotation logic unchanged from b001.
- Retained genuine MPU-6050 hardware testing as the only sensor-specific hardware-test gap.
- Performed a static package/documentation audit; no runtime algorithm changes were required.

## 0.1.1-dev-b001 - 2026-10-01

First post-0.1.0 hardware-expansion build.

- Added a direct-register QMI8658 accelerometer backend with no new third-party runtime dependency.
- Added QMI8658 address probing at `0x6A` / `0x6B` and identity validation using `WHO_AM_I = 0x05`.
- Configured the QMI8658 accelerometer for ±4 g, 125 Hz and LPF mode 0; the gyroscope remains disabled.
- Added `Waveshare ESP32-S3 RGB Matrix` as an I²C preset using onboard SDA GPIO47 and SCL GPIO48.
- Added QMI8658 addresses to the explicit Custom-I²C address list.
- Extended WLED Info diagnostics to identify the new preset and QMI8658 device.
- Updated README, roadmap and test plan for hardware testing on the newly available Waveshare board.
- Existing LIS3DH, MPU/ICM, rotation, calibration, recovery and raster algorithms remain unchanged.

## 0.1.0 - 2026-09-30

First stable release. Promoted from `0.1.0-rc.1` after the available LIS3DH and ICM-20689 regression matrix, including Shared I²C, Auto Calibrate and runtime disconnect/reconnect recovery, passed on hardware without requiring runtime code changes.

- Finalized package/version metadata as `0.1.0`.
- Marked the README as the stable release.
- Recorded the completed RC regression and disconnect/reconnect testing.
- Kept MPU-6050-specific hardware testing and dedicated rectangular-matrix hardware testing as non-blocking follow-up work.
- Runtime rotation, sensor, calibration, raster, recovery and I²C algorithms are unchanged from `0.1.0-rc.1`.

## 0.1.0-rc.1 - 2026-09-30

Release-candidate consolidation derived from the hardware-tested b016 path.

- Promoted the package/version metadata from development build b016 to `0.1.0-rc.1`.
- Recorded runtime disconnect/reconnect recovery as hardware PASS.
- Reworked the test plan into an RC regression matrix and removed superseded development-build wording.
- Cleaned the pre-release TODO list around actual remaining RC/final-release gates.
- Clarified Custom-I²C comments without changing its runtime behavior.
- Retained the complete development changelog for traceability.
- No rotation, sensor, calibration, raster, recovery or I²C runtime algorithm was changed for RC1.

## 0.1.0-dev-b016

- Consolidated the README around the current UI and hardware-tested behavior and added the official auto-rotation demo GIF plus the latest configuration screenshot.
- Expanded WLED Info diagnostics with runtime state, I²C configuration, reconnect countdown, filter values and allowed orientations.
- Split final rotation diagnostics into `requested` and `applied` values.
- Hardened rectangular-matrix behavior: composed final 90°/270° requests are explicitly blocked and deterministically fall back to the unrotated raster, without crop, resize or WLED geometry mutation.
- `MAR matrix` now reports square/rectangular capability and whether a quarter-turn request is currently blocked.
- Updated regression documentation and TODO state.

## 0.1.0-dev-b015

- Reworked only the `Auto Calibrate` presentation; calibration and runtime behavior are unchanged from b014.
- Grouped `Sensor Mounting` and `Invert Rotation` in the left column of a dedicated calibration block.
- Moved the `Auto Calibrate` button to a right-hand column, vertically centered across those two sensor-installation controls.
- Kept the WLED-style orange calibration instructions below the full two-column block.
- Added a narrow-screen fallback that stacks the button below the two sensor-installation rows.
- Recorded the b014 Auto Calibrate hardware pass on the current ESP32-C3 + ICM-20689 installation; runtime disconnect/reconnect testing remains pending.

## 0.1.0-dev-b014

- Corrected the conceptual target of `Auto Calibrate`: it now calibrates `Sensor Mounting` and `Invert Rotation`, not `Setup Rotation`.
- Moved `Auto Calibrate` beside `Sensor Mounting` in the usermod UI.
- `Setup Rotation` is now explicitly untouched by calibration because it is an independent final-raster visual offset.
- The first stable upright pose becomes the calibrated `Sensor Mounting`; the following stable 90° clockwise turn determines `Invert Rotation`.
- Changed the calibration helper note to WLED-style orange.
- Kept the existing detected-sensor/stable-pose gating, unsaved-hardware-change protection, 15-second timeout, transactional failure behavior, and explicit Save requirement.

## 0.1.0-dev-b013

- Added guided `Auto Calibrate` beside `Setup Rotation`.
- The control is enabled only when the configured sensor is detected and the current pose is stable.
- Calibration captures the upright pose, asks for a 90° clockwise turn, and derives both `Setup Rotation` and `Invert Rotation`.
- Calibration reads live runtime sensor data from WLED Info, so it does not require a new persistent calibration state or a custom HTTP endpoint.
- Unsaved Sensor/I²C hardware changes disable calibration because the running firmware is still using the previous bus configuration.
- A 15-second timeout or sensor loss aborts calibration transactionally without changing settings.
- Successful calibration updates the UI only; the normal WLED Save action remains responsible for persistence.
- Sensor Auto Detect remains deferred to 0.2.0.

## 0.1.0-dev-b012

- Added runtime sensor disconnect detection after 3 consecutive failed reads.
- Preserves the last valid matrix rotation while the sensor is offline.
- Retries sensor initialization every 60 seconds after disconnect or absence.
- Shared I²C retains a short 1-second retry window during the first 10 seconds of boot to tolerate usermod initialization order.
- Added automatic recovery without reboot when the sensor returns.
- Added runtime health counters: successful reads, read errors, consecutive errors, disconnects, reconnects, and last-good sample age.
- Expanded orientation diagnostics to show raw orientation, sensor mounting, inversion, auto rotation, setup rotation, and effective rotation.
- Updated the GUI screenshot to the current interface including Invert Rotation.


## 0.1.0-dev b011

Rotation-direction inversion build.

- Added `Invert Rotation` directly below `Sensor Mounting` in the usermod UI.
- The option mirrors only the automatic quarter-turn component; `Setup Rotation` remains the base installation reference.
- 0° and 180° are unchanged; 90° and 270° are swapped.
- Added persistent `invert-rotation` configuration with a default of `false`, preserving existing configurations.
- Added inversion state to WLED Info orientation diagnostics.
- Added documentation and a dedicated hardware regression test for opposite-face sensor mounting.
- Sensor drivers, I²C modes, stabilization logic and final-raster rotation code are otherwise unchanged from b010.

## 0.1.0-dev b010

Consolidation build after hardware testing of Shared I²C on ESP32-C3.

- froze the b009 rotation and Shared-I²C behavior as the functional baseline;
- removed the unused Shared-bus sensor error enum/path left from the b008 experiment;
- cleaned stale build-number and experimental comments from the code;
- rewrote README around current behavior instead of development history;
- added a concise hardware test matrix;
- documented LIS3DH and ICM-20689 as hardware-tested and MPU-6050 as implemented/pending dedicated hardware testing;
- rewrote the test plan as a current regression test checklist;
- made PlatformIO examples use the stable repository directory name rather than a build-specific path.

## 0.1.0-dev-b009

- Fixed Shared I²C coexistence with other WLED usermods.
- Shared mode now uses WLED's already-initialized global `Wire` object and never reinitializes, ends, repins, or retimes the bus.
- Removed the experimental direct ESP32 HAL I²C path introduced in b008.
- Fixed a latent b008 read-path issue where Shared mode could not sample acceleration because no `TwoWire` pointer was present.
- Intended test case: ESP32-C3 with PAJ7620 at `0x73` and ICM-20689 at `0x68` on the same WLED I²C bus.

## 0.1.0-dev b008 - 2026-09-21

Shared-I²C coexistence build; rotation/classifier behavior is unchanged.

- Added `Shared` I²C mode between Matrix Portal and Custom.
- Shared mode does not reserve pins or call `Wire.begin()`, `Wire.end()`, change pins, or change bus frequency.
- On ESP32, Shared mode accesses the already initialized hardware I²C0 controller directly through the Arduino-ESP32 HAL, so it can coexist with another `TwoWire` owner of the same controller.
- Added one-second deferred retry when Shared mode starts before the bus-owning usermod.
- Added `shared I2C bus not ready` diagnostic.
- Added `MAR I²C mode` to WLED Info.
- Kept legacy I²C mode values 2/3 reserved for migration; Shared uses value 4 so existing configurations are not reinterpreted.
- Documented the ESP32-C3 PAJ7620 (`0x73`) + ICM-20689 (`0x68`) shared-bus testing procedure.

## 0.1.0-dev b007 - 2026-09-21

MPU-family compatibility build. Rotation/raster logic is unchanged.

- Added ICM-20689 detection via `WHO_AM_I = 0x98`.
- Kept MPU-6050 compatibility (`WHO_AM_I = 0x68/0x69`).
- Added ICM-20689 accelerometer DLPF setup/read-back through `ACCEL_CONFIG2` (`0x1D`).
- WLED Info now reports the detected model (`MPU-6050` or `ICM-20689`) instead of only the configured family.
- Renamed the sensor UI option to `MPU-6050 / ICM-20689`.
- Documented the ESP32-C3 testing case using Custom I²C on GPIO6/GPIO7.

## 0.1.0-dev b006 - 2026-09-21

ESP32-C3 Custom I²C compatibility build; rotation logic is unchanged.

- Fixed Custom I²C on single-controller ESP32 targets such as ESP32-C3.
- Custom mode now reuses the global `Wire` controller on single-I²C SoCs instead of attempting unavailable `TwoWire(1)`.
- ESP32 targets with more than one I²C controller keep the dedicated secondary-bus behavior.
- Fixed misleading `GY-521 (MPU-6050) | OK` diagnostics when bus initialization fails before sensor initialization.
- Added ESP32-C3 Custom I²C testing notes.

## 0.1.0-dev b005 - 2026-09-20

GY-521 / MPU-6050 testing build; final-raster rotation logic is unchanged.

- Added explicit MPU-6050 initialization diagnostics.
- Added `WHO_AM_I` tracking and display in WLED Info.
- Added MPU-6050 configuration read-back verification for power/clock, DLPF, sample divider and accelerometer full-scale range.
- Replaced the generic sensor-init failure with specific status messages.
- Added a GY-521 wiring and hardware testing procedure to the documentation.
- Kept LIS3DH behavior and the orientation classifier, previously tested on hardware, unchanged.

## 0.1.0-dev b004 - 2026-09-20

Documentation/UI presentation cleanup; rotation/sensor behavior is unchanged.

- Removed the obsolete generic-ESP32 migration discussion from the README.
- Added the real WLED configuration screenshot to the README in place of the ASCII UI mock-up.
- Kept the Advanced field descriptions and runtime behavior unchanged.

## 0.1.0-dev b003 - 2026-09-20

UI polish build; rotation/sensor behavior is unchanged from the hardware-working baseline.

- Added `:` to all ordinary field labels to match WLED UI conventions.
- Removed the separator immediately below the MatrixAutoRotation title.
- Added visual spacing after `Setup Rotation:`.
- Reworked bus controls into one `I²C` section containing `Mode:` and the conditional Custom fields.
- Reduced the I²C mode selector to `Matrix Portal` and `Custom`.
- Removed the broad `ESP32 Generic / DevKit` and `ESP32-S3 DevKitC-1` choices.
- Added backward migration of those b002 modes to equivalent Custom pins (`21/22` and `8/9`).
- Removed the separator below the I²C area.
- Styled `Allow Rotation` as a larger bold subsection.
- Placed the `0 / 90 / 180 / 270` checkboxes on one responsive row.
- Kept `Advanced:` as a visibility toggle for threshold, hysteresis, stable time and poll interval.
- Standardized visible documentation/UI spelling to `I²C`.

## 0.1.0-dev b002 - 2026-09-20

UI refinement build based on the hardware-working b001 baseline.

- Confirmed b001 Matrix Portal + LIS3DH auto-rotation works on hardware.
- Confirmed setup/base rotation works on hardware.
- Renamed the I²C UI preset from `Matrix Portal / board default` to `Matrix Portal`.
- Added `ESP32 Generic / DevKit (21/22)` I²C preset, marked unverified.
- Added `ESP32-S3 DevKitC-1 (8/9)` I²C preset, marked unverified.
- Custom SDA/SCL/address controls now appear only when `Custom` I²C is selected.
- Removed the obsolete `Used only with Custom I²C` field descriptions.
- Added an `Advanced` checkbox; threshold, hysteresis, stable time and poll interval are hidden unless enabled.
- Grouped orientation mask under `Allow Rotation` with labels `0`, `90`, `180`, `270`.
- Added backward-compatible migration from b001 flat UI/config keys.
- Preset I²C modes use automatic sensor-address probing; explicit address selection is limited to Custom mode.

## 0.1.0-dev b001 - 2026-09-20

Initial development build.

- Added PlatformIO out-of-tree integration sample and dedicated integration notes.
- Added `*Zone.Identifier` to `.gitignore`.
- Added final-raster matrix rotation through WLED `handleOverlayDraw()`.
- Added setup/base rotation composition.
- Added automatic orientation detection derived from iDotMatrix Build 164.
- Added Matrix Portal S3 LIS3DH backend.
- Added GY-521 / MPU-6050 backend.
- Added sensor mounting compensation.
- Added Matrix Portal/default and custom ESP32 I²C modes.
- Added automatic I²C address probing.
- Added threshold, hysteresis, stable-time and poll-interval settings.
- Added allowed-orientation mask.
- Added WLED Info diagnostics.
- Added explicit square/rectangular rotation handling.
