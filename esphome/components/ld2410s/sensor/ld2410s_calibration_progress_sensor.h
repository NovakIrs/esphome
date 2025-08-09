#pragma once

#include "../ld2410s.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace ld2410s {

class LD2410SCalibrationProgressSensor : public LD2410SListener, public Component, sensor::Sensor {
 public:
  void set_calibration_progress_sensor(sensor::Sensor *sensor) { this->calibration_progress_sensor_ = sensor; }
  void on_calibration_progress(uint16_t progress) override {
    if (this->calibration_progress_sensor_ != nullptr) {
      if (this->calibration_progress_sensor_->get_state() != progress) {
        this->calibration_progress_sensor_->publish_state(progress);
      }
    }
  };

 private:
  sensor::Sensor *calibration_progress_sensor_{nullptr};
};

}  // namespace ld2410s
}  // namespace esphome
