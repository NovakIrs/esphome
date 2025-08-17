#include "ld2410s.h"

namespace esphome {
namespace ld2410s {

#ifdef LD2410S_V2

// PUBLIC

void LD2410S::dump_config() {
#ifdef USE_BUTTON
  ESP_LOGCONFIG(TAG, "Buttons:");
  LOG_BUTTON("  ", "Factory reset", this->factory_reset_button_);
  LOG_BUTTON("  ", "Start calibration", this->calibration_button_);
#endif

#ifdef USE_SWITCH
  ESP_LOGCONFIG(TAG, "Switches:");
  LOG_SWITCH("  ", "Minimal Output", this->minimal_output_switch_);
#endif
}

void LD2410S::init_() {
  ESP_LOGI(TAG, "init");
  // App.feed_wdt();

  this->init_done_ = false;

  this->minimal_output_ = true;

  this->read_all_();
}
void LD2410S::read_all_() {
  this->schedule_cmd_frame_(CONFIG_MODE_START_CMD);

  this->schedule_cmd_frame_(OUTPUT_MODE_SWITCH_CMD);
  this->schedule_cmd_frame_(FW_READ_CMD);
  this->schedule_cmd_frame_(PARAMS_READ_CMD);
  this->schedule_cmd_frame_(GATE_THRESHOLD_TRIGGER_READ_CMD);
  this->schedule_cmd_frame_(GATE_THRESHOLD_HOLD_READ_CMD);
  this->schedule_cmd_frame_(GATE_THRESHOLD_SNR_READ_CMD);

  this->schedule_cmd_frame_(CONFIG_MODE_END_CMD);
}

// button
void LD2410S::calibration() { this->schedule_cmd_frames_sequence_("calibration\0", CALIBRATION_CMD); }
void LD2410S::factory_reset() {
  ESP_LOGI(TAG, "factory_reset");

  this->max_dist_ = 16;
  this->min_dist_ = 0;
  this->delay_ = 10;
  this->status_freq_ = 80;
  this->dist_freq_ = 80;

  this->minimal_output_ = true;

  this->resp_speed_ = 5;

  for (uint8_t i = 0; i < 16; i++) {
    this->thresholds_trigger_[i] = GATE_THRESHOLD_TRIGGER_WRITE_DATA[i];
    this->thresholds_hold_[i] = GATE_THRESHOLD_HOLD_WRITE_DATA[i];
    this->thresholds_snr_[i] = GATE_THRESHOLD_SNR_WRITE_DATA[i];
  }

  this->schedule_cmd_frame_(CONFIG_MODE_START_CMD);

  this->schedule_cmd_frame_(OUTPUT_MODE_SWITCH_CMD);

  this->schedule_cmd_frame_(PARAMS_WRITE_CMD);
  this->schedule_cmd_frame_(GATE_THRESHOLD_TRIGGER_WRITE_CMD);
  this->schedule_cmd_frame_(GATE_THRESHOLD_HOLD_WRITE_CMD);
  this->schedule_cmd_frame_(GATE_THRESHOLD_SNR_WRITE_CMD);

  this->schedule_cmd_frame_(PARAMS_READ_CMD);
  this->schedule_cmd_frame_(GATE_THRESHOLD_TRIGGER_READ_CMD);
  this->schedule_cmd_frame_(GATE_THRESHOLD_HOLD_READ_CMD);
  this->schedule_cmd_frame_(GATE_THRESHOLD_SNR_READ_CMD);

  this->schedule_cmd_frame_(CONFIG_MODE_END_CMD);
}
// number
void LD2410S::set_delay(float delay) {
  this->delay_ = delay;
  this->schedule_cmd_frames_sequence_("set_delay\0", PARAMS_WRITE_CMD, CFG_NO_DELAY_VALUE);
  this->no_delay_number_->publish_state(this->delay_);
}
void LD2410S::set_distance_reporting_freq(float distance_reporting_freq) {
  this->dist_freq_ = distance_reporting_freq * 10;
  this->schedule_cmd_frames_sequence_("set_distance_reporting_freq\0", PARAMS_WRITE_CMD, CFG_DISTANCE_FREQ_VALUE);
  this->distance_reporting_freq_number_->publish_state(static_cast<float>(this->dist_freq_) / 10);
}
void LD2410S::set_max_distance(float max_distance) {
  this->max_dist_ = static_cast<float>(max_distance) / 0.7f;
  this->schedule_cmd_frames_sequence_("set_max_distance\0", PARAMS_WRITE_CMD, CFG_MAX_DETECTION_VALUE);
  this->max_distance_number_->publish_state(static_cast<float>(this->max_dist_) * 0.7);
}
void LD2410S::set_min_distance(float min_distance) {
  this->min_dist_ = static_cast<float>(min_distance) / 0.7f;
  this->schedule_cmd_frames_sequence_("set_min_distance\0", PARAMS_WRITE_CMD, CFG_MIN_DETECTION_VALUE);
  this->min_distance_number_->publish_state(static_cast<float>(this->min_dist_) * 0.7);
}
void LD2410S::set_status_reporting_freq(float status_reporting_freq) {
  this->status_freq_ = status_reporting_freq * 10;
  this->schedule_cmd_frames_sequence_("set_status_reporting_freq\0", PARAMS_WRITE_CMD, CFG_STATUS_FREQ_VALUE);
  this->status_reporting_freq_number_->publish_state(static_cast<float>(this->status_freq_) / 10);
}
void LD2410S::set_threshold_hold(float threshold_hold) {
  this->thresholds_hold_[this->thresholds_selected_gate_] = threshold_hold;
  this->schedule_cmd_frames_sequence_("set_threshold_hold\0", GATE_THRESHOLD_HOLD_WRITE_CMD,
                                      this->thresholds_selected_gate_);
  this->threshold_hold_number_->publish_state(this->thresholds_hold_[this->thresholds_selected_gate_]);
  this->publish_threshold_hold_();
}
void LD2410S::set_threshold_selected_gate(float threshold_selected_gate) {
  this->thresholds_selected_gate_ = threshold_selected_gate;
#ifdef USE_NUMBER
  this->threshold_selected_gate_number_->publish_state(this->thresholds_selected_gate_);
  this->threshold_trigger_number_->publish_state(this->thresholds_trigger_[this->thresholds_selected_gate_]);
  this->threshold_hold_number_->publish_state(this->thresholds_hold_[this->thresholds_selected_gate_]);
  this->threshold_snr_number_->publish_state(this->thresholds_snr_[this->thresholds_selected_gate_]);
#endif
}
void LD2410S::set_threshold_snr(float threshold_snr) {
  this->thresholds_snr_[this->thresholds_selected_gate_] = threshold_snr;
  this->schedule_cmd_frames_sequence_("set_threshold_snr\0", GATE_THRESHOLD_SNR_WRITE_CMD,
                                      this->thresholds_selected_gate_);
  this->threshold_snr_number_->publish_state(this->thresholds_snr_[this->thresholds_selected_gate_]);
  this->publish_threshold_snr_();
}
void LD2410S::set_threshold_trigger(float threshold_trigger) {
  this->thresholds_trigger_[this->thresholds_selected_gate_] = threshold_trigger;
  this->schedule_cmd_frames_sequence_("set_threshold_trigger\0", GATE_THRESHOLD_TRIGGER_WRITE_CMD,
                                      this->thresholds_selected_gate_);
  this->threshold_trigger_number_->publish_state(this->thresholds_trigger_[this->thresholds_selected_gate_]);
  this->publish_threshold_trigger_();
}
// select
void LD2410S::set_response_speed_select(const std::string &response_speed_select) {
  this->resp_speed_ = response_speed_select == RESPONSE_SPEED_NORMAL ? 5 : 10;
  this->schedule_cmd_frames_sequence_("set_response_speed_select\0", PARAMS_WRITE_CMD, CFG_RESPONSE_SPEED_VALUE);
#ifdef USE_SELECT
  this->response_speed_select_->publish_state(this->resp_speed_ == 5 ? RESPONSE_SPEED_NORMAL : RESPONSE_SPEED_FAST);
#endif
}
// switch
void LD2410S::set_minimal_output(bool state) {
  this->minimal_output_ = state;
  if (!state) {
    for (auto &energy_value : this->energy_values_) {
      energy_value = 0;
    }
  }
  this->schedule_cmd_frames_sequence_("set_minimal_output\0", OUTPUT_MODE_SWITCH_CMD);
}

// PROTECTED
void LD2410S::read_all_thresholds_() {
  this->status_set_warning("read_all_thresholds");

  this->schedule_cmd_frame_(CONFIG_MODE_START_CMD);
  this->schedule_cmd_frame_(GATE_THRESHOLD_TRIGGER_READ_CMD);
  this->schedule_cmd_frame_(GATE_THRESHOLD_HOLD_READ_CMD);
  this->schedule_cmd_frame_(GATE_THRESHOLD_SNR_READ_CMD);
  this->schedule_cmd_frame_(CONFIG_MODE_END_CMD);

  this->status_clear_warning();
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

void LD2410S::process_ack_config_read_(uint8_t *data) {
  ESP_LOGD(TAG, "process_ack_config_read_");

  this->max_dist_ = esphome::ld2410s::LD2410S::read_int(data, 0, 4);
  this->min_dist_ = esphome::ld2410s::LD2410S::read_int(data, 4, 4);
  this->delay_ = esphome::ld2410s::LD2410S::read_int(data, 8, 4);
  this->status_freq_ = esphome::ld2410s::LD2410S::read_int(data, 12, 4);
  this->dist_freq_ = esphome::ld2410s::LD2410S::read_int(data, 16, 4);
  this->resp_speed_ = esphome::ld2410s::LD2410S::read_int(data, 20, 4);

#ifdef USE_NUMBER
  this->max_distance_number_->publish_state(static_cast<float>(this->max_dist_) * 0.7);
  this->min_distance_number_->publish_state(static_cast<float>(this->min_dist_) * 0.7);
  this->no_delay_number_->publish_state(this->delay_);
  this->status_reporting_freq_number_->publish_state(static_cast<float>(this->status_freq_) / 10);
  this->distance_reporting_freq_number_->publish_state(static_cast<float>(this->dist_freq_) / 10);
#endif

#ifdef USE_SELECT
  this->response_speed_select_->publish_state(this->resp_speed_ == 5 ? RESPONSE_SPEED_NORMAL : RESPONSE_SPEED_FAST);
#endif

  ESP_LOGV(TAG,
           "Config: max_dist=%d, min_dist=%d, delay=%d, status_resp_freq=%d, "
           "dist_resp_freq=%d, resp_speed=%d",
           this->max_dist_, this->min_dist_, this->delay_, this->status_freq_, this->dist_freq_, this->resp_speed_);
}
void LD2410S::process_ack_fw_read_(const uint8_t *data) {
  int major_v = esphome::ld2410s::LD2410S::read_int(data, 4, 2);
  int minor_v = esphome::ld2410s::LD2410S::read_int(data, 6, 2);
  int patch_v = esphome::ld2410s::LD2410S::read_int(data, 8, 2);
  std::string version = "v" + std::to_string(major_v) + "." + std::to_string(minor_v) + "." + std::to_string(patch_v);

  this->publish_fw_version_(version);
}
void LD2410S::process_ack_threshold_trigger_read_(uint8_t *data) {
  esphome::ld2410s::LD2410S::four_byte_to_int_array(data, this->thresholds_trigger_, 16);
#ifdef USE_NUMBER
  this->threshold_trigger_number_->publish_state(this->thresholds_trigger_[this->thresholds_selected_gate_]);
#endif

  this->publish_threshold_trigger_();
}
void LD2410S::process_ack_threshold_hold_read_(uint8_t *data) {
  esphome::ld2410s::LD2410S::four_byte_to_int_array(data, this->thresholds_hold_, 16);
#ifdef USE_NUMBER
  this->threshold_hold_number_->publish_state(this->thresholds_hold_[this->thresholds_selected_gate_]);
#endif

  this->publish_threshold_hold_();
}
void LD2410S::process_ack_threshold_snr_read_(uint8_t *data) {
  esphome::ld2410s::LD2410S::four_byte_to_int_array(data, this->thresholds_snr_, 16);
#ifdef USE_NUMBER
  this->threshold_snr_number_->publish_state(this->thresholds_snr_[this->thresholds_selected_gate_]);
#endif

  this->publish_threshold_snr_();
}
void LD2410S::process_ack_minimal_output_(uint8_t *data) {
#ifdef USE_SWITCH
  this->minimal_output_switch_->publish_state(this->minimal_output_);
#endif

  ESP_LOGW(TAG, "Minimal Output Mode switched");
}

void LD2410S::publish_calibration_progress_(uint16_t calibration_progress, bool force_publish) {
#ifdef USE_SENSOR
  if (this->calibration_progress_sensor_ != nullptr) {
    if (calibration_progress == 100) {
      if (this->calibration_progress_sensor_->state != 0 || force_publish) {
        this->calibration_progress_sensor_->publish_state(0);
      }
    } else {
      if (this->calibration_progress_sensor_->state != calibration_progress || force_publish) {
        this->calibration_progress_sensor_->publish_state(calibration_progress);
      }
    }
  }
#endif
}
void LD2410S::publish_calibration_runing_(bool running, bool force_publish) {
#ifdef USE_BINARY_SENSOR
  if (this->calibration_runing_binary_sensor_ != nullptr) {
    if (this->calibration_runing_binary_sensor_->state != running || force_publish) {
      this->calibration_runing_binary_sensor_->publish_state(running);
    }
  }
#endif
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
  std::string vals = esphome::ld2410s::LD2410S::format_int(this->thresholds_trigger_, 16, 2);

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
  std::string vals = esphome::ld2410s::LD2410S::format_int(this->thresholds_hold_, 16, 2);

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
  std::string vals = esphome::ld2410s::LD2410S::format_int(this->thresholds_snr_, 16, 2);

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
