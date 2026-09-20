# WLED Matrix Auto Rotation

**Version:** `0.1.0-dev`  
**Build:** `3` (`b003`)  
**Status:** Matrix Portal/LIS3DH rotation path hardware-validated; UI refinement build

A standalone WLED usermod that rotates the **final 2D matrix raster** according to an accelerometer, without changing effect or segment state.

## Build 3 focus

Build 3 keeps the hardware-validated rotation engine unchanged and refines only configuration/UI behavior:

- all ordinary field labels end with `:` to match the rest of WLED;
- the separator immediately below the module title is removed;
- extra spacing is added after `Setup Rotation:`;
- the bus controls are grouped under an `I²C` section;
- the bus selector contains only `Matrix Portal` and `Custom`;
- Custom SDA/SCL/address fields appear only when `Custom` is selected;
- `Allow Rotation` is a larger bold subsection and its four checkboxes are displayed on one row when space permits;
- `Advanced:` remains a checkbox and reveals the four tuning parameters only when enabled;
- b001 and b002 configuration layouts remain accepted. The short-lived b002 generic ESP32 presets are migrated to equivalent Custom pin values.

## Important architecture rule

The usermod does **not** rotate individual effects and does not alter segment render buffers.

WLED composites segments into its final logical pixel buffer and then calls `handleOverlayDraw()` immediately before the logical-to-physical LED mapping/output stage. The usermod rotates that final logical matrix area there.

```text
WLED effects / segments
        |
        v
final logical raster
        |
        v
Matrix Auto Rotation
        |
        v
WLED ledmap / physical mapping
        |
        v
LED output
```

Existing WLED ledmaps remain downstream of the usermod transform.

## Rotation composition

The configured setup rotation is the base/reference rotation:

```text
effective = setupRotation + autoRotation  (mod 360)
```

Sensor mounting is removed from the raw sensor-derived orientation before the automatic rotation is composed:

```text
autoRotation = rawSensorRotation - sensorMounting  (mod 360)
```

`setup-rotation` was hardware-tested in b001 and composes correctly with the automatic rotation.

## Matrix Portal S3 / LIS3DH

The following mapping was measured on the physical Matrix Portal S3 and inherited from the hardware-tested iDotMatrix Build 164:

| Physical panel orientation | Gravity direction | Auto rotation |
| --- | --- | --- |
| 0 deg | +Y | 0 deg |
| 90 deg clockwise | +X | 90 deg |
| 180 deg | -Y | 180 deg |
| 270 deg clockwise | -X | 270 deg |

Defaults:

- polling: **100 ms**
- stable time: **600 ms**
- minimum dominant X/Y axis: **0.55 g**
- diagonal hysteresis: **0.12 g**

If neither X nor Y reaches the threshold, the last stable orientation is preserved. Z is not used to invent an orientation while the panel is lying nearly flat.

## Sensor backends

### LIS3DH

- Auto-address order: `0x19`, then `0x18`.
- Matrix Portal S3 onboard LIS3DH is expected at `0x19`.
- +/-2 g, high-resolution mode.
- 50 Hz sensor data rate.

### GY-521 / MPU-6050

- Auto-address order: `0x68`, then `0x69`.
- +/-2 g accelerometer range.
- DLPF enabled.
- 50 Hz internal sample cadence.
- Backend implemented but still awaiting hardware qualification.

No third-party accelerometer library is required; both chips are accessed directly over I²C.

## I²C modes

| UI mode | SDA | SCL | Address | Qualification |
| --- | ---: | ---: | --- | --- |
| Matrix Portal | board variant (`16`) | board variant (`17`) | automatic probe | **hardware-tested** |
| Custom | user selected | user selected | Auto or explicit | depends on target |

The two generic ESP32 presets briefly present in b003 were removed because the names were too broad to be reliable board descriptions. Users of other boards select `Custom` and enter the documented pins for that board.

For backward compatibility, a saved b002 `ESP32 Generic / DevKit` selection migrates to Custom `21/22`, while `ESP32-S3 DevKitC-1` migrates to Custom `8/9`.

## Configuration UI

In **Config -> Usermods -> MatrixAutoRotation** the intended layout is:

```text
Enabled:                  [x]
Setup Rotation:           [0 / 90 / 180 / 270]

Auto Rotation:            [x]
Sensor:                   [LIS3DH / GY-521 (MPU-6050)]
Sensor Mounting:          [0 / 90 / 180 / 270]

I²C
  Mode:                   [Matrix Portal / Custom]
  SDA Pin:                [...]        (Custom only)
  SCL Pin:                [...]        (Custom only)
  Address:                [Auto / 0x18 / 0x19 / 0x68 / 0x69] (Custom only)

Allow Rotation
  [x] 0    [x] 90    [x] 180    [x] 270

Advanced:                 [ ]
  Threshold G:            0.55
  Hysteresis G:           0.12
  Stable Ms:              600
  Poll Ms:                100
```

The four Advanced values are hidden while `Advanced:` is unchecked. At least one allowed orientation is always enforced; if all four are disabled, `0` is restored automatically.

## Rectangular matrices

Because a 90/270 degree turn swaps width and height and this usermod deliberately does not reconfigure WLED geometry:

- square matrix: `0 / 90 / 180 / 270` supported;
- rectangular matrix: `0 / 180` supported;
- rectangular matrix + effective `90 / 270`: raster is left unchanged and WLED Info reports the unsupported rotation.

There is no crop, resize, or implicit geometry change.

## PlatformIO / external usermod integration

The repository is laid out as a PlatformIO-compatible **out-of-tree WLED usermod** (`library.json` + `REGISTER_USERMOD()`). It is loaded by WLED through `custom_usermods`.

```ini
[env:matrix_auto_rotation_test]
extends = env:YOUR_WORKING_WLED_ENV
custom_usermods =
  ${env:YOUR_WORKING_WLED_ENV.custom_usermods}
  symlink:///absolute/path/to/wled-usermod-matrix-auto-rotation-v0.1.0-dev-b003
```

Then:

```bash
pio run -e matrix_auto_rotation_test
pio run -e matrix_auto_rotation_test -t upload
```

See `platformio_override.ini.sample` and `docs/PLATFORMIO.md`.

## Recommended Matrix Portal test configuration

```text
enabled             = true
setup-rotation       = 0
auto-rotation        = true
sensor               = LIS3DH
sensor-mounting      = 0
i2c.mode             = Matrix Portal
advanced             = false
allow rotation       = 0 / 90 / 180 / 270 enabled
```

With Advanced hidden, the qualified defaults remain active.

## Known limitations / deliberate choices

- Matrix Portal S3 + LIS3DH and setup/base rotation are hardware-qualified from b001.
- GY-521 / MPU-6050 remains to be tested on hardware.
- Custom I²C uses ESP32 `Wire1`. Pins are checked for validity/conflicts but are not claimed with a borrowed/fake Usermod ID.
- Per-pixel WLED CCT metadata is not rotated. This does not affect the target Matrix Portal HUB75 RGB matrix or ordinary RGB/RGBW pixel matrices.
- No attempt is made to infer orientation from Z while the panel is flat.

## Origin of the orientation algorithm

The orientation classifier and Matrix Portal axis convention are derived from the hardware-tested `IDotMatrix-ESP32-Emulator 0.6.0-dev Build 164` supplied during development. The qualified threshold/hysteresis/stability behavior and output-to-source rotation convention are unchanged.

## License

MIT. See `LICENSE`.
