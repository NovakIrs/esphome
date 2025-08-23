#include "ld2410s.h"

namespace esphome {
namespace ld2410s {

// PUBLIC

void LD2410S::setup() {
  ESP_LOGD(TAG, "setup");

  this->publish_distance_(0, true);
  this->publish_presence_(false, true);

#ifdef LD2410S_V2
  this->publish_calibration_progress_(0, true);
  this->publish_calibration_runing_(false, true);

  this->set_threshold_selected_gate(0);

  this->init_();
#endif
}
void LD2410S::loop() {
  if (!this->receive_()) {
#ifdef LD2410S_ENABLE_UART
    this->send_();
#endif
  }
  this->loop_count_++;
}

float LD2410S::get_setup_priority() const { return setup_priority::HARDWARE; }

// PROTECTED

// prepares scheduled frames for sending
// executes actual data sending
void LD2410S::send_() {
  switch (this->tx_schedule_.check_state()) {
    case TxCmdState::SCHEDULED:
      this->build_cmd_frame_(this->tx_schedule_.get_command(), this->tx_schedule_.get_sub_command());

    case TxCmdState::SEND:
      this->write_array(this->tx_frame_, sizeof(this->tx_frame_));
      // for (uint16_t index = 0; index < this->tx_frame_size_; index++) {
      //   this->write_byte(this->tx_frame_[index]);
      // }
      // this->flush();

      ESP_LOGI(TAG, "> %04x:%04x, loop:%d > %d", this->tx_schedule_.get_command(), this->tx_schedule_.get_sub_command(),
               this->loop_count_, format_hex_pretty(this->tx_frame_, this->tx_frame_size_, ' '));
      ESP_LOGD(TAG, "Sending, loop:%d", this->loop_count_);
      hex_diag(">", this->tx_frame_, this->tx_frame_size_);

      this->tx_schedule_.confirm_sent();
      break;

    case TxCmdState::ERROR:
      ESP_LOGW(TAG, "Scheduling command send failed!!!, re-initializing...");
      this->tx_schedule_.reset();
#ifdef LD2410S_V2
      this->init_();
#endif
      break;

    case TxCmdState::EMPTY:
      if (!this->init_done_) {
        ESP_LOGI(TAG, "Setup done");
        this->init_done_ = true;
      }
      break;

    case TxCmdState::SENT:
    default:
      break;
  }
}
// builds CMD_FRAME
void LD2410S::build_cmd_frame_(uint16_t command, uint16_t sub_command) {
#ifdef LD2410S_DEBUG_UART
  ESP_LOGD(TAG, "build_cmd_frame %04x : %04x", command, sub_command);
#endif

  this->tx_frame_size_ = 0;

  // Header
  append_seq_data(this->tx_frame_, this->tx_frame_size_, &CMD_FRAME_HEADER);

  // Frame size placeholder
  uint16_t size_start = this->tx_frame_size_;
  this->tx_frame_size_ += sizeof(size_start);

  // Data start
  uint16_t data_start = this->tx_frame_size_;

  // Command
  append_seq_data(this->tx_frame_, this->tx_frame_size_, &command, 1);

  // Parameters
  switch (command) {
    case OUTPUT_MODE_SWITCH_CMD: {
      if (this->minimal_output_) {
        append_seq_data(this->tx_frame_, this->tx_frame_size_, OUTPUT_MODE_VALUE_MIN, 6);
      } else {
        append_seq_data(this->tx_frame_, this->tx_frame_size_, OUTPUT_MODE_VALUE_STD, 6);
      }
    } break;

    case CONFIG_MODE_START_CMD:
      append_seq_data(this->tx_frame_, this->tx_frame_size_, &CONFIG_MODE_START_VALUE);
      break;

    case CONFIG_MODE_END_CMD:
      break;

    case PARAMS_READ_CMD:

      switch (sub_command) {
        case CFG_MAX_DETECTION_VALUE:
          append_seq_data(this->tx_frame_, this->tx_frame_size_, &CFG_MAX_DETECTION_VALUE);
          break;

        case CFG_MIN_DETECTION_VALUE:
          append_seq_data(this->tx_frame_, this->tx_frame_size_, &CFG_MIN_DETECTION_VALUE);
          break;

        case CFG_NO_DELAY_VALUE:
          append_seq_data(this->tx_frame_, this->tx_frame_size_, &CFG_NO_DELAY_VALUE);
          break;

        case CFG_STATUS_FREQ_VALUE:
          append_seq_data(this->tx_frame_, this->tx_frame_size_, &CFG_STATUS_FREQ_VALUE);
          break;

        case CFG_DISTANCE_FREQ_VALUE:
          append_seq_data(this->tx_frame_, this->tx_frame_size_, &CFG_DISTANCE_FREQ_VALUE);
          break;

        case CFG_RESPONSE_SPEED_VALUE:
          append_seq_data(this->tx_frame_, this->tx_frame_size_, &CFG_RESPONSE_SPEED_VALUE);
          break;

        default:
          append_seq_data(this->tx_frame_, this->tx_frame_size_, &CFG_MAX_DETECTION_VALUE);
          append_seq_data(this->tx_frame_, this->tx_frame_size_, &CFG_MIN_DETECTION_VALUE);
          append_seq_data(this->tx_frame_, this->tx_frame_size_, &CFG_NO_DELAY_VALUE);
          append_seq_data(this->tx_frame_, this->tx_frame_size_, &CFG_STATUS_FREQ_VALUE);
          append_seq_data(this->tx_frame_, this->tx_frame_size_, &CFG_DISTANCE_FREQ_VALUE);
          append_seq_data(this->tx_frame_, this->tx_frame_size_, &CFG_RESPONSE_SPEED_VALUE);
          break;
      }

      break;

    case FW_READ_CMD:
      break;

    case PARAMS_WRITE_CMD:
      if (this->resp_speed_ == 0) {
        ESP_LOGD(TAG, "PARAMS_WRITE_CMD Error, bad new_config");
        return;
      } else {
        switch (sub_command) {
          case CFG_MAX_DETECTION_VALUE:
            append_seq_data(this->tx_frame_, this->tx_frame_size_, &CFG_MAX_DETECTION_VALUE);
            append_seq_data(this->tx_frame_, this->tx_frame_size_, &this->max_dist_);
            break;

          case CFG_MIN_DETECTION_VALUE:
            append_seq_data(this->tx_frame_, this->tx_frame_size_, &CFG_MIN_DETECTION_VALUE);
            append_seq_data(this->tx_frame_, this->tx_frame_size_, &this->min_dist_);
            break;

          case CFG_NO_DELAY_VALUE:
            append_seq_data(this->tx_frame_, this->tx_frame_size_, &CFG_NO_DELAY_VALUE);
            append_seq_data(this->tx_frame_, this->tx_frame_size_, &this->delay_);
            break;

          case CFG_STATUS_FREQ_VALUE:
            append_seq_data(this->tx_frame_, this->tx_frame_size_, &CFG_STATUS_FREQ_VALUE);
            append_seq_data(this->tx_frame_, this->tx_frame_size_, &this->status_freq_);
            break;

          case CFG_DISTANCE_FREQ_VALUE:
            append_seq_data(this->tx_frame_, this->tx_frame_size_, &CFG_DISTANCE_FREQ_VALUE);
            append_seq_data(this->tx_frame_, this->tx_frame_size_, &this->dist_freq_);
            break;

          case CFG_RESPONSE_SPEED_VALUE:
            append_seq_data(this->tx_frame_, this->tx_frame_size_, &CFG_RESPONSE_SPEED_VALUE);
            append_seq_data(this->tx_frame_, this->tx_frame_size_, &this->resp_speed_);
            break;

          default:

            append_seq_data(this->tx_frame_, this->tx_frame_size_, &CFG_MAX_DETECTION_VALUE);
            append_seq_data(this->tx_frame_, this->tx_frame_size_, &this->max_dist_);

            append_seq_data(this->tx_frame_, this->tx_frame_size_, &CFG_MIN_DETECTION_VALUE);
            append_seq_data(this->tx_frame_, this->tx_frame_size_, &this->min_dist_);

            append_seq_data(this->tx_frame_, this->tx_frame_size_, &CFG_NO_DELAY_VALUE);
            append_seq_data(this->tx_frame_, this->tx_frame_size_, &this->delay_);

            append_seq_data(this->tx_frame_, this->tx_frame_size_, &CFG_STATUS_FREQ_VALUE);
            append_seq_data(this->tx_frame_, this->tx_frame_size_, &this->status_freq_);

            append_seq_data(this->tx_frame_, this->tx_frame_size_, &CFG_DISTANCE_FREQ_VALUE);
            append_seq_data(this->tx_frame_, this->tx_frame_size_, &this->dist_freq_);

            append_seq_data(this->tx_frame_, this->tx_frame_size_, &CFG_RESPONSE_SPEED_VALUE);
            append_seq_data(this->tx_frame_, this->tx_frame_size_, &this->resp_speed_);

            break;
        }
        break;
      }

    case CALIBRATION_CMD:
      append_seq_data(this->tx_frame_, this->tx_frame_size_, &CALIBRATION_TRIGGER_VALUE);
      append_seq_data(this->tx_frame_, this->tx_frame_size_, &CALIBRATION_RETENTION_VALUE);
      append_seq_data(this->tx_frame_, this->tx_frame_size_, &CALIBRATION_TIME_VALUE);
      break;

    case GATE_THRESHOLD_TRIGGER_READ_CMD:
    case GATE_THRESHOLD_HOLD_READ_CMD:
    case GATE_THRESHOLD_SNR_READ_CMD:
      if (sub_command != NO_SUB_CMD) {
        append_seq_data(this->tx_frame_, this->tx_frame_size_, &sub_command);
      } else {
        for (uint16_t i = 0; i < 16; i++) {
          append_seq_data(this->tx_frame_, this->tx_frame_size_, &i);
        }
      }
      break;

    case GATE_THRESHOLD_TRIGGER_WRITE_CMD:
      if (sub_command != NO_SUB_CMD) {
        append_seq_data(this->tx_frame_, this->tx_frame_size_, &sub_command);
        append_seq_data(this->tx_frame_, this->tx_frame_size_, &this->thresholds_trigger_[sub_command]);
      } else {
        for (uint16_t i = 0; i < 16; i++) {
          append_seq_data(this->tx_frame_, this->tx_frame_size_, &i, 1);
          append_seq_data(this->tx_frame_, this->tx_frame_size_, &this->thresholds_trigger_[i]);
        }
      }
      break;

    case GATE_THRESHOLD_HOLD_WRITE_CMD:
      if (sub_command != NO_SUB_CMD) {
        append_seq_data(this->tx_frame_, this->tx_frame_size_, &sub_command);
        append_seq_data(this->tx_frame_, this->tx_frame_size_, &this->thresholds_hold_[sub_command]);
      } else {
        for (uint16_t i = 0; i < 16; i++) {
          append_seq_data(this->tx_frame_, this->tx_frame_size_, &i);
          append_seq_data(this->tx_frame_, this->tx_frame_size_, &this->thresholds_hold_[i]);
        }
      }
      break;

    case GATE_THRESHOLD_SNR_WRITE_CMD:
      if (sub_command != NO_SUB_CMD) {
        append_seq_data(this->tx_frame_, this->tx_frame_size_, &sub_command);
        append_seq_data(this->tx_frame_, this->tx_frame_size_, &this->thresholds_snr_[sub_command]);
      } else {
        for (uint16_t i = 0; i < 16; i++) {
          append_seq_data(this->tx_frame_, this->tx_frame_size_, &i);
          append_seq_data(this->tx_frame_, this->tx_frame_size_, &this->thresholds_snr_[i]);
        }
      }
      break;

    default:

      break;
  }

  // Frame size
  uint16_t data_size = this->tx_frame_size_ - data_start;
  append_seq_data(this->tx_frame_, size_start, &data_size);

  // Footer
  append_seq_data(this->tx_frame_, this->tx_frame_size_, &CMD_FRAME_FOOTER);
}

// receives frames and starts processing
bool LD2410S::receive_() {
  bool received = false;
  if (this->available()) {
    ESP_LOGD(TAG, "receiving, loop:%d", this->loop_count_);
    received = true;
  }

  int rx_bytes_count = 0;
  while (this->available() && rx_bytes_count < RX_MAX_BYTES_PER_LOOP) {
    //    uint8_t rx = (int8_t) this->read();
    uint8_t rx;
    if (!this->read_byte(&rx))
      break;

#ifdef LD2410S_ENABLE_DC
    this->dc_.receive_byte(rx);
#endif

#ifdef LD2410S_ENABLE_UART
    if (this->rx_.receive_byte(rx) == RxEvaluationResult::OK) {
      this->process_();
    }
    rx_bytes_count++;
  }
#endif

#ifdef LD2410S_ENABLE_DC
  this->dc_.flush();
#endif

  return received;
}
// starts received frame decoding, and handling received data
void LD2410S::process_() {
  switch (this->rx_.frame_type()) {
    case RxFrameType::SHORT_DATA_FRAME:
      this->process_short_data_frame_();
      break;

    case RxFrameType::STD_DATA_FRAME:
      this->process_data_frame_();
      break;

    case RxFrameType::CMD_FRAME:
      this->process_cmd_frame_();
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
#ifdef LD2410S_V2
      uint16_t progress = encode_uint16(this->rx_.payload_data()[2], this->rx_.payload_data()[1]);

      if (progress == 100) {
        this->publish_calibration_runing_(false);
        this->read_all_thresholds_();
      } else {
        this->publish_calibration_runing_(true);
      }
      this->publish_calibration_progress_(progress);
#endif

      break;
    }

    default:
      break;
  }
}
void LD2410S::process_cmd_frame_() {
  uint8_t *data_start = this->rx_.payload_data();
  uint16_t read_position = 0;
  uint16_t command_word = 0;
  uint16_t ack = 0;

  read_seq_data(data_start, read_position, &command_word);
  read_seq_data(data_start, read_position, &ack);

  if (ack != 0x0000) {
    ESP_LOGW(TAG, "Command %04x failed, ack: %04x", command_word, ack);
  }

  this->tx_schedule_.verify_response(command_word);

  uint8_t *data = &data_start[read_position];

  switch (command_word) {
    // Process acknowledgements

#ifdef LD2410S_V2

    case CONFIG_MODE_START_CMD | CMD_CONFIRMATION:
      this->process_ack_config_start_(data);
      break;

    case CONFIG_MODE_END_CMD | CMD_CONFIRMATION:
      ESP_LOGD(TAG, "Config mode disabled");
      break;

    case CALIBRATION_CMD | CMD_CONFIRMATION:
      ESP_LOGD(TAG, "Calibration started");
      break;

      // Write command acknowledgements

    case PARAMS_WRITE_CMD | CMD_CONFIRMATION:
      ESP_LOGD(TAG, "Config written");
      break;

    case OUTPUT_MODE_SWITCH_CMD | CMD_CONFIRMATION:
      this->process_ack_minimal_output_(data);
      break;

    case GATE_THRESHOLD_TRIGGER_WRITE_CMD | CMD_CONFIRMATION:
      ESP_LOGD(TAG, "Trigger Threshold written");
      break;

    case GATE_THRESHOLD_HOLD_WRITE_CMD | CMD_CONFIRMATION:
      ESP_LOGD(TAG, "Trigger Hold written");
      break;

    case GATE_THRESHOLD_SNR_WRITE_CMD | CMD_CONFIRMATION:
      ESP_LOGD(TAG, "Trigger SNR written");
      break;

      // Read command acknowledgements

    case PARAMS_READ_CMD | CMD_CONFIRMATION:
      this->process_ack_config_read_(data);
      break;

    case FW_READ_CMD | CMD_CONFIRMATION:
      this->process_ack_fw_read_(data);
      break;

    case GATE_THRESHOLD_TRIGGER_READ_CMD | CMD_CONFIRMATION:
      this->process_ack_threshold_trigger_read_(data);
      break;

    case GATE_THRESHOLD_HOLD_READ_CMD | CMD_CONFIRMATION:
      this->process_ack_threshold_hold_read_(data);
      break;

    case GATE_THRESHOLD_SNR_READ_CMD | CMD_CONFIRMATION:
      this->process_ack_threshold_snr_read_(data);
      break;
#endif

    default:
      ESP_LOGE(TAG, "< Unknown: %4x", command_word);
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
void LD2410S::publish_presence_(bool presence, bool force_publish) {
#ifdef USE_BINARY_SENSOR
  if (this->presence_binary_sensor_ != nullptr) {
    if (this->presence_binary_sensor_->state != presence || force_publish) {
      this->presence_binary_sensor_->publish_state(presence);
    }
  }
#endif
}

}  // namespace ld2410s
}  // namespace esphome
