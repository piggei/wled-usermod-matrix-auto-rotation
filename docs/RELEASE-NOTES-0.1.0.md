# WLED Matrix Auto Rotation 0.1.0

First stable release of the WLED Matrix Auto Rotation usermod.

## Highlights

- Automatic 0° / 90° / 180° / 270° rotation of the final WLED 2D raster.
- Hardware-tested LIS3DH support on Matrix Portal S3.
- Hardware-tested ICM-20689 support on ESP32-C3; MPU-6050 backend implemented.
- Matrix Portal, Shared and Custom I²C modes.
- `Setup Rotation`, `Sensor Mounting` and `Invert Rotation` kept as separate concepts.
- Guided `Auto Calibrate` for sensor mounting and rotation direction.
- Threshold, hysteresis, stable-time and polling controls.
- Runtime disconnect detection and automatic reconnect every 60 seconds while preserving the last valid rotation.
- Extended WLED Info diagnostics for sensor, I²C, orientation pipeline, health and recovery state.
- Explicit rectangular-matrix protection: final 90°/270° requests are blocked without crop, resize or WLED geometry mutation.
- PlatformIO-ready out-of-tree integration through WLED `custom_usermods`.

## Hardware qualification

Validated on:

- Matrix Portal S3 + onboard LIS3DH;
- ESP32-C3 + ICM-20689;
- ESP32-C3 Shared I²C with PAJ7620 (`0x73`) and ICM-20689 (`0x68`) concurrently;
- runtime sensor disconnect/reconnect recovery;
- opposite-face sensor installation using `Invert Rotation` and `Auto Calibrate`.

## Known qualification gaps

- Dedicated hardware validation on a confirmed genuine MPU-6050 remains pending.
- Rectangular-matrix quarter-turn blocking is statically verified; a dedicated hardware regression remains desirable when suitable hardware is available.

See `README.md`, `docs/TEST-PLAN.md` and `CHANGELOG.md` for full details.
