# Changelog

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
