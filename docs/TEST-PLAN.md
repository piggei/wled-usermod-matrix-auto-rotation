# v0.1.0-dev-b010 validation plan

This plan covers current behavior only. Historical development-build migrations are regression-tested in code/config handling but are intentionally omitted from the user-facing test procedure.

## 1. Configuration UI

Verify:

- normal field labels end with `:`;
- `I²C` is shown as a section heading;
- modes are `Matrix Portal`, `Shared`, `Custom`;
- SDA/SCL/Address appear only for `Custom`;
- `Advanced:` reveals Threshold G, Hysteresis G, Stable Ms and Poll Ms;
- `Allow Rotation` displays 0 / 90 / 180 / 270 and prevents an empty allow mask.

## 2. Matrix Portal S3 + LIS3DH regression

Configuration:

- Sensor: `LIS3DH`
- I²C Mode: `Matrix Portal`
- Sensor Mounting: `0°`
- Setup Rotation: `0°`
- all rotations allowed

Expected:

| Physical orientation | Axis | Auto |
| --- | --- | ---: |
| 0° | +Y | 0° |
| 90° CW | +X | 90° |
| 180° | -Y | 180° |
| 270° CW | -X | 270° |

Also verify Setup Rotation at 90° composes with automatic rotation rather than replacing it.

## 3. Detection stability

Using qualified defaults (0.55 g / 0.12 g / 600 ms / 100 ms):

1. Hold near a 45° diagonal: no X/Y chatter.
2. Cross into another orientation for less than 600 ms: no accepted rotation.
3. Hold a new orientation for more than 600 ms: one clean rotation.
4. Lay the panel nearly flat: preserve the last stable orientation.
5. Return upright: detection resumes normally.
6. Disable one or more orientations and verify disallowed candidates are ignored.

## 4. ICM-20689 on ESP32-C3

Known-qualified device:

- address `0x68`;
- `WHO_AM_I = 0x98`.

Expected WLED Info:

- `MAR status: sensor ready`;
- sensor identified as `ICM-20689`;
- non-zero live X/Y/Z values;
- automatic rotation follows the same four-orientation behavior as LIS3DH after any required `Sensor Mounting` offset.

## 5. Shared I²C coexistence on ESP32-C3

Connect both sensors to the same physical SDA/SCL pair:

- PAJ7620: `0x73`;
- ICM-20689: `0x68`.

Configure the bus owner normally and set Matrix Auto Rotation to `Shared`.

Expected:

- `MAR I²C mode: Shared (WLED Wire)`;
- PAJ7620 remains connected and gestures remain functional;
- ICM-20689 remains detected and automatic rotation works;
- MAR does not change bus pins or clock;
- a late-initialized shared bus is recovered by MAR's periodic sensor-init retry.

## 6. Custom I²C

On a board/pin pair not already owned by WLED:

- configure valid SDA/SCL;
- leave Address on Auto first;
- verify sensor detection and XYZ;
- verify explicit supported address selection;
- verify invalid/equal/conflicting pins are rejected without destabilizing WLED.

On single-controller ESP32 targets, document that Custom rebinds the sole controller; use Shared instead when another component already owns the bus.

## 7. Rectangular matrix guard

For a non-square matrix:

- 0° and 180° must rotate normally;
- effective 90°/270° must leave the raster unchanged;
- WLED Info must report the unsupported rotation condition;
- no crop, resize or geometry mutation is allowed.

## 8. Diagnostics and error paths

Verify meaningful status for:

- no I²C response;
- WHO_AM_I read failure/mismatch;
- configuration write/read-back failure;
- framebuffer allocation failure;
- unsupported rectangular rotation.

A failure before `sensor.begin()` must never be displayed as sensor `OK`.

## 9. Pending hardware qualification

A confirmed MPU-6050 device still requires a dedicated hardware pass. Expected IDs are `0x68`/`0x69`; repeat sections 3, 4 and 6 when available.
