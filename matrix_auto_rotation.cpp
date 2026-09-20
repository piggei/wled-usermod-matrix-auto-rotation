#include "wled.h"
#include <Wire.h>
#include <math.h>
#include "matrix_auto_rotation_sensor.h"

// WLED Matrix Auto Rotation
// v0.1.0-dev build 3
//
// Design rule: the usermod never modifies effect/segment state. Rotation is
// applied in handleOverlayDraw(), after WLED has composited the final logical
// raster and immediately before WLED applies its own logical->physical ledmap.

namespace {
constexpr char MAR_VERSION[] = "0.1.0-dev-b003";
constexpr uint8_t I2C_MODE_MATRIXPORTAL = 0;
constexpr uint8_t I2C_MODE_CUSTOM = 1;
// Legacy b002 values retained only for transparent configuration migration.
constexpr uint8_t I2C_MODE_LEGACY_ESP32_GENERIC = 2;
constexpr uint8_t I2C_MODE_LEGACY_ESP32S3_DEVKIT = 3;

constexpr int8_t ORIENT_UNKNOWN = -1;
constexpr int8_t DIR_UNKNOWN = -1;
constexpr int8_t DIR_POS_X = 0;
constexpr int8_t DIR_NEG_X = 1;
constexpr int8_t DIR_POS_Y = 2;
constexpr int8_t DIR_NEG_Y = 3;

#if defined(ARDUINO_ARCH_ESP32)
TwoWire marCustomWire(1);
#endif

float absf(float v) { return v < 0.0f ? -v : v; }
uint8_t quarterTurnsFromDegrees(uint16_t deg) { return ((deg / 90u) & 0x03u); }
uint16_t degreesFromQuarterTurns(uint8_t q) { return uint16_t(q & 0x03u) * 90u; }
uint8_t normalizeQuarterTurns(int value) { return uint8_t((value % 4 + 4) % 4); }
}

class MatrixAutoRotationUsermod : public Usermod {
private:
  static const char _name[];

  bool _enabled = true;
  bool _autoRotationEnabled = true;
  uint16_t _setupRotationDeg = 0;
  uint16_t _sensorMountingDeg = 0;
  uint8_t _sensorType = static_cast<uint8_t>(MARSensorType::LIS3DH);
  uint8_t _i2cMode = I2C_MODE_MATRIXPORTAL;
  int8_t _customSda = -1;
  int8_t _customScl = -1;
  uint8_t _configuredAddress = 0; // 0 = auto (used in Custom mode)
  bool _showAdvanced = false; // UI preference only; does not change runtime behavior

  float _axisThresholdG = 0.55f;
  float _hysteresisG = 0.12f;
  uint16_t _stableTimeMs = 600;
  uint16_t _pollIntervalMs = 100;
  bool _allow0 = true;
  bool _allow90 = true;
  bool _allow180 = true;
  bool _allow270 = true;

  bool _initDone = false;
  bool _needsReinit = false;
  bool _sensorReady = false;
  bool _customBusStarted = false;
  const char *_status = "not initialized";

  MARAccelerometer _sensor;
  MARAccelSample _lastSample;
  uint32_t _lastSampleAt = 0;
  uint32_t _readErrors = 0;

  int8_t _candidateDirection = DIR_UNKNOWN;
  int8_t _stableDirection = DIR_UNKNOWN;
  uint32_t _candidateSince = 0;
  int8_t _candidateAutoRotation = ORIENT_UNKNOWN;
  int8_t _stableAutoRotation = ORIENT_UNKNOWN;
  uint8_t _effectiveRotation = 0;
  bool _rotationUnsupported = false;

  uint32_t *_frameBuffer = nullptr;
  size_t _frameCapacity = 0;
  uint16_t _bufferWidth = 0;
  uint16_t _bufferHeight = 0;

  void sanitizeConfig() {
    switch (_setupRotationDeg) {
      case 0: case 90: case 180: case 270: break;
      default: _setupRotationDeg = 0; break;
    }
    switch (_sensorMountingDeg) {
      case 0: case 90: case 180: case 270: break;
      default: _sensorMountingDeg = 0; break;
    }
    if (_sensorType > static_cast<uint8_t>(MARSensorType::MPU6050)) _sensorType = static_cast<uint8_t>(MARSensorType::LIS3DH);

    // b002 briefly exposed two generic ESP32 presets. They were too broad for
    // a board-oriented UI, so b003 migrates them to an equivalent Custom bus
    // instead of silently changing the wiring that an upgraded device uses.
    if (_i2cMode == I2C_MODE_LEGACY_ESP32_GENERIC) {
      _i2cMode = I2C_MODE_CUSTOM;
      _customSda = 21;
      _customScl = 22;
    } else if (_i2cMode == I2C_MODE_LEGACY_ESP32S3_DEVKIT) {
      _i2cMode = I2C_MODE_CUSTOM;
      _customSda = 8;
      _customScl = 9;
    } else if (_i2cMode != I2C_MODE_MATRIXPORTAL && _i2cMode != I2C_MODE_CUSTOM) {
      _i2cMode = I2C_MODE_MATRIXPORTAL;
    }

    if (_configuredAddress > 0x7F) _configuredAddress = 0;
    _axisThresholdG = constrain(_axisThresholdG, 0.10f, 1.50f);
    _hysteresisG = constrain(_hysteresisG, 0.0f, 0.50f);
    _stableTimeMs = constrain(_stableTimeMs, uint16_t(0), uint16_t(10000));
    _pollIntervalMs = constrain(_pollIntervalMs, uint16_t(20), uint16_t(2000));
    if (!_allow0 && !_allow90 && !_allow180 && !_allow270) _allow0 = true;
  }

  void freeFrameBuffer() {
    if (_frameBuffer) {
      p_free(_frameBuffer);
      _frameBuffer = nullptr;
    }
    _frameCapacity = 0;
    _bufferWidth = 0;
    _bufferHeight = 0;
  }

  bool ensureFrameBuffer(uint16_t width, uint16_t height) {
    const size_t required = size_t(width) * size_t(height);
    if (!required) return false;
    if (_frameBuffer && _frameCapacity >= required && _bufferWidth == width && _bufferHeight == height) return true;

    freeFrameBuffer();
    _frameBuffer = static_cast<uint32_t*>(allocate_buffer(required * sizeof(uint32_t), BFRALLOC_PREFER_PSRAM | BFRALLOC_NOBYTEACCESS));
    if (!_frameBuffer) {
      _status = "framebuffer allocation failed";
      return false;
    }
    _frameCapacity = required;
    _bufferWidth = width;
    _bufferHeight = height;
    return true;
  }

  void stopCustomBus() {
#if defined(ARDUINO_ARCH_ESP32)
    if (_customBusStarted) marCustomWire.end();
#endif
    _customBusStarted = false;
  }

  TwoWire *prepareI2CBus() {
    stopCustomBus();

    if (_i2cMode == I2C_MODE_MATRIXPORTAL) {
      // Adafruit MatrixPortal ESP32-S3 Arduino variant: SDA=16, SCL=17.
      // Keep Wire.begin() without explicit pins so the board variant remains
      // authoritative; this path is hardware-qualified on the test unit.
      Wire.begin();
      Wire.setClock(400000);
      return &Wire;
    }

#if defined(ARDUINO_ARCH_ESP32)
    if (_customSda < 0 || _customScl < 0 || _customSda == _customScl) {
      _status = "invalid custom I²C pins";
      return nullptr;
    }
    if (!PinManager::isPinOk(uint8_t(_customSda), true) || !PinManager::isPinOk(uint8_t(_customScl), true)) {
      _status = "custom I²C pin not usable";
      return nullptr;
    }

    // Custom pins are checked against WLED's PinManager but are not
    // claimed with a fake/borrowed Usermod ID. This keeps the usermod standalone
    // and avoids impersonating another PinOwner. A dedicated upstream ID can be
    // considered later if this mode becomes a release requirement.
    if (PinManager::isPinAllocated(uint8_t(_customSda)) || PinManager::isPinAllocated(uint8_t(_customScl))) {
      _status = "custom I²C pin conflict";
      return nullptr;
    }

    if (!marCustomWire.begin(_customSda, _customScl, 400000)) {
      _status = "custom I²C begin failed";
      stopCustomBus();
      return nullptr;
    }
    _customBusStarted = true;
    return &marCustomWire;
#else
    _status = "custom I²C requires ESP32";
    return nullptr;
#endif
  }

  void resetOrientationState() {
    _candidateDirection = DIR_UNKNOWN;
    _stableDirection = DIR_UNKNOWN;
    _candidateAutoRotation = ORIENT_UNKNOWN;
    _stableAutoRotation = ORIENT_UNKNOWN;
    _candidateSince = 0;
    _lastSampleAt = 0;
    _readErrors = 0;
    _lastSample = MARAccelSample{};
  }

  void updateEffectiveRotation(bool forceTrigger = false) {
    const uint8_t base = quarterTurnsFromDegrees(_setupRotationDeg);
    uint8_t automatic = 0;
    if (_autoRotationEnabled && _stableAutoRotation != ORIENT_UNKNOWN) automatic = uint8_t(_stableAutoRotation);
    const uint8_t next = normalizeQuarterTurns(int(base) + int(automatic));
    if (next != _effectiveRotation || forceTrigger) {
      _effectiveRotation = next;
      strip.trigger();
    }
  }

  void initializeRuntime() {
    sanitizeConfig();
    resetOrientationState();
    _sensorReady = false;
    _rotationUnsupported = false;
    _status = _enabled ? "ready" : "disabled";

    if (!_enabled) {
      stopCustomBus();
      _effectiveRotation = 0;
      strip.trigger();
      return;
    }

    updateEffectiveRotation(true);

    if (!_autoRotationEnabled) {
      stopCustomBus();
      _status = "static rotation";
      return;
    }

    TwoWire *wire = prepareI2CBus();
    if (!wire) return;

    const uint8_t effectiveAddress = (_i2cMode == I2C_MODE_CUSTOM) ? _configuredAddress : 0;
    _sensorReady = _sensor.begin(*wire, static_cast<MARSensorType>(_sensorType), effectiveAddress);
    if (_sensorReady) {
      _status = "sensor ready";
      DEBUG_PRINTF_P(PSTR("[MatrixAutoRotation] sensor=%s address=0x%02X init=OK\n"), _sensor.name(), _sensor.address());
    } else {
      _status = "sensor not found";
      DEBUG_PRINTF_P(PSTR("[MatrixAutoRotation] sensor=%s init=FAILED\n"), _sensor.name());
    }
  }

  bool orientationAllowed(uint8_t q) const {
    switch (q & 0x03u) {
      case 0: return _allow0;
      case 1: return _allow90;
      case 2: return _allow180;
      case 3: return _allow270;
    }
    return false;
  }

  int8_t classifyGravity(const MARAccelSample &sample) const {
    const float ax = absf(sample.xG);
    const float ay = absf(sample.yG);
    const float strongest = ax > ay ? ax : ay;
    if (strongest < _axisThresholdG) return DIR_UNKNOWN;

    // Same behavior qualified in iDotMatrix Build 164: near 45 degrees retain
    // the existing candidate/stable axis rather than chatter X<->Y.
    if (absf(ax - ay) < _hysteresisG) {
      if (_candidateDirection != DIR_UNKNOWN) return _candidateDirection;
      if (_stableDirection != DIR_UNKNOWN) return _stableDirection;
    }

    if (ax >= ay) return sample.xG >= 0.0f ? DIR_POS_X : DIR_NEG_X;
    return sample.yG >= 0.0f ? DIR_POS_Y : DIR_NEG_Y;
  }

  uint8_t rotationForDirection(int8_t direction) const {
    // MatrixPortal S3 mapping validated on hardware in iDotMatrix Build 164:
    // +Y=0, +X=90, -Y=180, -X=270.
    switch (direction) {
      case DIR_POS_X: return 1;
      case DIR_NEG_Y: return 2;
      case DIR_NEG_X: return 3;
      case DIR_POS_Y:
      default: return 0;
    }
  }

  int8_t normalizeSensorRotation(uint8_t rawRotation) const {
    const uint8_t mounting = quarterTurnsFromDegrees(_sensorMountingDeg);
    const uint8_t normalized = normalizeQuarterTurns(int(rawRotation) - int(mounting));
    return orientationAllowed(normalized) ? int8_t(normalized) : ORIENT_UNKNOWN;
  }

  void pollSensor(uint32_t now) {
    if (!_sensorReady) return;
    if (uint32_t(now - _lastSampleAt) < _pollIntervalMs) return;
    _lastSampleAt = now;

    MARAccelSample sample;
    if (!_sensor.read(sample)) {
      _readErrors++;
      return;
    }
    _lastSample = sample;

    const int8_t nextDirection = classifyGravity(sample);
    const int8_t nextAutoRotation = nextDirection == DIR_UNKNOWN
      ? ORIENT_UNKNOWN
      : normalizeSensorRotation(rotationForDirection(nextDirection));

    // A disallowed orientation behaves as UNKNOWN: keep the last accepted
    // stable orientation and wait until an allowed direction is stable.
    if (nextAutoRotation == ORIENT_UNKNOWN) {
      _candidateAutoRotation = ORIENT_UNKNOWN;
      _candidateDirection = DIR_UNKNOWN;
      _candidateSince = now;
      return;
    }

    if (nextDirection != _candidateDirection || nextAutoRotation != _candidateAutoRotation) {
      _candidateDirection = nextDirection;
      _candidateAutoRotation = nextAutoRotation;
      _candidateSince = now;
      return;
    }

    if (_candidateAutoRotation != _stableAutoRotation && uint32_t(now - _candidateSince) >= _stableTimeMs) {
      _stableDirection = _candidateDirection;
      _stableAutoRotation = _candidateAutoRotation;
      updateEffectiveRotation();
      DEBUG_PRINTF_P(PSTR("[MatrixAutoRotation] auto=%u setup=%u effective=%u\n"),
        degreesFromQuarterTurns(uint8_t(_stableAutoRotation)),
        _setupRotationDeg,
        degreesFromQuarterTurns(_effectiveRotation));
    }
  }

  const char *sensorStatusText() const {
    return _status;
  }

  const char *configuredSensorName() const {
    return _sensorType == static_cast<uint8_t>(MARSensorType::MPU6050)
      ? "GY-521 (MPU-6050)"
      : "LIS3DH";
  }

  const char *directionName(int8_t dir) const {
    switch (dir) {
      case DIR_POS_X: return "+X";
      case DIR_NEG_X: return "-X";
      case DIR_POS_Y: return "+Y";
      case DIR_NEG_Y: return "-Y";
      default: return "UNKNOWN";
    }
  }

public:
  void setup() override {
    initializeRuntime();
    _initDone = true;
  }

  void loop() override {
    if (_needsReinit && !strip.isUpdating()) {
      _needsReinit = false;
      initializeRuntime();
    }
    if (!_enabled || !_autoRotationEnabled || !_sensorReady) return;
    pollSensor(millis());
  }

  void handleOverlayDraw() override {
#ifndef WLED_DISABLE_2D
    if (!_enabled || _effectiveRotation == 0) return;

    const uint16_t width = Segment::maxWidth;
    const uint16_t height = Segment::maxHeight;
    if (width < 2 || height < 2) return;

    // Quarter-turns on non-square matrices change the logical dimensions.
    // This usermod deliberately does not crop/rescale/reconfigure WLED geometry.
    if ((_effectiveRotation == 1 || _effectiveRotation == 3) && width != height) {
      _rotationUnsupported = true;
      return;
    }
    _rotationUnsupported = false;

    const size_t matrixPixels = size_t(width) * size_t(height);
    if (matrixPixels > strip.getLengthTotal()) {
      _status = "matrix geometry exceeds pixel buffer";
      return;
    }
    if (!ensureFrameBuffer(width, height)) return;

    // Snapshot only the matrix raster. Any trailing 1D LEDs remain untouched.
    for (size_t i = 0; i < matrixPixels; i++) _frameBuffer[i] = strip.getPixelColorNoMap(i);

    for (uint16_t y = 0; y < height; y++) {
      for (uint16_t x = 0; x < width; x++) {
        uint16_t sourceX = x;
        uint16_t sourceY = y;
        switch (_effectiveRotation) {
          case 1: // physical display 90 CW -> sample rendered raster 90 CCW
            sourceX = width - 1u - y;
            sourceY = x;
            break;
          case 2:
            sourceX = width - 1u - x;
            sourceY = height - 1u - y;
            break;
          case 3: // physical display 270 CW -> sample rendered raster 90 CW
            sourceX = y;
            sourceY = height - 1u - x;
            break;
          default:
            break;
        }
        const size_t destination = size_t(y) * width + x;
        const size_t source = size_t(sourceY) * width + sourceX;
        strip.setPixelColor(destination, _frameBuffer[source]);
      }
    }
#endif
  }

  void addToJsonInfo(JsonObject &root) override {
    JsonObject user = root["u"];
    if (user.isNull()) user = root.createNestedObject("u");

    JsonArray ver = user.createNestedArray(F("Matrix Auto Rotation"));
    ver.add(MAR_VERSION);

    JsonArray status = user.createNestedArray(F("MAR status"));
    status.add(sensorStatusText());

    char sensorText[48];
    if (_sensorReady) snprintf(sensorText, sizeof(sensorText), "%s @ 0x%02X", _sensor.name(), _sensor.address());
    else snprintf(sensorText, sizeof(sensorText), "%s", configuredSensorName());
    JsonArray sensor = user.createNestedArray(F("MAR sensor"));
    sensor.add(sensorText);

    char accelText[72];
    snprintf(accelText, sizeof(accelText), "X %.3f  Y %.3f  Z %.3f g", _lastSample.xG, _lastSample.yG, _lastSample.zG);
    JsonArray accel = user.createNestedArray(F("MAR acceleration"));
    accel.add(accelText);

    char orientText[96];
    const int stableAutoDeg = _stableAutoRotation == ORIENT_UNKNOWN ? -1 : int(degreesFromQuarterTurns(uint8_t(_stableAutoRotation)));
    snprintf(orientText, sizeof(orientText), "axis %s | auto %d | setup %u | effective %u",
      directionName(_stableDirection), stableAutoDeg, _setupRotationDeg, degreesFromQuarterTurns(_effectiveRotation));
    JsonArray orient = user.createNestedArray(F("MAR orientation"));
    orient.add(orientText);

    char matrixText[64];
    snprintf(matrixText, sizeof(matrixText), "%ux%u%s", Segment::maxWidth, Segment::maxHeight,
      _rotationUnsupported ? " | 90/270 unsupported" : "");
    JsonArray matrix = user.createNestedArray(F("MAR matrix"));
    matrix.add(matrixText);

    if (_readErrors) {
      JsonArray errors = user.createNestedArray(F("MAR I²C read errors"));
      errors.add(_readErrors);
    }
  }

  void addToConfig(JsonObject &root) override {
    JsonObject top = root.createNestedObject(FPSTR(_name));
    top["enabled"] = _enabled;
    top["setup-rotation"] = _setupRotationDeg;
    top["auto-rotation"] = _autoRotationEnabled;
    top["sensor"] = _sensorType;
    top["sensor-mounting"] = _sensorMountingDeg;

    // Keep all bus-related controls in one visible I²C section. Only the
    // Matrix Portal preset and Custom mode are exposed in b003.
    JsonObject i2c = top.createNestedObject("i2c");
    i2c["mode"] = _i2cMode;
    i2c["SDA-pin"] = _customSda;
    i2c["SCL-pin"] = _customScl;
    i2c["address"] = _configuredAddress;

    JsonObject allowed = top.createNestedObject("allow-rotation");
    allowed["0"] = _allow0;
    allowed["90"] = _allow90;
    allowed["180"] = _allow180;
    allowed["270"] = _allow270;

    top["advanced"] = _showAdvanced;
    JsonObject advanced = top.createNestedObject("advanced-settings");
    advanced["threshold-g"] = _axisThresholdG;
    advanced["hysteresis-g"] = _hysteresisG;
    advanced["stable-ms"] = _stableTimeMs;
    advanced["poll-ms"] = _pollIntervalMs;
  }

  bool readFromConfig(JsonObject &root) override {
    JsonObject top = root[FPSTR(_name)];
    bool complete = !top.isNull();

    complete &= getJsonValue(top["enabled"], _enabled, true);
    complete &= getJsonValue(top["setup-rotation"], _setupRotationDeg, uint16_t(0));
    complete &= getJsonValue(top["auto-rotation"], _autoRotationEnabled, true);
    complete &= getJsonValue(top["sensor"], _sensorType, uint8_t(static_cast<uint8_t>(MARSensorType::LIS3DH)));
    complete &= getJsonValue(top["sensor-mounting"], _sensorMountingDeg, uint16_t(0));

    // b003 grouped layout.
    JsonObject i2c = top["i2c"];
    if (!i2c.isNull()) {
      complete &= getJsonValue(i2c["mode"], _i2cMode, uint8_t(I2C_MODE_MATRIXPORTAL));
      complete &= getJsonValue(i2c["SDA-pin"], _customSda, int8_t(-1));
      complete &= getJsonValue(i2c["SCL-pin"], _customScl, int8_t(-1));
      complete &= getJsonValue(i2c["address"], _configuredAddress, uint8_t(0));
    } else {
      // b002: mode at top level and Custom I2C as a nested group.
      // b001: all of these keys were flat. Both forms remain accepted.
      complete = false;
      getJsonValue(top["i2c-mode"], _i2cMode, uint8_t(I2C_MODE_MATRIXPORTAL));
      JsonObject custom = top["custom-I2C"];
      if (!custom.isNull()) {
        getJsonValue(custom["SDA-pin"], _customSda, int8_t(-1));
        getJsonValue(custom["SCL-pin"], _customScl, int8_t(-1));
        getJsonValue(custom["i2c-address"], _configuredAddress, uint8_t(0));
      } else {
        getJsonValue(top["SDA-pin"], _customSda, int8_t(-1));
        getJsonValue(top["SCL-pin"], _customScl, int8_t(-1));
        getJsonValue(top["i2c-address"], _configuredAddress, uint8_t(0));
      }
    }

    JsonObject allowed = top["allow-rotation"];
    if (!allowed.isNull()) {
      complete &= getJsonValue(allowed["0"], _allow0, true);
      complete &= getJsonValue(allowed["90"], _allow90, true);
      complete &= getJsonValue(allowed["180"], _allow180, true);
      complete &= getJsonValue(allowed["270"], _allow270, true);
    } else {
      complete = false;
      getJsonValue(top["allow-0"], _allow0, true);
      getJsonValue(top["allow-90"], _allow90, true);
      getJsonValue(top["allow-180"], _allow180, true);
      getJsonValue(top["allow-270"], _allow270, true);
    }

    complete &= getJsonValue(top["advanced"], _showAdvanced, false);
    JsonObject advanced = top["advanced-settings"];
    if (!advanced.isNull()) {
      complete &= getJsonValue(advanced["threshold-g"], _axisThresholdG, 0.55f);
      complete &= getJsonValue(advanced["hysteresis-g"], _hysteresisG, 0.12f);
      complete &= getJsonValue(advanced["stable-ms"], _stableTimeMs, uint16_t(600));
      complete &= getJsonValue(advanced["poll-ms"], _pollIntervalMs, uint16_t(100));
    } else {
      complete = false;
      getJsonValue(top["threshold-g"], _axisThresholdG, 0.55f);
      getJsonValue(top["hysteresis-g"], _hysteresisG, 0.12f);
      getJsonValue(top["stable-ms"], _stableTimeMs, uint16_t(600));
      getJsonValue(top["poll-ms"], _pollIntervalMs, uint16_t(100));
    }

    sanitizeConfig();
    if (_initDone) _needsReinit = true;
    return complete;
  }

  void appendConfigData(Print &settingsScript) override {
    settingsScript.print(F("dd=addDropdown('")); settingsScript.print(FPSTR(_name)); settingsScript.print(F("','setup-rotation');"));
    settingsScript.print(F("addOption(dd,'0 deg',0);addOption(dd,'90 deg',90);addOption(dd,'180 deg',180);addOption(dd,'270 deg',270);"));

    settingsScript.print(F("dd=addDropdown('")); settingsScript.print(FPSTR(_name)); settingsScript.print(F("','sensor');"));
    settingsScript.print(F("addOption(dd,'LIS3DH',0);addOption(dd,'GY-521 (MPU-6050)',1);"));

    settingsScript.print(F("dd=addDropdown('")); settingsScript.print(FPSTR(_name)); settingsScript.print(F("','sensor-mounting');"));
    settingsScript.print(F("addOption(dd,'0 deg',0);addOption(dd,'90 deg',90);addOption(dd,'180 deg',180);addOption(dd,'270 deg',270);"));

    settingsScript.print(F("dd=addDropdown('")); settingsScript.print(FPSTR(_name)); settingsScript.print(F(":i2c','mode');"));
    settingsScript.print(F("addOption(dd,'Matrix Portal',0);addOption(dd,'Custom',1);"));

    settingsScript.print(F("dd=addDropdown('")); settingsScript.print(FPSTR(_name)); settingsScript.print(F(":i2c','address');"));
    settingsScript.print(F("addOption(dd,'Auto',0);addOption(dd,'0x18',24);addOption(dd,'0x19',25);addOption(dd,'0x68',104);addOption(dd,'0x69',105);"));

    settingsScript.print(F("addInfo('")); settingsScript.print(FPSTR(_name)); settingsScript.print(F(":advanced-settings:threshold-g',1,'Minimum dominant X/Y gravity. Default: 0.55 g.');"));
    settingsScript.print(F("addInfo('")); settingsScript.print(FPSTR(_name)); settingsScript.print(F(":advanced-settings:hysteresis-g',1,'Diagonal X/Y hysteresis. Default: 0.12 g.');"));
    settingsScript.print(F("addInfo('")); settingsScript.print(FPSTR(_name)); settingsScript.print(F(":advanced-settings:stable-ms',1,'Candidate orientation must remain stable for this time.');"));
    settingsScript.print(F("addInfo('")); settingsScript.print(FPSTR(_name)); settingsScript.print(F(":advanced-settings:poll-ms',1,'Accelerometer polling interval.');"));

    // The stock WLED usermod page is intentionally schema-simple: fields are
    // emitted as text/input/<br> sequences. This small DOM pass changes only
    // presentation; persisted JSON keys stay stable and runtime logic is not
    // involved.
    settingsScript.print(F(
      "(()=>{"
      "const N='MatrixAutoRotation:';"
      "const E=n=>[...d.getElementsByName(n)].find(e=>e.type!='hidden');"
      "const L=e=>{if(!e)return null;let n=e.previousSibling;while(n){if(n.nodeType==3&&n.nodeValue.trim())return n;n=n.previousSibling;}return null;};"
      "const W=n=>{let e=E(n);if(!e)return null;let l=L(e),s=l||e,p=e.parentNode,w=cE('div');w.className='mar-row';p.insertBefore(w,s);let x=s;while(x){let y=x.nextSibling;w.appendChild(x);if(x==e){while(y&&!(y.nodeType==1&&y.tagName=='BR')){let z=y.nextSibling;w.appendChild(y);y=z;}if(y&&y.tagName=='BR')y.remove();break;}x=y;}return w;};"
      "const T=(n,t)=>{let e=E(n),l=L(e);if(l)l.nodeValue=' '+t+' ';return e;};"
      "let sec=[...d.querySelectorAll('#um .sec')].find(s=>{let h=s.querySelector('h3');return h&&h.textContent=='MatrixAutoRotation';});if(!sec)return;"
      "sec.querySelectorAll('hr.sml').forEach(h=>h.remove());"
      "T(N+'enabled','Enabled:');T(N+'setup-rotation','Setup Rotation:');T(N+'auto-rotation','Auto Rotation:');T(N+'sensor','Sensor:');T(N+'sensor-mounting','Sensor Mounting:');"
      "T(N+'i2c:mode','Mode:');T(N+'i2c:SDA-pin','SDA Pin:');T(N+'i2c:SCL-pin','SCL Pin:');T(N+'i2c:address','Address:');"
      "T(N+'advanced','Advanced:');T(N+'advanced-settings:threshold-g','Threshold G:');T(N+'advanced-settings:hysteresis-g','Hysteresis G:');T(N+'advanced-settings:stable-ms','Stable Ms:');T(N+'advanced-settings:poll-ms','Poll Ms:');"
      "let sr=W(N+'setup-rotation');if(sr)sr.style.marginBottom='12px';"
      "let heads=[...sec.querySelectorAll('p>u')];"
      "const H=t=>heads.find(u=>u.textContent.trim().toLowerCase()==t.toLowerCase());"
      "let ih=H('I2c');if(ih){ih.innerHTML='I<sup>2</sup>C';ih.style.cssText='font-weight:700;font-size:1.15em;text-decoration:none';ih.parentElement.style.margin='14px 0 8px';}"
      "let ah=H('Allow Rotation');if(ah){ah.textContent='Allow Rotation';ah.style.cssText='font-weight:700;font-size:1.15em;text-decoration:none';ah.parentElement.style.margin='14px 0 8px';}"
      "let xh=H('Advanced Settings');if(xh&&xh.parentElement)xh.parentElement.remove();"
      "let mode=E(N+'i2c:mode'),cr=[W(N+'i2c:SDA-pin'),W(N+'i2c:SCL-pin'),W(N+'i2c:address')].filter(Boolean);"
      "let adv=E(N+'advanced'),ar=[W(N+'advanced-settings:threshold-g'),W(N+'advanced-settings:hysteresis-g'),W(N+'advanced-settings:stable-ms'),W(N+'advanced-settings:poll-ms')].filter(Boolean);"
      "let av=[];[['0','0'],['90','90'],['180','180'],['270','270']].forEach(a=>{T(N+'allow-rotation:'+a[0],a[1]);let r=W(N+'allow-rotation:'+a[0]);if(r){r.style.cssText='display:inline-flex;align-items:center;gap:4px;margin:0';av.push(r);}});"
      "if(ah&&av.length){let box=cE('div');box.style.cssText='display:flex;justify-content:center;align-items:center;gap:18px;flex-wrap:wrap;margin:2px 0 10px';ah.parentElement.insertAdjacentElement('afterend',box);av.forEach(r=>box.appendChild(r));}"
      "let u=()=>{cr.forEach(r=>r.style.display=(mode&&mode.value=='1')?'block':'none');ar.forEach(r=>r.style.display=(adv&&adv.checked)?'block':'none');};"
      "if(mode)mode.addEventListener('change',u);if(adv)adv.addEventListener('change',u);u();"
      "})();"));
  }


};

const char MatrixAutoRotationUsermod::_name[] PROGMEM = "MatrixAutoRotation";

static MatrixAutoRotationUsermod matrixAutoRotationUsermod;
REGISTER_USERMOD(matrixAutoRotationUsermod);
