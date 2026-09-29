# Changelog

## 0.1.0-dev-b015

- Reworked only the `Auto Calibrate` presentation; calibration and runtime behavior are unchanged from b014.
- Grouped `Sensor Mounting` and `Invert Rotation` in the left column of a dedicated calibration block.
- Moved the `Auto Calibrate` button to a right-hand column, vertically centered across those two sensor-installation controls.
- Kept the WLED-style orange calibration instructions below the full two-column block.
- Added a narrow-screen fallback that stacks the button below the two sensor-installation rows.
- Recorded the b014 Auto Calibrate hardware pass on the current ESP32-C3 + ICM-20689 installation; runtime disconnect/reconnect qualification remains pending.

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

Consolidation build after hardware qualification of Shared I²C on ESP32-C3.

- froze the b009 rotation and Shared-I²C behavior as the functional baseline;
- removed the unused Shared-bus sensor error enum/path left from the b008 experiment;
- cleaned stale build-number and experimental comments from the code;
- rewrote README around current behavior instead of development history;
- added a concise hardware-validation matrix;
- documented LIS3DH and ICM-20689 as hardware-tested and MPU-6050 as implemented/pending dedicated hardware qualification;
- rewrote the test plan as a current regression/qualification checklist;
- made PlatformIO examples use the stable repository directory name rather than a build-specific path.

## 0.1.0-dev-b009

- Fixed Shared I²C coexistence with other WLED usermods.
- Shared mode now uses WLED's already-initialized global `Wire` object and never reinitializes, ends, repins, or retimes the bus.
- Removed the experimental direct ESP32 HAL I²C path introduced in b008.
- Fixed a latent b008 read-path issue where Shared mode could not sample acceleration because no `TwoWire` pointer was present.
- Intended validation case: ESP32-C3 with PAJ7620 at `0x73` and ICM-20689 at `0x68` on the same WLED I²C bus.

## 0.1.0-dev b008 - 2026-09-21

Shared-I²C coexistence build; rotation/classifier behavior is unchanged.

- Added `Shared` I²C mode between Matrix Portal and Custom.
- Shared mode does not reserve pins or call `Wire.begin()`, `Wire.end()`, change pins, or change bus frequency.
- On ESP32, Shared mode accesses the already initialized hardware I²C0 controller directly through the Arduino-ESP32 HAL, so it can coexist with another `TwoWire` owner of the same controller.
- Added one-second deferred retry when Shared mode starts before the bus-owning usermod.
- Added `shared I2C bus not ready` diagnostic.
- Added `MAR I²C mode` to WLED Info.
- Kept legacy I²C mode values 2/3 reserved for migration; Shared uses value 4 so existing configurations are not reinterpreted.
- Documented the ESP32-C3 PAJ7620 (`0x73`) + ICM-20689 (`0x68`) shared-bus qualification procedure.

## 0.1.0-dev b007 - 2026-09-21

MPU-family compatibility build. Rotation/raster logic is unchanged.

- Added ICM-20689 detection via `WHO_AM_I = 0x98`.
- Kept MPU-6050 compatibility (`WHO_AM_I = 0x68/0x69`).
- Added ICM-20689 accelerometer DLPF setup/read-back through `ACCEL_CONFIG2` (`0x1D`).
- WLED Info now reports the detected model (`MPU-6050` or `ICM-20689`) instead of only the configured family.
- Renamed the sensor UI option to `MPU-6050 / ICM-20689`.
- Documented the ESP32-C3 qualification case using Custom I²C on GPIO6/GPIO7.

## 0.1.0-dev b006 - 2026-09-21

ESP32-C3 Custom I²C compatibility build; rotation logic is unchanged.

- Fixed Custom I²C on single-controller ESP32 targets such as ESP32-C3.
- Custom mode now reuses the global `Wire` controller on single-I²C SoCs instead of attempting unavailable `TwoWire(1)`.
- ESP32 targets with more than one I²C controller keep the dedicated secondary-bus behavior.
- Fixed misleading `GY-521 (MPU-6050) | OK` diagnostics when bus initialization fails before sensor initialization.
- Added ESP32-C3 Custom I²C qualification notes.

## 0.1.0-dev b005 - 2026-09-20

GY-521 / MPU-6050 qualification build; final-raster rotation logic is unchanged.

- Added explicit MPU-6050 initialization diagnostics.
- Added `WHO_AM_I` tracking and display in WLED Info.
- Added MPU-6050 configuration read-back verification for power/clock, DLPF, sample divider and accelerometer full-scale range.
- Replaced the generic sensor-init failure with specific status messages.
- Added a GY-521 wiring and hardware qualification procedure to the documentation.
- Kept LIS3DH behavior and the hardware-qualified orientation classifier unchanged.

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
