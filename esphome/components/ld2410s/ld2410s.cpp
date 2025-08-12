#include "ld2410s.h"

namespace esphome {
namespace ld2410s {

void LD2410S::setup() {
  ESP_LOGD(TAG, "setup");
  // this->tx_.set_settings(this->settings_);
  this->init_();

  this->publish_distance_(0, true);
  this->publish_presence_(false, true);

  this->publish_calibration_progress_(0, true);
  this->publish_calibration_runing_(false, true);

#ifdef LD2410S_V2
  this->set_threshold_selected_gate(0);
#endif
}
void LD2410S::loop() {
  // ESP_LOGD(TAG, "loop");
  // if (this->rx_.receive_()) {
  //   this->process_();
  //   } else {
  this->send_();
  // }
}

void LD2410S::dump_config() {
  // #ifdef USE_BUTTON
  //   ESP_LOGCONFIG(TAG, "Buttons:");
  //   LOG_BUTTON("  ", "Factory reset", this->factory_reset_button_);
  //   LOG_BUTTON("  ", "Start calibration", this->calibration_button_);
  // #endif

  // #ifdef USE_SWITCH
  //   ESP_LOGCONFIG(TAG, "Switches:");
  //   LOG_SWITCH("  ", "Minimal Output", this->minimal_output_switch_);
  // #endif
}
float LD2410S::get_setup_priority() const { return setup_priority::HARDWARE; }

void LD2410S::calibration() { this->tx_.schedule_cmd_("calibration\0", CALIBRATION_CMD); }
void LD2410S::factory_reset() {
  this->status_set_warning("factory_reset");

  this->settings_.max_dist = 16;
  this->settings_.min_dist = 0;
  this->settings_.delay = 10;
  this->settings_.status_freq = 80;
  this->settings_.dist_freq = 80;

  this->settings_.minimal_output = true;

  this->settings_.resp_speed = 5;

  for (uint8_t i = 0; i < 16; i++) {
    this->settings_.thresholds.trigger[i] = GATE_THRESHOLD_TRIGGER_WRITE_DATA[i];
    this->settings_.thresholds.hold[i] = GATE_THRESHOLD_HOLD_WRITE_DATA[i];
    this->settings_.thresholds.snr[i] = GATE_THRESHOLD_SNR_WRITE_DATA[i];
  }

  this->tx_.schedule_cmd_frame_(CONFIG_MODE_START_CMD);

  this->tx_.schedule_cmd_frame_(OUTPUT_MODE_SWITCH_CMD);

  this->tx_.schedule_cmd_frame_(PARAMS_WRITE_CMD);
  this->tx_.schedule_cmd_frame_(GATE_THRESHOLD_TRIGGER_WRITE_CMD);
  this->tx_.schedule_cmd_frame_(GATE_THRESHOLD_HOLD_WRITE_CMD);
  this->tx_.schedule_cmd_frame_(GATE_THRESHOLD_SNR_WRITE_CMD);

  this->tx_.schedule_cmd_frame_(PARAMS_READ_CMD);
  this->tx_.schedule_cmd_frame_(GATE_THRESHOLD_TRIGGER_READ_CMD);
  this->tx_.schedule_cmd_frame_(GATE_THRESHOLD_HOLD_READ_CMD);
  this->tx_.schedule_cmd_frame_(GATE_THRESHOLD_SNR_READ_CMD);

  this->tx_.schedule_cmd_frame_(CONFIG_MODE_END_CMD);
  this->status_clear_warning();
}

// PROTECTED

void LD2410S::init_() {
  ESP_LOGD(TAG, "init");
  // App.feed_wdt();

  this->settings_.minimal_output = true;

  this->init_status_ = 0;

  this->tx_.schedule_cmd_frame_(CONFIG_MODE_START_CMD);
  this->tx_.schedule_cmd_frame_(OUTPUT_MODE_SWITCH_CMD);
  this->tx_.schedule_cmd_frame_(FW_READ_CMD);
  this->tx_.schedule_cmd_frame_(PARAMS_READ_CMD);
  this->tx_.schedule_cmd_frame_(GATE_THRESHOLD_TRIGGER_READ_CMD);
  this->tx_.schedule_cmd_frame_(GATE_THRESHOLD_HOLD_READ_CMD);
  this->tx_.schedule_cmd_frame_(GATE_THRESHOLD_SNR_READ_CMD);
  this->tx_.schedule_cmd_frame_(CONFIG_MODE_END_CMD);
}

void LD2410S::send_() {
  if (this->tx_.get_schedule_empty() && this->init_status_ != 0b11111111) {
    ESP_LOGE(TAG, "Setup failed! Retry...  %x", this->init_status_);
    this->init_();
  } else {
    if (this->tx_.loop_send_command_()) {
      for (uint16_t index = 0; index < this->tx_.data_length; index++) {
        this->write_byte(this->tx_.tx_buffer[index]);
      }

      this->flush();

      this->hex_diag(">", this->tx_.tx_buffer, this->tx_.data_length);
    }
  }
}

void LD2410S::process_() {
  uint8_t *data = &this->rx_.payload_data()[0];
  switch (this->rx_.frame_type()) {
    case RxFrameType::SHORT_DATA_FRAME:
      this->process_short_data_frame_();
      break;

    case RxFrameType::STD_DATA_FRAME:
      this->process_data_frame_();
      break;

    case RxFrameType::CMD_FRAME:
      this->process_cmd_frame_();
      this->tx_.cmd_buffer_finished_();
      break;

    default:
      ESP_LOGE(TAG, "Received Unknown package type!!!");
      break;
  }
}

void LD2410S::process_short_data_frame_() {
  const bool presence_state = this->rx_.payload_data()[0] > 1;
  uint16_t distance = encode_uint16(this->rx_.payload_data()[2], this->rx_.payload_data()[1]);

  if (!presence_state)
    distance = 0;

  this->publish_distance_(distance);
  this->publish_presence_(presence_state);
}
void LD2410S::process_data_frame_() {
  // uint8_t *data, size_t data_size

  switch (this->rx_.payload_data()[0]) {
    case 0x01:  // standard data
    {
      const bool presence_state = this->rx_.payload_data()[1] > 1;

      uint16_t distance = encode_uint16(this->rx_.payload_data()[3], this->rx_.payload_data()[2]);
      if (!presence_state)
        distance = 0;

      this->publish_distance_(distance);
      this->publish_presence_(presence_state);

#ifdef LD2410S_V2
      this->process_data_energy_values_read_(&this->rx_.payload_data()[6]);
#endif

      break;
    }

    case 0x03:  // calibration progress
    {
      uint16_t progress = encode_uint16(this->rx_.payload_data()[2], this->rx_.payload_data()[1]);

      if (progress == 100) {
        this->publish_calibration_runing_(false);

#ifdef LD2410S_V2
        this->read_all_thresholds_();
#endif

      } else {
        this->publish_calibration_runing_(true);
      }

      this->publish_calibration_progress_(progress);

      break;
    }

    default:
      break;
  }
}
void LD2410S::process_cmd_frame_() {
  int command_word = encode_uint16(this->rx_.payload_data()[1], this->rx_.payload_data()[0]);
  bool ack = encode_uint16(this->rx_.payload_data()[3], this->rx_.payload_data()[2]);
  if (!ack) {
    ESP_LOGW(TAG, "Command %x failed", command_word);
  }

  uint8_t *data = &this->rx_.payload_data()[4];

  switch (command_word) {
    case PARAMS_READ_REPLY:
      this->process_ack_config_read_(data);
      this->init_status_ = this->init_status_ | 0b00001000;
      break;

    case FW_READ_REPLY:

#ifdef LD2410S_V2
      this->process_ack_fw_read_(data);
#endif
      this->init_status_ = this->init_status_ | 0b00000100;
      break;

    case GATE_THRESHOLD_TRIGGER_READ_REPLY:

#ifdef LD2410S_V2
      this->process_ack_threshold_trigger_read_(data);
#endif
      this->init_status_ = this->init_status_ | 0b00010000;
      break;

    case GATE_THRESHOLD_HOLD_READ_REPLY:

#ifdef LD2410S_V2
      this->process_ack_threshold_hold_read_(data);
#endif
      this->init_status_ = this->init_status_ | 0b00100000;
      break;

    case GATE_THRESHOLD_SNR_READ_REPLY:

#ifdef LD2410S_V2
      this->process_ack_threshold_snr_read_(data);
#endif
      this->init_status_ = this->init_status_ | 0b01000000;
      break;

    case CONFIG_MODE_START_REPLY:
      this->init_status_ = this->init_status_ | 0b00000001;
      ESP_LOGD(TAG, "Config mode enabled");
      break;

    case CONFIG_MODE_END_REPLY:
      this->init_status_ = this->init_status_ | 0b10000000;
      ESP_LOGD(TAG, "Config mode disabled");
      break;

    case PARAMS_WRITE_REPLY:
      ESP_LOGD(TAG, "Config written");
      break;

    case GATE_THRESHOLD_TRIGGER_WRITE_REPLY:
      ESP_LOGD(TAG, "Trigger Threshold written");
      break;

    case GATE_THRESHOLD_HOLD_WRITE_REPLY:
      ESP_LOGD(TAG, "Trigger Hold written");
      break;

    case GATE_THRESHOLD_SNR_WRITE_REPLY:
      ESP_LOGD(TAG, "Trigger SNR written");
      break;

    case OUTPUT_MODE_SWITCH_REPLY:

#ifdef LD2410S_V2
      this->process_ack_minimal_output_(data);
#endif
      this->init_status_ = this->init_status_ | 0b00000010;
      break;

    default:
      ESP_LOGW(TAG, "< Unknown: %4x", command_word);
      break;
  }
}

void LD2410S::publish_distance_(uint16_t distance, bool force_publish) {
#ifdef USE_SENSOR
  if (this->distance_sensor_ != nullptr) {
    if (this->distance_sensor_->state != distance || force_publish) {
      this->distance_sensor_->publish_state(distance);
    }
  }
#endif
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
void LD2410S::publish_presence_(bool presence, bool force_publish) {
#ifdef USE_BINARY_SENSOR
  if (this->presence_binary_sensor_ != nullptr) {
    if (this->presence_binary_sensor_->state != presence || force_publish) {
      this->presence_binary_sensor_->publish_state(presence);
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

}  // namespace ld2410s
}  // namespace esphome
