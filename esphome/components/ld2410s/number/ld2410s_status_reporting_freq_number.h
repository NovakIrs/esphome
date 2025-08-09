#pragma once

#include "../ld2410s.h"
#include "esphome/components/number/number.h"

namespace esphome {
namespace ld2410s {

class LD2410SStatusReportingFreqNumber : public number::Number, public Parented<LD2410S> {
 public:
  LD2410SStatusReportingFreqNumber() = default;

 protected:
  void control(float status_reporting_freq) override {
    this->publish_state(status_reporting_freq);
    this->parent_->set_status_reporting_freq(status_reporting_freq);
  }
};

}  // namespace ld2410s
}  // namespace esphome
