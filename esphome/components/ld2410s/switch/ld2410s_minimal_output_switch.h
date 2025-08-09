#pragma once

#include "esphome/components/switch/switch.h"
#include "../ld2410s.h"

namespace esphome {
namespace ld2410s {

class LD2410SMinimalOutputSwitch : public switch_::Switch, public Parented<LD2410S> {
 public:
  LD2410SMinimalOutputSwitch() = default;

 protected:
  void write_state(bool state) override {
    this->publish_state(state);
    this->parent_->set_minimal_output(state);
  }
};

}  // namespace ld2410s
}  // namespace esphome
