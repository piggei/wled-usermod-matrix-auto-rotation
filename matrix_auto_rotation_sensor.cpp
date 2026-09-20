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
constexpr uint8_t MPU6050_REG_ACCEL_XOUT_H = 0x3B;
constexpr uint8_t MPU6050_REG_PWR_MGMT_1 = 0x6B;
constexpr uint8_t MPU6050_REG_WHO_AM_I = 0x75;
}

const char *MARAccelerometer::name() const {
  return _type == MARSensorType::MPU6050 ? "GY-521 (MPU-6050)" : "LIS3DH";
}

bool MARAccelerometer::begin(TwoWire &wire, MARSensorType type, uint8_t configuredAddress) {
  _wire = &wire;
  _type = type;
  _address = 0;
  _ready = false;

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
  if (_wire->endTransmission() != 0) return false;

  const uint8_t previous = _address;
  _address = address;
  uint8_t who = 0;
  const bool readOk = readRegister(_type == MARSensorType::LIS3DH ? LIS3DH_REG_WHO_AM_I : MPU6050_REG_WHO_AM_I, who);
  _address = previous;
  if (!readOk) return false;

  if (_type == MARSensorType::LIS3DH) return who == LIS3DH_WHO_AM_I;
  // Genuine MPU-6050 parts normally return 0x68. Accept 0x69 as well for
  // compatible modules/clones that reflect AD0 in WHO_AM_I.
  return who == 0x68 || who == 0x69;
}

bool MARAccelerometer::initLIS3DH() {
  // 50 Hz, XYZ enabled.
  if (!writeRegister(LIS3DH_REG_CTRL1, 0x47)) return false;
  // BDU + high-resolution mode, +/-2 g.
  if (!writeRegister(LIS3DH_REG_CTRL4, 0x88)) return false;
  delay(5);
  return true;
}

bool MARAccelerometer::initMPU6050() {
  // Wake device and use X-axis gyro PLL as the clock source.
  if (!writeRegister(MPU6050_REG_PWR_MGMT_1, 0x01)) return false;
  delay(5);
  // DLPF ~44 Hz (gyro) / ~42 Hz (accelerometer).
  if (!writeRegister(MPU6050_REG_CONFIG, 0x03)) return false;
  // 1 kHz / (1 + 19) = 50 Hz internal sample cadence with DLPF enabled.
  if (!writeRegister(MPU6050_REG_SMPLRT_DIV, 19)) return false;
  // Accelerometer full scale +/-2 g.
  if (!writeRegister(MPU6050_REG_ACCEL_CONFIG, 0x00)) return false;
  delay(5);
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
  if (!_wire || !_address || !data || !len) return false;

  _wire->beginTransmission(_address);
  _wire->write(lisAutoIncrement ? uint8_t(reg | 0x80u) : reg);
  if (_wire->endTransmission(false) != 0) return false;

  const size_t received = _wire->requestFrom(_address, static_cast<uint8_t>(len), true);
  if (received != len) {
    while (_wire->available()) (void)_wire->read();
    return false;
  }
  for (size_t i = 0; i < len; i++) data[i] = static_cast<uint8_t>(_wire->read());
  return true;
}

bool MARAccelerometer::writeRegister(uint8_t reg, uint8_t value) {
  if (!_wire || !_address) return false;
  _wire->beginTransmission(_address);
  _wire->write(reg);
  _wire->write(value);
  return _wire->endTransmission() == 0;
}
