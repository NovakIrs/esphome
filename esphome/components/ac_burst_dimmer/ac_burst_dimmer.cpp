#include "ac_burst_dimmer.h"
#include <algorithm>

namespace esphome {
namespace ac_burst_dimmer {

void AcBurstDimmer::setup() {
  zc_pin_->pin_mode_input();
  zc_pin_->set_parent(this);
  zc_pin_->set_on_state_callback([this](bool state) {
    if (state && !last_zc_state_)
      this->on_zero_cross_();
    last_zc_state_ = state;
  });

  out_pin_->write(false);
}

void AcBurstDimmer::write_state(float level) {
  int pct = (int) round(level * 1000);
  int g = std::gcd(pct, 1000);
  window_size_ = default_window_ * (1000 / g);
  target_on_cycles_ = std::max(min_cycles_, std::min(pct / g, window_size_));
  cycle_counter_ = 0;
  polarity_flip_ = false;
}

void AcBurstDimmer::on_zero_cross_() {
  current_polarity_ = !current_polarity_;
  cycle_counter_++;
  if (cycle_counter_ >= window_size_) {
    cycle_counter_ = 0;
    if (alternate_polarity_)
      polarity_flip_ = !polarity_flip_;
  }

  int index = cycle_counter_;
  if (alternate_polarity_ && (window_size_ % 2 == 1))
    index = (cycle_counter_ + (polarity_flip_ ? 1 : 0)) % window_size_;

  if (index < target_on_cycles_) {
    out_pin_->write(true);
    if (!pulse_full_half_) {
      this->set_timeout("pulse_end", pulse_us_ / 1000, [this]() { out_pin_->write(false); });
    }
  } else if (pulse_full_half_) {
    out_pin_->write(false);
  }
}

}  // namespace ac_burst_dimmer
}  // namespace esphome