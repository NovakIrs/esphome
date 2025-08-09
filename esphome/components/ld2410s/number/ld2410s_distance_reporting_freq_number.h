#pragma once

#include "../ld2410s.h"
#include "esphome/components/number/number.h"

namespace esphome {
namespace ld2410s {

class LD2410SDistReportingFreqNumber : public number::Number, public Parented<LD2410S> {
 public:
  LD2410SDistReportingFreqNumber() = default;

 protected:
  void control(float distance_reporting_freq) override {
    this->publish_state(distance_reporting_freq);
    this->parent_->set_distance_reporting_freq(distance_reporting_freq);
  }
};

}  // namespace ld2410s
}  // namespace esphome
