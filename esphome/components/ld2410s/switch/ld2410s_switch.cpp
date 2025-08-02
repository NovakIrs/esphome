#include "ld2410s_switch.h"

namespace esphome {
namespace ld2410s {

void LD2410SMinimalOutputSwitch::write_state(bool state) {
  this->publish_state(state);
  this->parent_->set_minimal_output(state);
  // this->parent_->toggle_minimal();
}

}  // namespace ld2410s
}  // namespace esphome
