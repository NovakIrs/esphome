#include "ld2410s_button.h"

namespace esphome {
namespace ld2410s {

void LD2410SCalibrationButton::press_action() { this->parent_->calibration(); }

void LD2410SFactoryResetButton::press_action() { this->parent_->factory_reset(); }

}  // namespace ld2410s
}  // namespace esphome
