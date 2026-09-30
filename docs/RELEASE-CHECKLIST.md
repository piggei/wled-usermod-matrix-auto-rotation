# 0.1.0 release checklist

## Qualified RC1 baseline

- [x] LIS3DH detection/XYZ hardware pass
- [x] Matrix Portal S3 0/90/180/270 hardware pass
- [x] Setup Rotation composition hardware pass
- [x] ICM-20689 detection/XYZ hardware pass
- [x] ESP32-C3 automatic rotation hardware pass
- [x] ESP32-C3 Custom I²C hardware pass
- [x] ESP32-C3 Shared I²C with PAJ7620 coexistence hardware pass
- [x] Invert Rotation hardware pass
- [x] Auto Calibrate hardware pass on current opposite-face installation
- [x] Runtime disconnect/reconnect recovery hardware pass
- [x] Current desktop configuration UI hardware/UI pass
- [x] README demo GIF and configuration screenshot present
- [x] PlatformIO out-of-tree integration documented
- [x] Version consistency/static package audit for RC1

## 0.1.0 finalization

- [x] Re-run the available-hardware RC regression matrix
- [x] Confirm no RC1 regression reports require code changes
- [x] Change version metadata from `0.1.0-rc.1` to `0.1.0`
- [x] Change README status from release candidate to stable release
- [x] Add final changelog entry
- [x] Verify archive contains no temporary/generated/unwanted files
- [ ] Create Git tag `v0.1.0` and publish matching GitHub release notes

## Non-blocking qualification gaps

- Genuine MPU-6050 hardware pass
- Dedicated rectangular-matrix hardware pass
- Additional accelerometer boards planned for later support
