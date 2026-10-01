# v0.1.1 validation plan

`0.1.1` is the stable promotion of the hardware-qualified QMI8658 / Waveshare path. Existing 0.1.0 sensor and rotation paths remain the regression baseline, and the final promotion from RC1 introduces no runtime algorithm changes.

Hardware validated for the 0.1.0 release:

- LIS3DH on Matrix Portal S3;
- automatic 0° / 90° / 180° / 270° rotation;
- Setup Rotation composition;
- ICM-20689 on ESP32-C3;
- Custom I²C on ESP32-C3;
- Shared I²C coexistence with PAJ7620;
- Invert Rotation;
- Auto Calibrate on the current opposite-face sensor installation;
- runtime disconnect/reconnect recovery without reboot.

The checks below are the release-regression matrix.

## 1. Configuration UI

Verify:

- normal field labels end with `:`;
- `Sensor Mounting:` and `Invert Rotation:` occupy the left column of the calibration block;
- `Auto Calibrate` is vertically centered in the right column across those two rows;
- the upright/90° clockwise instruction appears in orange below the complete calibration block;
- the button is disabled until the configured sensor is detected and the current orientation is stable;
- changing Sensor or I²C hardware fields without saving disables the button;
- `I²C` is shown as a section heading;
- modes are `Matrix Portal`, `Waveshare ESP32-S3 RGB Matrix`, `Shared`, `Custom`;
- SDA/SCL/Address appear only for `Custom`;
- `Advanced:` reveals Threshold G, Hysteresis G, Stable Ms and Poll Ms;
- `Allow Rotation` displays 0 / 90 / 180 / 270 and prevents an empty allow mask;
- narrow/mobile layout stacks without horizontal overflow.

## 2. Auto Calibrate

With the sensor already detected and the display held upright:

1. Wait until `Auto Calibrate` becomes enabled.
2. Press it; no settings should change yet.
3. Rotate the display exactly 90° clockwise and hold it steady.
4. Verify the helper updates `Sensor Mounting` and `Invert Rotation` so the initial upright pose resolves to automatic rotation 0°; `Setup Rotation` must remain unchanged.
5. Verify the success message asks the user to press the normal WLED Save button.
6. Start again and do not rotate: after 15 seconds calibration must time out without changing settings.
7. Disconnect the sensor during calibration: the operation must abort without changing settings.
8. Change Sensor, I²C mode, SDA/SCL or address without saving: the button must remain disabled until the active hardware configuration matches the page again.

Calibration is intentionally separate from future Sensor Auto Detect: it operates only on an already configured and detected sensor.

## 3. Matrix Portal S3 + LIS3DH regression

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

## 4. Invert Rotation

With `Setup Rotation = 0°`, `Sensor Mounting = 0°` and all orientations allowed:

| Physical orientation | Normal auto | Invert Rotation auto |
| --- | ---: | ---: |
| 0° / +Y | 0° | 0° |
| 90° CW / +X | 90° | 270° |
| 180° / -Y | 180° | 180° |
| 270° CW / -X | 270° | 90° |

Verify that enabling/disabling `Invert Rotation` does not alter `Setup Rotation`; the effective rotation remains `setup + auto (mod 360)`. Verify persistence across Save + reboot.

## 5. Detection stability

Using qualified defaults (0.55 g / 0.12 g / 600 ms / 100 ms):

1. Hold near a 45° diagonal: no X/Y chatter.
2. Cross into another orientation for less than 600 ms: no accepted rotation.
3. Hold a new orientation for more than 600 ms: one clean rotation.
4. Lay the panel nearly flat: preserve the last stable orientation.
5. Return upright: detection resumes normally.
6. Disable one or more orientations and verify disallowed candidates are ignored.

## 6. ICM-20689 on ESP32-C3

Known-qualified device:

- address `0x68`;
- `WHO_AM_I = 0x98`.

Expected WLED Info:

- `MAR status: sensor ready`;
- sensor identified as `ICM-20689`;
- non-zero live X/Y/Z values;
- automatic rotation follows the same four-orientation behavior as LIS3DH after any required `Sensor Mounting` offset.

## 7. Shared I²C coexistence on ESP32-C3

Connect both devices to the same physical SDA/SCL pair:

- PAJ7620: `0x73`;
- ICM-20689: `0x68`.

Configure the bus owner normally and set Matrix Auto Rotation to `Shared`.

Expected:

- `MAR I²C mode: Shared (WLED Wire)`;
- PAJ7620 remains connected and gestures remain functional;
- ICM-20689 remains detected and automatic rotation works;
- MAR does not change bus pins or clock;
- a late-initialized shared bus is recovered by MAR's sensor-init retry.

## 8. Custom I²C

On a board/pin pair not already owned by WLED:

- configure valid SDA/SCL;
- leave Address on Auto first;
- verify sensor detection and XYZ;
- verify explicit supported address selection;
- verify invalid/equal/conflicting pins are rejected without destabilizing WLED.

On single-controller ESP32 targets, Custom rebinds the sole controller; use Shared instead when another component already owns the bus.

## 9. Runtime disconnect/reconnect

Hardware status: **PASS** (qualified in 0.1.0 and carried forward unchanged into 0.1.1).

Regression procedure:

1. Start with the sensor detected and confirm auto rotation works.
2. Disconnect SDA or sensor power while WLED is running.
3. Confirm MAR keeps the last applied rotation and reports `sensor disconnected` after three failed reads.
4. Confirm `MAR health` increments `errors` and `disconnects`.
5. Reconnect the sensor.
6. Wait up to 60 seconds without rebooting.
7. Confirm MAR returns to `sensor ready`, increments `reconnects`, and resumes orientation updates.
8. On Shared I²C, confirm the other device remains operational throughout.

## 10. Rectangular matrix guard

For a non-square matrix:

- final 0° and 180° rotate normally;
- a composed final request of 90°/270° applies 0° instead of attempting crop, resize or geometry mutation;
- `MAR orientation` shows different `requested` and `applied` values for a blocked quarter-turn;
- `MAR matrix` identifies the geometry as rectangular, reports `final 0/180 only`, and flags a blocked quarter-turn while requested;
- returning to a supported request clears the blocked state automatically;
- square matrices continue to report full 0/90/180/270 capability.

This guard is statically verified for 0.1.1; a dedicated rectangular hardware regression remains desirable when suitable hardware is available.

## 11. Diagnostics and error paths

Verify meaningful status for:

- no I²C response;
- WHO_AM_I read failure/mismatch;
- configuration write/read-back failure;
- framebuffer allocation failure;
- blocked rectangular quarter-turn.

Verify the 0.1.1 diagnostic set:

- `MAR runtime` reports enabled/auto/sensor-ready state;
- `MAR I²C config` reports Custom pins/address or the Shared/Matrix Portal ownership model;
- `MAR orientation` reports raw, mounting, invert, auto, setup, requested and applied rotations;
- `MAR health` reports read/error/disconnect/reconnect counters and last-good age;
- `MAR recovery` reports online/idle or offline retry countdown;
- `MAR filter` reports threshold, hysteresis, stable and poll values;
- `MAR allowed` reports enabled automatic orientations;
- `MAR matrix` reports geometry class and final rotation capability.

A failure before `sensor.begin()` must never be displayed as sensor `OK`.

## 12. Remaining device qualification

A confirmed MPU-6050 device still requires a dedicated hardware pass. Expected IDs are `0x68`/`0x69`. This is a device-qualification gap, not a blocker for the already-qualified LIS3DH and ICM-20689 targets.


## 13. Waveshare ESP32-S3 RGB Matrix + QMI8658

Hardware-qualified stable-release target for `0.1.1`.

Board preset:

- I²C Mode: `Waveshare ESP32-S3 RGB Matrix`;
- SDA: GPIO47;
- SCL: GPIO48;
- Sensor: `QMI8658`;
- address: auto-probe `0x6A` then `0x6B`;
- expected `WHO_AM_I`: `0x05`.

Expected WLED Info after boot:

- `MAR status: sensor ready`;
- `MAR I²C mode: Waveshare ESP32-S3 RGB Matrix`;
- `MAR I²C config: SDA 47 | SCL 48 | address auto`;
- sensor identified as `QMI8658 @ 0x6A/0x6B | ID 0x05`;
- live X/Y/Z acceleration values with one gravity-dominant axis near 1 g while upright.

Hardware result:

- QMI8658 detection and live XYZ values: **PASS**.
- Automatic 0°/90°/180°/270° rotation: **PASS**.
- `Auto Calibrate`: **PASS**.
- Waveshare SDA47/SCL48 preset: **PASS**.
- No changes were required to the common orientation engine or raster-rotation path.

Final Waveshare release qualification snapshot:

- WLED: `17.0.0-devV5`;
- matrix: `64x64` HUB75;
- detected device: `QMI8658 @ 0x6B | ID 0x05`;
- bus: SDA `47` / SCL `48`;
- health snapshot: `1146` successful reads, `0` errors, `0` disconnects, `0` reconnects;
- automatic rotation: **PASS**;
- Auto Calibrate: **PASS**.
