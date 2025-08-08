#pragma once

#include "esphome/components/button/button.h"
#include "../ld2410s.h"

namespace esphome {
namespace ld2410s {

class LD2410SFactoryResetButton : public button::Button, public Parented<LD2410S> {
 public:
  LD2410SFactoryResetButton() = default;

 protected:
  void press_action() override { this->parent_->factory_reset(); }
};

}  // namespace ld2410s
}  // namespace esphome
