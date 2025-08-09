#pragma once

#include "../ld2410s.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

namespace esphome {
namespace ld2410s {

class LD2410SCalibrationUpdateBinarySensor : public LD2410SListener,
                                             public Component,
                                             public binary_sensor::BinarySensor {
 public:
  void set_calibration_update_sensor(binary_sensor::BinarySensor *bsensor) {
    this->calibration_update_bsensor_ = bsensor;
  };
  void on_calibration_update(bool running) override {
    if (this->calibration_update_bsensor_ != nullptr) {
      if (this->calibration_update_bsensor_->state != running) {
        this->calibration_update_bsensor_->publish_state(running);
      }
    }
  };

 private:
  binary_sensor::BinarySensor *calibration_update_bsensor_{nullptr};
};

}  // namespace ld2410s
}  // namespace esphome
