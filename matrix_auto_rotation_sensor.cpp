#include "matrix_auto_rotation_sensor.h"
namespace {
constexpr uint8_t LIS3DH_ADDR_PRIMARY = 0x19;   // MatrixPortal S3 onboard sensor
constexpr uint8_t LIS3DH_ADDR_SECONDARY = 0x18;
constexpr uint8_t LIS3DH_REG_WHO_AM_I = 0x0F;
constexpr uint8_t LIS3DH_WHO_AM_I = 0x33;
constexpr uint8_t LIS3DH_REG_CTRL1 = 0x20;
constexpr uint8_t LIS3DH_REG_CTRL4 = 0x23;
constexpr uint8_t LIS3DH_REG_OUT_X_L = 0x28;

constexpr uint8_t MPU6050_ADDR_PRIMARY = 0x68;
constexpr uint8_t MPU6050_ADDR_SECONDARY = 0x69;
constexpr uint8_t MPU6050_REG_SMPLRT_DIV = 0x19;
constexpr uint8_t MPU6050_REG_CONFIG = 0x1A;
constexpr uint8_t MPU6050_REG_ACCEL_CONFIG = 0x1C;
constexpr uint8_t MPU_FAMILY_REG_ACCEL_CONFIG2 = 0x1D;
constexpr uint8_t MPU6050_REG_ACCEL_XOUT_H = 0x3B;
constexpr uint8_t MPU6050_REG_PWR_MGMT_1 = 0x6B;
constexpr uint8_t MPU6050_REG_WHO_AM_I = 0x75;
}

const char *MARAccelerometer::name() const {
  if (_type != MARSensorType::MPU6050) return "LIS3DH";
  if (_whoAmI == 0x98) return "ICM-20689";
  if (_whoAmI == 0x68 || _whoAmI == 0x69) return "MPU-6050";
  return "MPU-6050 / ICM-20689";
}

const char *MARAccelerometer::initErrorText() const {
  switch (_initError) {
    case MARSensorInitError::NO_RESPONSE: return "no I2C response";
    case MARSensorInitError::WHO_AM_I_READ_FAILED: return "WHO_AM_I read failed";
    case MARSensorInitError::WHO_AM_I_MISMATCH: return "WHO_AM_I mismatch";
    case MARSensorInitError::CONFIG_WRITE_FAILED: return "sensor config write failed";
    case MARSensorInitError::CONFIG_VERIFY_FAILED: return "sensor config verify failed";
    case MARSensorInitError::NONE:
    default: return "OK";
  }
}

bool MARAccelerometer::begin(TwoWire &wire, MARSensorType type, uint8_t configuredAddress) {
  _wire = &wire;
  _type = type;
  _address = 0;
  _ready = false;
  _whoAmI = 0;
  _initError = MARSensorInitError::NONE;

  if (configuredAddress != 0) {
    if (!probeAddress(configuredAddress)) return false;
    _address = configuredAddress;
  } else if (_type == MARSensorType::LIS3DH) {
    if (probeAddress(LIS3DH_ADDR_PRIMARY)) _address = LIS3DH_ADDR_PRIMARY;
    else if (probeAddress(LIS3DH_ADDR_SECONDARY)) _address = LIS3DH_ADDR_SECONDARY;
    else return false;
  } else {
    if (probeAddress(MPU6050_ADDR_PRIMARY)) _address = MPU6050_ADDR_PRIMARY;
    else if (probeAddress(MPU6050_ADDR_SECONDARY)) _address = MPU6050_ADDR_SECONDARY;
    else return false;
  }

  _ready = (_type == MARSensorType::LIS3DH) ? initLIS3DH() : initMPU6050();
  return _ready;
}


bool MARAccelerometer::probeAddress(uint8_t address) {
  if (!_wire) return false;
  _wire->beginTransmission(address);
  if (_wire->endTransmission() != 0) {
    _initError = MARSensorInitError::NO_RESPONSE;
    return false;
  }

  const uint8_t previous = _address;
  _address = address;
  uint8_t who = 0;
  const bool readOk = readRegister(_type == MARSensorType::LIS3DH ? LIS3DH_REG_WHO_AM_I : MPU6050_REG_WHO_AM_I, who);
  _address = previous;
  if (!readOk) {
    _initError = MARSensorInitError::WHO_AM_I_READ_FAILED;
    return false;
  }

  _whoAmI = who;
  const bool matches = (_type == MARSensorType::LIS3DH)
    ? (who == LIS3DH_WHO_AM_I)
    : (who == 0x68 || who == 0x69 || who == 0x98);
  if (!matches) {
    _initError = MARSensorInitError::WHO_AM_I_MISMATCH;
    return false;
  }

  _initError = MARSensorInitError::NONE;
  return true;
}

bool MARAccelerometer::initLIS3DH() {
  // 50 Hz, XYZ enabled.
  if (!writeRegister(LIS3DH_REG_CTRL1, 0x47) ||
      !writeRegister(LIS3DH_REG_CTRL4, 0x88)) {
    _initError = MARSensorInitError::CONFIG_WRITE_FAILED;
    return false;
  }
  delay(5);
  _initError = MARSensorInitError::NONE;
  return true;
}

bool MARAccelerometer::initMPU6050() {
  // Wake device and use X-axis gyro PLL as the clock source.
  if (!writeRegister(MPU6050_REG_PWR_MGMT_1, 0x01)) {
    _initError = MARSensorInitError::CONFIG_WRITE_FAILED;
    return false;
  }
  delay(10);

  // Common MPU-60x0 / ICM-20689 setup: 50 Hz output and +/-2g.
  // On ICM-20689 the accelerometer has its own DLPF register (0x1D),
  // while MPU-6050 uses the common CONFIG path.
  if (!writeRegister(MPU6050_REG_CONFIG, 0x03) ||
      !writeRegister(MPU6050_REG_SMPLRT_DIV, 19) ||
      !writeRegister(MPU6050_REG_ACCEL_CONFIG, 0x00) ||
      (_whoAmI == 0x98 && !writeRegister(MPU_FAMILY_REG_ACCEL_CONFIG2, 0x03))) {
    _initError = MARSensorInitError::CONFIG_WRITE_FAILED;
    return false;
  }
  delay(5);

  // Read back the relevant bits. This catches wiring/power problems and
  // modules that ACK the address but are not behaving like a supported MPU-family device.
  if (!verifyRegisterMasked(MPU6050_REG_PWR_MGMT_1, 0x47, 0x01) ||
      !verifyRegisterMasked(MPU6050_REG_CONFIG, 0x07, 0x03) ||
      !verifyRegisterMasked(MPU6050_REG_SMPLRT_DIV, 0xFF, 19) ||
      !verifyRegisterMasked(MPU6050_REG_ACCEL_CONFIG, 0x18, 0x00) ||
      (_whoAmI == 0x98 && !verifyRegisterMasked(MPU_FAMILY_REG_ACCEL_CONFIG2, 0x0F, 0x03))) {
    _initError = MARSensorInitError::CONFIG_VERIFY_FAILED;
    return false;
  }

  _initError = MARSensorInitError::NONE;
  return true;
}

bool MARAccelerometer::read(MARAccelSample &sample) {
  if (!_ready || !_wire || !_address) return false;
  return _type == MARSensorType::LIS3DH ? readLIS3DH(sample) : readMPU6050(sample);
}

bool MARAccelerometer::readLIS3DH(MARAccelSample &sample) {
  uint8_t data[6];
  if (!readRegisters(LIS3DH_REG_OUT_X_L, data, sizeof(data), true)) return false;

  // High-resolution +/-2g data is 12-bit, left-aligned. Sensitivity = 1 mg/LSB.
  const int16_t rawX = static_cast<int16_t>((uint16_t(data[1]) << 8) | data[0]) >> 4;
  const int16_t rawY = static_cast<int16_t>((uint16_t(data[3]) << 8) | data[2]) >> 4;
  const int16_t rawZ = static_cast<int16_t>((uint16_t(data[5]) << 8) | data[4]) >> 4;
  sample.xG = rawX * 0.001f;
  sample.yG = rawY * 0.001f;
  sample.zG = rawZ * 0.001f;
  return true;
}

bool MARAccelerometer::readMPU6050(MARAccelSample &sample) {
  uint8_t data[6];
  if (!readRegisters(MPU6050_REG_ACCEL_XOUT_H, data, sizeof(data))) return false;

  const int16_t rawX = static_cast<int16_t>((uint16_t(data[0]) << 8) | data[1]);
  const int16_t rawY = static_cast<int16_t>((uint16_t(data[2]) << 8) | data[3]);
  const int16_t rawZ = static_cast<int16_t>((uint16_t(data[4]) << 8) | data[5]);
  constexpr float scale = 1.0f / 16384.0f; // +/-2g
  sample.xG = rawX * scale;
  sample.yG = rawY * scale;
  sample.zG = rawZ * scale;
  return true;
}

bool MARAccelerometer::readRegister(uint8_t reg, uint8_t &value) {
  return readRegisters(reg, &value, 1);
}

bool MARAccelerometer::readRegisters(uint8_t reg, uint8_t *data, size_t len, bool lisAutoIncrement) {
  if (!_address || !data || !len) return false;
  const uint8_t registerAddress = lisAutoIncrement ? uint8_t(reg | 0x80u) : reg;


  if (!_wire) return false;
  _wire->beginTransmission(_address);
  _wire->write(registerAddress);
  if (_wire->endTransmission(false) != 0) return false;

  const size_t received = _wire->requestFrom(_address, static_cast<uint8_t>(len), true);
  if (received != len) {
    while (_wire->available()) (void)_wire->read();
    return false;
  }
  for (size_t i = 0; i < len; i++) data[i] = static_cast<uint8_t>(_wire->read());
  return true;
}

bool MARAccelerometer::verifyRegisterMasked(uint8_t reg, uint8_t mask, uint8_t expected) {
  uint8_t value = 0;
  return readRegister(reg, value) && ((value & mask) == (expected & mask));
}

bool MARAccelerometer::writeRegister(uint8_t reg, uint8_t value) {
  if (!_address) return false;


  if (!_wire) return false;
  _wire->beginTransmission(_address);
  _wire->write(reg);
  _wire->write(value);
  return _wire->endTransmission() == 0;
}
