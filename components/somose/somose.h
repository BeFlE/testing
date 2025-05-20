#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace somose {

enum EnergyMode_t {
  continous,
  energy_saving
};

class SOMOSE : public PollingComponent, public i2c::I2CDevice {
 public:
  void dump_config() override;
  void update() override;
  void setup() override;
  float get_setup_priority() const override;

  void set_temperature_sensor(sensor::Sensor *temperature_sensor) { this->temperature_sensor_ = temperature_sensor; }
  void set_moisture_sensor(sensor::Sensor *moisture_sensor) { this->moisture_sensor_ = moisture_sensor; }

 protected:
   void set_new_i2c_address(uint8_t old_addr, uint8_t new_addr);
  bool set_reference_dry(uint16_t new_value);
  bool set_reference_wet(uint16_t new_value);
  uint8_t get_averaged_sensor_value_();
  uint16_t get_raw_sensor_value();
  uint8_t get_sensor_value();
  int8_t get_temperature_value_signed_();
  uint16_t get_reference_dry_value_();
  uint16_t get_reference_wet_value_();
  float get_hw_version_();
  float get_fw_version_();
  bool get_low_power_mode_();
  bool set_low_power_mode(bool turn_on);
  bool start_measurement(uint8_t repetitions);
  bool is_measurement_finished_();
  uint8_t get_sensor_value_();
  uint16_t get_sensor_raw_value_();
  uint8_t get_temperature_value_();
  sensor::Sensor *temperature_sensor_{nullptr};
  sensor::Sensor *moisture_sensor_{nullptr};
  uint16_t moisture_min_{0};
  uint16_t moisture_max_{0};
  EnergyMode_t EnergyMode;
};

} // namespace somose
} // namespace esphome
