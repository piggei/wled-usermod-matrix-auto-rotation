#pragma once

#include <Arduino.h>
#include <Wire.h>

enum class MARSensorType : uint8_t {
  LIS3DH = 0,
  MPU6050 = 1
};

enum class MARSensorInitError : uint8_t {
  NONE = 0,
  NO_RESPONSE,
  WHO_AM_I_READ_FAILED,
  WHO_AM_I_MISMATCH,
  CONFIG_WRITE_FAILED,
  CONFIG_VERIFY_FAILED
};

struct MARAccelSample {
  float xG = 0.0f;
  float yG = 0.0f;
  float zG = 0.0f;
};

class MARAccelerometer {
public:
  bool begin(TwoWire &wire, MARSensorType type, uint8_t configuredAddress = 0);
  bool read(MARAccelSample &sample);

  bool ready() const { return _ready; }
  uint8_t address() const { return _address; }
  MARSensorType type() const { return _type; }
  const char *name() const;
  uint8_t whoAmI() const { return _whoAmI; }
  MARSensorInitError initError() const { return _initError; }
  const char *initErrorText() const;

private:
  bool probeAddress(uint8_t address);
  bool initLIS3DH();
  bool initMPU6050();
  bool readLIS3DH(MARAccelSample &sample);
  bool readMPU6050(MARAccelSample &sample);
  bool readRegister(uint8_t reg, uint8_t &value);
  bool readRegisters(uint8_t reg, uint8_t *data, size_t len, bool lisAutoIncrement = false);
  bool writeRegister(uint8_t reg, uint8_t value);
  bool verifyRegisterMasked(uint8_t reg, uint8_t mask, uint8_t expected);

  TwoWire *_wire = nullptr;
  MARSensorType _type = MARSensorType::LIS3DH;
  uint8_t _address = 0;
  bool _ready = false;
  uint8_t _whoAmI = 0;
  MARSensorInitError _initError = MARSensorInitError::NONE;
};
