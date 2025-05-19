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

uint8_t SOMOSE::getSensorValue_() {
  uint8_t command = 0x76;
  uint8_t value = 0;
  uint8_t dump;

  if (this->write(&command, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Write failed for sensor value!");
    this->status_set_warning();
    return 0;
  }

  delay(1); // Maybe some delay is required

  if (this->read(&dump, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Read dump failed for sensor value!");
    this->status_set_warning();
    return 0;
  }

  if (this->read(&value, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Read moisture failed!");
    this->status_set_warning();
    return 0;
  }

  return value;
}

uint16_t SOMOSE::getSensorRAWValue_() {
  uint8_t command = 0x72;
  uint8_t value[2];

  if (this->write(&command, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Write failed for raw sensor value!");
    this->status_set_warning();
    return 0;
  }

  delay(1); // Maybe some delay is required

  if (this->read(value, 2) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Read raw value failed!");
    this->status_set_warning();
    return 0;
  }

  ESP_LOGD(TAG, "Read moisture raw value %d (%d, %d).", value[0] * 256 + value[1], value[0], value[1]);
  return value[0] * 256 + value[1];
}

uint8_t SOMOSE::getTemperatureValue_() {
  uint8_t command = 0x74;
  uint8_t value = 0;

  if (this->write(&command, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Write failed for temperature!");
    this->status_set_warning();
    return 0;
  }

  delay(1); // Maybe some delay is required

  if (this->read(&value, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Read temperature failed!");
    this->status_set_warning();
    return 0;
  }
  return value;
}

void SOMOSE::update() {
  ESP_LOGD(TAG, "SOMOSE::update");

  float temperature = getTemperatureValue_() * 1.0f;
  float moisture = static_cast<float>(getSensorRAWValue_()) * 100.0f / 8000.0f;

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
