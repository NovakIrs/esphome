#include "ld2410s_button.h"

namespace esphome {
namespace ld2410s {

void LD2410SReadAllButton::press_action() { this->parent_->read_all(); }

void LD2410SApplyConfigButton::press_action() { this->parent_->write_all(); }

void LD2410SCalibrationButton::press_action() { this->parent_->calibration(); }

void LD2410SFactoryResetButton::press_action() { this->parent_->factory_reset(); }

void LD2410SMinimalOutputButton::press_action() { this->parent_->toggle_minimal(); }

}  // namespace ld2410s
}  // namespace esphome
