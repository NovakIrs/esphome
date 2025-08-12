#include "ld2410s.h"

namespace esphome {
namespace ld2410s {

#ifdef LD2410S_V2

// PUBLIC
// number
void LD2410S::set_delay(float delay) {
  this->settings_.delay = delay;
  this->tx_.schedule_cmd_("set_delay\0", PARAMS_WRITE_CMD, CFG_NO_DELAY_VALUE);
}
void LD2410S::set_distance_reporting_freq(float distance_reporting_freq) {
  this->settings_.dist_freq = distance_reporting_freq * 10;
  this->tx_.schedule_cmd_("set_distance_reporting_freq\0", PARAMS_WRITE_CMD, CFG_DISTANCE_FREQ_VALUE);
}
void LD2410S::set_max_distance(float max_distance) {
  this->settings_.max_dist = static_cast<float>(max_distance) / 0.7f;
  this->tx_.schedule_cmd_("set_max_distance\0", PARAMS_WRITE_CMD, CFG_MAX_DETECTION_VALUE);
}
void LD2410S::set_min_distance(float min_distance) {
  this->settings_.min_dist = static_cast<float>(min_distance) / 0.7f;
  this->tx_.schedule_cmd_("set_min_distance\0", PARAMS_WRITE_CMD, CFG_MIN_DETECTION_VALUE);
}
void LD2410S::set_status_reporting_freq(float status_reporting_freq) {
  this->settings_.status_freq = status_reporting_freq * 10;
  this->tx_.schedule_cmd_("set_status_reporting_freq\0", PARAMS_WRITE_CMD, CFG_STATUS_FREQ_VALUE);
}
void LD2410S::set_threshold_hold(float threshold_hold) {
  this->settings_.thresholds.hold[this->settings_.thresholds.selected_gate] = threshold_hold;
  this->tx_.schedule_cmd_("set_threshold_hold\0", GATE_THRESHOLD_HOLD_WRITE_CMD,
                          this->settings_.thresholds.selected_gate);
  this->publish_threshold_hold_();
}
void LD2410S::set_threshold_selected_gate(float threshold_selected_gate) {
  this->settings_.thresholds.selected_gate = threshold_selected_gate;
#ifdef USE_NUMBER
  this->threshold_selected_gate_number_->publish_state(this->settings_.thresholds.selected_gate);
  this->threshold_trigger_number_->publish_state(
      this->settings_.thresholds.trigger[this->settings_.thresholds.selected_gate]);
  this->threshold_hold_number_->publish_state(
      this->settings_.thresholds.hold[this->settings_.thresholds.selected_gate]);
  this->threshold_snr_number_->publish_state(this->settings_.thresholds.snr[this->settings_.thresholds.selected_gate]);
#endif
}
void LD2410S::set_threshold_snr(float threshold_snr) {
  this->settings_.thresholds.snr[this->settings_.thresholds.selected_gate] = threshold_snr;
  this->tx_.schedule_cmd_("set_threshold_snr\0", GATE_THRESHOLD_SNR_WRITE_CMD,
                          this->settings_.thresholds.selected_gate);
  this->publish_threshold_snr_();
}
void LD2410S::set_threshold_trigger(float threshold_trigger) {
  this->settings_.thresholds.trigger[this->settings_.thresholds.selected_gate] = threshold_trigger;
  this->tx_.schedule_cmd_("set_threshold_trigger\0", GATE_THRESHOLD_TRIGGER_WRITE_CMD,
                          this->settings_.thresholds.selected_gate);
  this->publish_threshold_trigger_();
}
// select
void LD2410S::set_response_speed_select(const std::string &response_speed_select) {
  this->settings_.resp_speed = response_speed_select == RESPONSE_SPEED_NORMAL ? 5 : 10;
  this->tx_.schedule_cmd_("set_response_speed_select\0", PARAMS_WRITE_CMD, CFG_RESPONSE_SPEED_VALUE);
}
// switch
void LD2410S::set_minimal_output(bool state) {
  this->settings_.minimal_output = state;
  if (!state) {
    for (auto &energy_value : this->energy_values_) {
      energy_value = 0;
    }
  }
  this->tx_.schedule_cmd_("set_minimal_output\0", OUTPUT_MODE_SWITCH_CMD);
}

// PROTECTED
void LD2410S::read_all_thresholds_() {
  this->status_set_warning("read_all_thresholds");

  this->tx_.schedule_cmd_frame_(CONFIG_MODE_START_CMD);
  this->tx_.schedule_cmd_frame_(GATE_THRESHOLD_TRIGGER_READ_CMD);
  this->tx_.schedule_cmd_frame_(GATE_THRESHOLD_HOLD_READ_CMD);
  this->tx_.schedule_cmd_frame_(GATE_THRESHOLD_SNR_READ_CMD);
  this->tx_.schedule_cmd_frame_(CONFIG_MODE_END_CMD);

  this->status_clear_warning();
}

void LD2410S::process_ack_config_read_(uint8_t *data) {
  this->settings_.max_dist = esphome::ld2410s::LD2410S::read_int(data, 0, 4);
  this->settings_.min_dist = esphome::ld2410s::LD2410S::read_int(data, 4, 4);
  this->settings_.delay = esphome::ld2410s::LD2410S::read_int(data, 8, 4);
  this->settings_.status_freq = esphome::ld2410s::LD2410S::read_int(data, 12, 4);
  this->settings_.dist_freq = esphome::ld2410s::LD2410S::read_int(data, 16, 4);
  this->settings_.resp_speed = esphome::ld2410s::LD2410S::read_int(data, 20, 4);

#ifdef USE_NUMBER
  this->max_distance_number_->publish_state(static_cast<float>(this->settings_.max_dist) * 0.7);
  this->min_distance_number_->publish_state(static_cast<float>(this->settings_.min_dist) * 0.7);
  this->no_delay_number_->publish_state(this->settings_.delay);
  this->status_reporting_freq_number_->publish_state(static_cast<float>(this->settings_.status_freq) / 10);
  this->distance_reporting_freq_number_->publish_state(static_cast<float>(this->settings_.dist_freq) / 10);
#endif

#ifdef USE_SELECT
  this->response_speed_select_->publish_state(this->settings_.resp_speed == 5 ? RESPONSE_SPEED_NORMAL
                                                                              : RESPONSE_SPEED_FAST);
#endif

  ESP_LOGV(TAG,
           "Config: max_dist=%d, min_dist=%d, delay=%d, status_resp_freq=%d, "
           "dist_resp_freq=%d, resp_speed=%d",
           this->settings_.max_dist, this->settings_.min_dist, this->settings_.delay, this->settings_.status_freq,
           this->settings_.dist_freq, this->settings_.resp_speed);
}
void LD2410S::process_ack_fw_read_(const uint8_t *data) {
  int major_v = esphome::ld2410s::LD2410S::read_int(data, 4, 2);
  int minor_v = esphome::ld2410s::LD2410S::read_int(data, 6, 2);
  int patch_v = esphome::ld2410s::LD2410S::read_int(data, 8, 2);
  std::string version = "v" + std::to_string(major_v) + "." + std::to_string(minor_v) + "." + std::to_string(patch_v);

  this->publish_fw_version_(version);
}
void LD2410S::process_ack_threshold_trigger_read_(uint8_t *data) {
  esphome::ld2410s::LD2410S::four_byte_to_int_array(data, this->settings_.thresholds.trigger, 16);
#ifdef USE_NUMBER
  this->threshold_trigger_number_->publish_state(
      this->settings_.thresholds.trigger[this->settings_.thresholds.selected_gate]);
#endif

  this->publish_threshold_trigger_();
}
void LD2410S::process_ack_threshold_hold_read_(uint8_t *data) {
  esphome::ld2410s::LD2410S::four_byte_to_int_array(data, this->settings_.thresholds.hold, 16);
#ifdef USE_NUMBER
  this->threshold_hold_number_->publish_state(
      this->settings_.thresholds.hold[this->settings_.thresholds.selected_gate]);
#endif

  this->publish_threshold_hold_();
}
void LD2410S::process_ack_threshold_snr_read_(uint8_t *data) {
  esphome::ld2410s::LD2410S::four_byte_to_int_array(data, this->settings_.thresholds.snr, 16);
#ifdef USE_NUMBER
  this->threshold_snr_number_->publish_state(this->settings_.thresholds.snr[this->settings_.thresholds.selected_gate]);
#endif

  this->publish_threshold_snr_();
}
void LD2410S::process_ack_minimal_output_(uint8_t *data) {
#ifdef USE_SWITCH
  this->minimal_output_switch_->publish_state(this->settings_.minimal_output);
#endif

  ESP_LOGW(TAG, "Minimal Output Mode switched");
}

void LD2410S::process_data_energy_values_read_(uint8_t *data) {
  for (uint8_t i = 0; i < 16; i++) {
    uint32_t val = encode_uint32(data[i * 4 + 3], data[i * 4 + 2], data[i * 4 + 1], data[i * 4 + 0]);
    uint32_t db = 0;
    if (val > 0) {
      db = 10 * log10(val);
    }
    if (db > this->energy_values_[i]) {
      this->energy_values_[i] = db;
    }
  }
  this->publish_energy_values_();
}

void LD2410S::publish_fw_version_(const std::string &version, bool force_publish) {
#ifdef USE_TEXT_SENSOR
  if (this->fw_version_text_sensor_ != nullptr) {
    if (this->fw_version_text_sensor_->state != version || force_publish) {
      this->fw_version_text_sensor_->publish_state(version);
    }
  }
#endif
  ESP_LOGI(TAG, "Firmware version: %s", version.c_str());
}

void LD2410S::publish_threshold_trigger_(bool force_publish) {
  std::string vals = esphome::ld2410s::LD2410S::format_int(this->settings_.thresholds.trigger, 16, 2);

#ifdef USE_TEXT_SENSOR
  if (this->threshold_trigger_text_sensor_ != nullptr) {
    if (this->threshold_trigger_text_sensor_->state != vals || force_publish) {
      this->threshold_trigger_text_sensor_->publish_state(vals);
    }
  }
#endif
  ESP_LOGI(TAG, "Gate Trigger Thresholds: %s", vals.c_str());
}
void LD2410S::publish_threshold_hold_(bool force_publish) {
  std::string vals = esphome::ld2410s::LD2410S::format_int(this->settings_.thresholds.hold, 16, 2);

#ifdef USE_TEXT_SENSOR
  if (this->threshold_hold_text_sensor_ != nullptr) {
    if (this->threshold_hold_text_sensor_->state != vals || force_publish) {
      this->threshold_hold_text_sensor_->publish_state(vals);
    }
  }
#endif
  ESP_LOGI(TAG, "Gate Trigger Holds: %s", vals.c_str());
}
void LD2410S::publish_threshold_snr_(bool force_publish) {
  std::string vals = esphome::ld2410s::LD2410S::format_int(this->settings_.thresholds.snr, 16, 2);

#ifdef USE_TEXT_SENSOR
  if (this->threshold_snr_text_sensor_ != nullptr) {
    if (this->threshold_snr_text_sensor_->state != vals || force_publish) {
      this->threshold_snr_text_sensor_->publish_state(vals);
    }
  }
#endif
  ESP_LOGI(TAG, "Gate Trigger SNR: %s", vals.c_str());
}

void LD2410S::publish_energy_values_(bool force_publish) {
  this->energy_values_str_ = esphome::ld2410s::LD2410S::format_int(this->energy_values_, 16, 2);

#ifdef USE_TEXT_SENSOR
  if (this->energy_values_text_sensor_ != nullptr) {
    if (this->energy_values_text_sensor_->state != this->energy_values_str_ || force_publish) {
      this->energy_values_text_sensor_->publish_state(this->energy_values_str_);
    }
  }
#endif
  ESP_LOGD(TAG, "Energy Values: %s", this->energy_values_str_.c_str());
}

std::string LD2410S::format_int(uint32_t *in, uint8_t len, uint8_t min_w) {
  if (len == 0)
    return "";

  std::string result;
  int sum = 0;
  for (uint8_t i = 0; i < len; ++i) {
    sum += in[i];

    if (i > 0)
      result += ',';

    std::string num = std::to_string(in[i]);

    if (num.length() < min_w)
      result += std::string(min_w - num.length(), '0');

    result += num;
  }

  if (sum == 0) {
    result = "";
  }

  return result;
}

#endif

}  // namespace ld2410s
}  // namespace esphome
