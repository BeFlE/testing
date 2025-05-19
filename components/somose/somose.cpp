#include "somose.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace somose {

static const char *const TAG = "somose";

void SOMOSE::setup() {
  ESP_LOGCONFIG(TAG, "Setting up SOMOSE...");

  uint8_t command = 0x64;
  uint8_t value[2];

  auto read_two_bytes = [&](uint8_t cmd, uint16_t &target, const char *log_message) {
    if (this->write(&cmd, 1) != i2c::ERROR_OK) {
      ESP_LOGE(TAG, "Write failed for %s!", log_message);
      this->status_set_warning();
      return false;
    }
    delay(1); // Maybe some delay is required
    if (this->read(value, 2) != i2c::ERROR_OK) {
      ESP_LOGE(TAG, "Read failed for %s!", log_message);
      this->status_set_warning();
      return false;
    }
    target = value[0] * 256 + value[1];
    ESP_LOGD(TAG, "Read %s value %d (%d, %d).", log_message, target, value[0], value[1]);
    return true;
  };

  if (!read_two_bytes(0x64, this->moisture_min_, "moisture min")) {
    return;
  }

  if (!read_two_bytes(0x75, this->moisture_max_, "moisture max")) {
    return;
  }

  // get status - commented out in original code
  /*
  uint8_t status = 0;
  if (this->read(&status, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Read status failed!");
    this->mark_failed();
    return;
  }

  // reset registers if required
  if ((status & 0x18) != 0x18) {
    ESP_LOGD(TAG, "Resetting SOMOSE registers");
    if (!this->reset_register_(0x1B) || !this->reset_register_(0x1C) || !this->reset_register_(0x1E)) {
      this->mark_failed();
      return;
    }
  }
  */
}

void SOMOSE::set_new_i2c_address(uint8_t old_addr, uint8_t new_addr) {
  ESP_LOGD(TAG, "Setting new I2C address from 0x%02X to 0x%02X", old_addr, new_addr);
  uint8_t command = 0x41;
  uint8_t new_addr_byte = (new_addr << 1) & 0xFE;
  uint8_t buf[2];

  buf[0] = command;
  buf[1] = new_addr_byte;
  if (this->write(&buf, 2) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Failed to set new I2C address!");
    this->status_set_warning();
  }
}

bool SOMOSE::set_reference_dry(uint16_t new_value) {
  ESP_LOGD(TAG, "Setting dry reference to %d", new_value);
  uint8_t command = 0x44;
  uint8_t value_low = new_value & 0xFF;
  uint8_t value_high = (new_value >> 8) & 0xFF;
  uint8_t buf[3];

  buf[0] = command;
  buf[1] = value_low;
  buf[2] = value_high;
  if (this->write(&buf,3) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Failed to set dry reference!");
    this->status_set_warning();
    return false;
  }
  return true;
}

bool SOMOSE::set_reference_wet(uint16_t new_value) {
  ESP_LOGD(TAG, "Setting wet reference to %d", new_value);
  uint8_t command = 0x55;
  uint8_t value_low = new_value & 0xFF;
  uint8_t value_high = (new_value >> 8) & 0xFF;
  uint8_t buf[3];
  
  buf[0] = command;
  buf[1] = value_low;
  buf[2] = value_high;
  if (this->write(&buf,3) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Failed to set wet reference!");
    this->status_set_warning();
    return false;
  }
  return true;
}

uint8_t SOMOSE::get_averaged_sensor_value_() {
  uint8_t command = 0x76;
  uint8_t value = 0;
  uint8_t dump;
  uint8_t buf[2];

  if (this->write(&command, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Write failed for averaged sensor value!");
    this->status_set_warning();
    return 0;
  }
  delay(1);
  if (this->read(&buf, 2) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Read failed for averaged sensor value!");
    this->status_set_warning();
    return 0;
  }
  dump = buf[0];
  value = Buf[1];
  ESP_LOGD(TAG, "Read averaged sensor value (dump: %d, value: %d)", dump, value);
  return value;
}

uint16_t SOMOSE::get_raw_sensor_value() {
  uint8_t command = 'r';
  uint8_t value[2];

  if (this->write(&command, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Write failed for raw sensor value (Arduino-like)!");
    this->status_set_warning();
    return 0;
  }
  delay(1);
  if (this->read(&value, 2) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Read failed for raw sensor value (Arduino-like)!");
    this->status_set_warning();
    return 0;
  }
  uint16_t raw_value = (static_cast<uint16_t>(value[0]) << 8) | value[1];
  ESP_LOGD(TAG, "Read raw sensor value (Arduino-like): %d (%d, %d)", raw_value, value[0], value[1]);
  return raw_value;
}

uint8_t SOMOSE::get_sensor_value() {
  uint8_t command = 0x76;
  uint8_t value = 0;
  uint8_t dump;

  if (this->write(&command, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Write failed for aliased sensor value!");
    this->status_set_warning();
    return 0;
  }
  delay(1);
  if (this->read({&dump, &value}, 2) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Read failed for aliased sensor value!");
    this->status_set_warning();
    return 0;
  }
  ESP_LOGD(TAG, "Read aliased sensor value (dump: %d, value: %d)", dump, value);
  return value;
}

int8_t SOMOSE::get_temperature_value_signed_() {
  uint8_t command = 0x74;
  int8_t value = 0;

  if (this->write(&command, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Write failed for signed temperature value!");
    this->status_set_warning();
    return 0;
  }
  delay(1);
  if (this->read(reinterpret_cast<uint8_t *>(&value), 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Read failed for signed temperature value!");
    this->status_set_warning();
    return 0;
  }
  ESP_LOGD(TAG, "Read signed temperature value: %d", value);
  return value;
}

uint16_t SOMOSE::get_reference_dry_value_() {
  uint8_t command = 0x64;
  uint8_t value[2];
  uint16_t ref_dry_value = 0;

  if (this->write(&command, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Write failed to read dry reference!");
    this->status_set_warning();
    return 0;
  }
  delay(1);
  if (this->read(value, 2) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Failed to read dry reference value!");
    this->status_set_warning();
    return 0;
  }
  ref_dry_value = (static_cast<uint16_t>(value[0]) << 8) | value[1];
  ESP_LOGD(TAG, "Read dry reference value: %d (%d, %d)", ref_dry_value, value[0], value[1]);
  return ref_dry_value;
}

uint16_t SOMOSE::get_reference_wet_value_() {
  uint8_t command = 0x75;
  uint8_t value[2];
  uint16_t ref_wet_value = 0;

  if (this->write(&command, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Write failed to read wet reference!");
    this->status_set_warning();
    return 0;
  }
  delay(1);
  if (this->read(value, 2) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Failed to read wet reference value!");
    this->status_set_warning();
    return 0;
  }
  ref_wet_value = (static_cast<uint16_t>(value[0]) << 8) | value[1];
  ESP_LOGD(TAG, "Read wet reference value: %d (%d, %d)", ref_wet_value, value[0], value[1]);
  return ref_wet_value;
}

float SOMOSE::get_hw_version_() {
  uint8_t command = 'h';
  uint8_t response[4];

  if (this->write(&command, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Failed to write command for HW version!");
    return 0.0f;
  }
  delay(1);
  if (this->read(response, 4) != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "Failed to read HW version. Assuming v2.2.");
    return 2.2f;
  }

  if (response[0] != 'v') {
    ESP_LOGW(TAG, "Invalid HW version response. Assuming v2.2.");
    return 2.2f;
  }
  uint8_t major = response[1] - '0';
  if (response[2] != '.') {
    ESP_LOGW(TAG, "Invalid HW version format. Assuming v2.2.");
    return 2.2f;
  }
  uint8_t minor = response[3] - '0';
  float version = static_cast<float>(major) + static_cast<float>(minor) / 10.0f;
  ESP_LOGD(TAG, "Read HW version: %.1f", version);
  return version;
}

float SOMOSE::get_fw_version_() {
  uint8_t command = 'f';
  uint8_t response[4];

  if (this->write(&command, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Failed to write command for FW version!");
    return 0.0f;
  }
  delay(1);
  if (this->read(response, 4) != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "Failed to read FW version. Assuming v1.2.");
    return 1.2f;
  }

  if (response[0] != 'v') {
    ESP_LOGW(TAG, "Invalid FW version response. Assuming v1.2.");
    return 1.2f;
  }
  uint8_t major = response[1] - '0';
  if (response[2] != '.') {
    ESP_LOGW(TAG, "Invalid FW version format. Assuming v1.2.");
    return 1.2f;
  }
  uint8_t minor = response[3] - '0';
  float version = static_cast<float>(major) + static_cast<float>(minor) / 10.0f;
  ESP_LOGD(TAG, "Read FW version: %.1f", version);
  return version;
}

bool SOMOSE::get_low_power_mode_() {
  uint8_t command = 'o';
  uint8_t status = 0;

  if (this->write(&command, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Failed to write command for low power mode status!");
    return false;
  }
  delay(1);
  if (this->read(&status, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Failed to read low power mode status!");
    return false;
  }
  bool low_power = (status & 0x02) != 0;
  ESP_LOGD(TAG, "Low power mode is: %s", low_power ? "on" : "off");
  return low_power;
}

bool SOMOSE::set_low_power_mode(bool turn_on) {
  ESP_LOGD(TAG, "Setting low power mode to: %s", turn_on ? "on" : "off");
  uint8_t command = 'L';
  uint8_t value = turn_on ? 1 : 0;
  if (this->write({command, value}) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Failed to set low power mode!");
    this->status_set_warning();
    return false;
  }
  delay(25);
  return true;
}

bool SOMOSE::start_measurement(uint8_t repetitions) {
  ESP_LOGD(TAG, "Starting measurement with %d repetitions", repetitions);
  uint8_t command = 'M';
  if (this->write({command, repetitions}) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Failed to start measurement!");
    this->status_set_warning();
    return false;
  }
  return true;
}

bool SOMOSE::is_measurement_finished_() {
  uint8_t command = 'o';
  uint8_t status = 0;

  if (this->write(&command, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Failed to write command for measurement finished status!");
    return false;
  }
  delay(1);
  if (this->read(&status, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Failed to read measurement finished status!");
    return false;
  }
  bool finished = !(status & 0x01);
  ESP_LOGD(TAG, "Measurement finished: %s", finished ? "yes" : "no");
  return finished;
}


void SOMOSE::update() {
  ESP_LOGD(TAG, "SOMOSE::update");

  float temperature = :get_temperature_value_signed_() * 1.0f;
  float moisture = static_cast<float>(get_averaged_sensor_value_());

  if (this->temperature_sensor_ != nullptr) {
    this->temperature_sensor_->publish_state(temperature);
  }
  if (this->moisture_sensor_ != nullptr) {
    this->moisture_sensor_->publish_state(moisture);
  }

  this->status_clear_warning();
}

void SOMOSE::dump_config() {
  ESP_LOGCONFIG(TAG, "SOMOSE:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with SOMOSE failed!");
  }
  LOG_SENSOR("  ", "Temperature", this->temperature_sensor_);
  LOG_SENSOR("  ", "Moisture", this->moisture_sensor_);
}

float SOMOSE::get_setup_priority() const {
  return setup_priority::DATA;
}

} // namespace somose
} // namespace esphome
