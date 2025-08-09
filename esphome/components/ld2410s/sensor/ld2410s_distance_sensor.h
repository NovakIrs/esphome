#pragma once

#include "../ld2410s.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace ld2410s {

class LD2410SDistanceSensor : public LD2410SListener, public Component, public sensor::Sensor {
 public:
  void set_distance_sensor(sensor::Sensor *sensor) { this->distance_sensor_ = sensor; }
  void on_distance(uint16_t distance) override {
    if (this->distance_sensor_ != nullptr) {
      if (this->distance_sensor_->get_state() != distance) {
        this->distance_sensor_->publish_state(distance);
      }
    }
  }

 private:
  sensor::Sensor *distance_sensor_{nullptr};
};

}  // namespace ld2410s
}  // namespace esphome
