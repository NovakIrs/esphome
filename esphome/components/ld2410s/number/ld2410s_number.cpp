#include "ld2410s_number.h"

namespace esphome {
namespace ld2410s {
void LD2410SDelayNumber::control(float delay) {
  this->publish_state(delay);
  this->parent_->set_delay(delay);
}

void LD2410SDistReportingFreqNumber::control(float distance_reporting_freq) {
  this->publish_state(distance_reporting_freq);
  this->parent_->set_distance_reporting_freq(distance_reporting_freq);
}

void LD2410SMaxDistanceNumber::control(float max_distance) {
  this->publish_state(max_distance);
  this->parent_->set_max_distance(max_distance);
}

void LD2410SMinDistanceNumber::control(float min_distance) {
  this->publish_state(min_distance);
  this->parent_->set_min_distance(min_distance);
}

void LD2410SStatusReportingFreqNumber::control(float status_reporting_freq) {
  this->publish_state(status_reporting_freq);
  this->parent_->set_status_reporting_freq(status_reporting_freq);
}

void LD2410SThresholdSelectedGateNumber::control(float threshold_selected_gate) {
  this->publish_state(threshold_selected_gate);
  this->parent_->set_threshold_selected_gate(threshold_selected_gate);
}

void LD2410SThresholdTriggerNumber::control(float threshold_trigger) {
  this->publish_state(threshold_trigger);
  this->parent_->set_threshold_trigger(threshold_trigger);
}

void LD2410SThresholdHoldNumber::control(float threshold_hold) {
  this->publish_state(threshold_hold);
  this->parent_->set_threshold_hold(threshold_hold);
}

void LD2410SThresholdSnrNumber::control(float threshold_snr) {
  this->publish_state(threshold_snr);
  this->parent_->set_threshold_snr(threshold_snr);
}

}  // namespace ld2410s
}  // namespace esphome
