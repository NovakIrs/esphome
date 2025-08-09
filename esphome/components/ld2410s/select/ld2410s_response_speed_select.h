#pragma once

#include "../ld2410s.h"
#include "esphome/components/select/select.h"

namespace esphome {
namespace ld2410s {

class LD2410sResponseSpeedSelect : public Component, public select::Select, public Parented<LD2410S> {
 public:
  LD2410sResponseSpeedSelect() = default;

 protected:
  void control(const std::string &response_speed_select) override {
    this->publish_state(response_speed_select);
    this->parent_->set_response_speed_select(response_speed_select);
  }
};

}  // namespace ld2410s
}  // namespace esphome
