# PlatformIO integration

This repository is an **out-of-tree WLED usermod**. It is not intended to be
compiled as a standalone firmware project: WLED supplies `wled.h`, the Usermod
API, LED matrix code and the target board environment.

The repository root contains `library.json`, so PlatformIO can load it directly
through WLED's `custom_usermods` mechanism.

## Fast local integration

Recommended directory layout:

```text
projects/
├── WLED/
└── wled-usermod-matrix-auto-rotation-v0.1.0-dev-b003/
```

In `WLED/platformio_override.ini`, add the usermod to the environment that you
already use for the target board:

```ini
[env:YOUR_WLED_ENV]
custom_usermods =
  ${env:YOUR_WLED_ENV.custom_usermods}
  symlink:///absolute/path/to/wled-usermod-matrix-auto-rotation-v0.1.0-dev-b003
```

If that environment already has a section in `platformio_override.ini`, **do not
create another section with the same name**. Add the `custom_usermods` entry to
the existing section.

Using `symlink://` is recommended during development: source edits in this
repository are picked up by the next PlatformIO build without copying files.

## Dedicated test environment

For early testing it is often safer to create a derived environment and leave
the known-good WLED environment unchanged:

```ini
[env:matrix_auto_rotation_test]
extends = env:BASE_WLED_ENV
custom_usermods =
  ${env:BASE_WLED_ENV.custom_usermods}
  symlink:///absolute/path/to/wled-usermod-matrix-auto-rotation-v0.1.0-dev-b003
```

Then use:

```bash
pio run -e matrix_auto_rotation_test
pio run -e matrix_auto_rotation_test -t upload
pio device monitor -e matrix_auto_rotation_test
```

`BASE_WLED_ENV` should be the WLED environment that already builds correctly for
your hardware. The usermod deliberately does not duplicate board, HUB75,
partition, PSRAM or USB settings.

## MatrixPortal S3

For MatrixPortal S3 testing, derive the test environment from the MatrixPortal
WLED environment that is already known to drive the panel correctly. The
usermod only adds accelerometer/orientation functionality; it must not replace
or silently change the existing HUB75 build flags.

Current WLED HUB75 code recognizes the MatrixPortal S3 through
`ARDUINO_ADAFRUIT_MATRIXPORTAL_ESP32S3` or `MATRIXPORTAL_S3_PINOUT`. Keep the
same MatrixPortal/HUB75 configuration used by your working base environment.

## Git-based integration

After the usermod repository has been pushed to GitHub, `custom_usermods` can
also point to a tag or commit:

```ini
custom_usermods =
  ${env:BASE_WLED_ENV.custom_usermods}
  https://github.com/YOUR_USER/wled-usermod-matrix-auto-rotation.git#TAG_OR_COMMIT
```

For development, prefer `symlink://`; for reproducible builds, prefer a pinned
tag or commit.

## Why there is no standalone `platformio.ini`

A standalone PlatformIO project would have to duplicate a substantial portion
of WLED's own build configuration and dependencies. That would be brittle and
could compile against settings different from the firmware actually flashed on
the device. The supported integration point is WLED's `custom_usermods`
mechanism plus this repository's `library.json`.

See also `platformio_override.ini.sample` in the repository root.
