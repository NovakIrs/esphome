/*
burst fire ac dimmer
requires zero cross detection
avoids DC bias by alternating if first if first or second halfway is passed
reduces effective window if possible
*/

#pragma once
#include "esphome/core/component.h"
#include "esphome/components/output/binary_output.h"

namespace esphome {
namespace ac_burst_dimmer {

class AcBurstDimmer : public Component, public output::BinaryOutput {
 public:
  void set_zc_pin(InternalGPIOPin *pin) { zc_pin_ = pin; }
  void set_out_pin(output::BinaryOutput *pin) { out_pin_ = pin; }
  void set_window(int window) { default_window_ = window; }
  void set_min_cycles(int minc) { min_cycles_ = minc; }
  void set_pulse_us(uint32_t us) { pulse_us_ = us; }
  void set_full_half(bool full) { pulse_full_half_ = full; }
  void set_alternate_polarity(bool alt) { alternate_polarity_ = alt; }

  void setup() override;
  void loop() override {}
  void write_state(float level) override;

 protected:
  void on_zero_cross_();
  void compute_sequence_();

  InternalGPIOPin *zc_pin_;
  output::BinaryOutput *out_pin_;
  int default_window_{10};
  int window_size_{10};
  int min_cycles_{1};
  int target_on_cycles_{0};
  int cycle_counter_{0};

  uint32_t pulse_us_{100};
  bool pulse_full_half_{false};
  bool alternate_polarity_{true};
  bool current_polarity_{false};
  bool polarity_flip_{false};
  bool last_zc_state_{false};

  std::vector<bool> burst_sequence_;
};

}  // namespace ac_burst_dimmer
}  // namespace esphome