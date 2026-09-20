# v0.1.0-dev b004 test plan

b004 keeps the b001/b002 rotation engine unchanged. Matrix Portal + onboard LIS3DH auto-rotation and Setup Rotation are already hardware-confirmed. This build primarily needs UI regression testing.

## A. UI layout

### A1 - Standard labels

Confirm these visible field labels end with `:`:

- Enabled:
- Setup Rotation:
- Auto Rotation:
- Sensor:
- Sensor Mounting:
- Mode:
- SDA Pin: / SCL Pin: / Address: when Custom is selected
- Advanced:
- Threshold G: / Hysteresis G: / Stable Ms: / Poll Ms: when Advanced is enabled

The orientation values inside `Allow Rotation` remain plain `0 / 90 / 180 / 270` and are not field titles.

### A2 - Separators and spacing

Expected:

- no thin separator between `MatrixAutoRotation` and `Enabled:`;
- visible spacing after `Setup Rotation:`;
- no thin separator immediately below the I²C controls;
- subsection titles rely on spacing/typography rather than extra rules.

### A3 - I²C section

Expected section title: `I²C` with the `2` raised.

Mode choices must be exactly:

- Matrix Portal
- Custom

With `Matrix Portal`, SDA Pin, SCL Pin and Address are hidden.
With `Custom`, all three appear immediately and can be edited.

### A4 - Allow Rotation

Expected:

- `Allow Rotation` is larger and bold;
- checkboxes `0 / 90 / 180 / 270` are on one row at normal desktop width;
- the row may wrap on a narrow/mobile viewport;
- saving/restoring each checkbox works.

### A5 - Advanced

With `Advanced:` unchecked, the four tuning controls are hidden.
With it checked, Threshold G, Hysteresis G, Stable Ms and Poll Ms appear.
Saving and reloading must preserve both the checkbox and values.

## B. Configuration migration

### B1 - b001

Upgrade a configuration with the old flat I²C and allow keys. Values must be retained and rewritten in the current grouped structure after save.

### B2 - b002 Matrix Portal / Custom

Both modes and their values must survive unchanged.

### B3 - b002 removed presets

Legacy mode `ESP32 Generic / DevKit (21/22)` must migrate to Custom SDA 21 / SCL 22.
Legacy mode `ESP32-S3 DevKitC-1 (8/9)` must migrate to Custom SDA 8 / SCL 9.

## C. Matrix Portal regression

Use LIS3DH, Matrix Portal, address Auto, all rotations allowed, and the qualified defaults:

- Threshold: 0.55 g
- Hysteresis: 0.12 g
- Stable time: 600 ms
- Poll interval: 100 ms

Confirm physical orientation mapping remains:

| Physical panel | Expected automatic rotation |
| --- | ---: |
| +Y upright | 0° |
| +X | 90° |
| -Y | 180° |
| -X | 270° |

Also recheck Setup Rotation at least at 0° and 90° to ensure the UI-only changes did not affect composition.

## D. GY-521 / MPU-6050

Deferred until the test wiring is soldered. Use Custom I²C and start with address Auto.
