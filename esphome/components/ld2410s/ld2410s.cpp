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
  // ESP_LOGD(TAG, "loop");
  if (!this->receive_()) {
    this->send_();
  }
  this->loop_count_++;
}

float LD2410S::get_setup_priority() const { return setup_priority::HARDWARE; }

// PROTECTED

// builds CMD_FRAME with configuration start end and appends it to the schedule
void LD2410S::schedule_cmd_frames_sequence_(const char *msg, uint16_t command, uint16_t sub_command) {
  ESP_LOGD(TAG, "schedule_cmd_frames_sequence_: %s : %x : %x", msg, command, sub_command);

  this->schedule_cmd_frame_(CONFIG_MODE_START_CMD);
  this->schedule_cmd_frame_(command, sub_command);
  this->schedule_cmd_frame_(CONFIG_MODE_END_CMD);
}
// builds CMD_FRAME as TxFrameT and appends it to the schedule
void LD2410S::schedule_cmd_frame_(uint16_t command, uint16_t sub_command) {
  ESP_LOGD(TAG, "schedule_cmd_frame %x : %x", command, sub_command);

  uint8_t data[RX_TX_BUFFER_SIZE];
  uint16_t data_length = 0;

  this->cmd_frame_append_data_(data, data_length, &command, 1);
  switch (command) {
    case OUTPUT_MODE_SWITCH_CMD: {
      if (this->minimal_output_) {
        this->cmd_frame_append_data_(data, data_length, OUTPUT_MODE_VALUE_MIN, 6);
      } else {
        this->cmd_frame_append_data_(data, data_length, OUTPUT_MODE_VALUE_STD, 6);
      }
    } break;

    case CONFIG_MODE_START_CMD:
      this->cmd_frame_append_data_(data, data_length, &CONFIG_MODE_START_VALUE, 1);
      break;

    case CONFIG_MODE_END_CMD:
      break;

    case PARAMS_READ_CMD:

      switch (sub_command) {
        case CFG_MAX_DETECTION_VALUE:
          this->cmd_frame_append_data_(data, data_length, &CFG_MAX_DETECTION_VALUE, 1);
          break;

        case CFG_MIN_DETECTION_VALUE:
          this->cmd_frame_append_data_(data, data_length, &CFG_MIN_DETECTION_VALUE, 1);
          break;

        case CFG_NO_DELAY_VALUE:
          this->cmd_frame_append_data_(data, data_length, &CFG_NO_DELAY_VALUE, 1);
          break;

        case CFG_STATUS_FREQ_VALUE:
          this->cmd_frame_append_data_(data, data_length, &CFG_STATUS_FREQ_VALUE, 1);
          break;

        case CFG_DISTANCE_FREQ_VALUE:
          this->cmd_frame_append_data_(data, data_length, &CFG_DISTANCE_FREQ_VALUE, 1);
          break;

        case CFG_RESPONSE_SPEED_VALUE:
          this->cmd_frame_append_data_(data, data_length, &CFG_RESPONSE_SPEED_VALUE, 1);
          break;

        default:
          this->cmd_frame_append_data_(data, data_length, &CFG_MAX_DETECTION_VALUE, 1);
          this->cmd_frame_append_data_(data, data_length, &CFG_MIN_DETECTION_VALUE, 1);
          this->cmd_frame_append_data_(data, data_length, &CFG_NO_DELAY_VALUE, 1);
          this->cmd_frame_append_data_(data, data_length, &CFG_STATUS_FREQ_VALUE, 1);
          this->cmd_frame_append_data_(data, data_length, &CFG_DISTANCE_FREQ_VALUE, 1);
          this->cmd_frame_append_data_(data, data_length, &CFG_RESPONSE_SPEED_VALUE, 1);
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
            this->cmd_frame_append_data_(data, data_length, &CFG_MAX_DETECTION_VALUE, 1);
            this->cmd_frame_append_data_(data, data_length, &this->max_dist_, 1);
            break;

          case CFG_MIN_DETECTION_VALUE:
            this->cmd_frame_append_data_(data, data_length, &CFG_MIN_DETECTION_VALUE, 1);
            this->cmd_frame_append_data_(data, data_length, &this->min_dist_, 1);
            break;

          case CFG_NO_DELAY_VALUE:
            this->cmd_frame_append_data_(data, data_length, &CFG_NO_DELAY_VALUE, 1);
            this->cmd_frame_append_data_(data, data_length, &this->delay_, 1);
            break;

          case CFG_STATUS_FREQ_VALUE:
            this->cmd_frame_append_data_(data, data_length, &CFG_STATUS_FREQ_VALUE, 1);
            this->cmd_frame_append_data_(data, data_length, &this->status_freq_, 1);
            break;

          case CFG_DISTANCE_FREQ_VALUE:
            this->cmd_frame_append_data_(data, data_length, &CFG_DISTANCE_FREQ_VALUE, 1);
            this->cmd_frame_append_data_(data, data_length, &this->dist_freq_, 1);
            break;

          case CFG_RESPONSE_SPEED_VALUE:
            this->cmd_frame_append_data_(data, data_length, &CFG_RESPONSE_SPEED_VALUE, 1);
            this->cmd_frame_append_data_(data, data_length, &this->resp_speed_, 1);
            break;

          default:

            this->cmd_frame_append_data_(data, data_length, &CFG_MAX_DETECTION_VALUE, 1);
            this->cmd_frame_append_data_(data, data_length, &this->max_dist_, 1);

            this->cmd_frame_append_data_(data, data_length, &CFG_MIN_DETECTION_VALUE, 1);
            this->cmd_frame_append_data_(data, data_length, &this->min_dist_, 1);

            this->cmd_frame_append_data_(data, data_length, &CFG_NO_DELAY_VALUE, 1);
            this->cmd_frame_append_data_(data, data_length, &this->delay_, 1);

            this->cmd_frame_append_data_(data, data_length, &CFG_STATUS_FREQ_VALUE, 1);
            this->cmd_frame_append_data_(data, data_length, &this->status_freq_, 1);

            this->cmd_frame_append_data_(data, data_length, &CFG_DISTANCE_FREQ_VALUE, 1);
            this->cmd_frame_append_data_(data, data_length, &this->dist_freq_, 1);

            this->cmd_frame_append_data_(data, data_length, &CFG_RESPONSE_SPEED_VALUE, 1);
            this->cmd_frame_append_data_(data, data_length, &this->resp_speed_, 1);

            break;
        }
        break;
      }

    case CALIBRATION_CMD:
      this->cmd_frame_append_data_(data, data_length, &CALIBRATION_TRIGGER_VALUE, 1);
      this->cmd_frame_append_data_(data, data_length, &CALIBRATION_RETENTION_VALUE, 1);
      this->cmd_frame_append_data_(data, data_length, &CALIBRATION_TIME_VALUE, 1);
      break;

    case GATE_THRESHOLD_TRIGGER_READ_CMD:
    case GATE_THRESHOLD_HOLD_READ_CMD:
    case GATE_THRESHOLD_SNR_READ_CMD:
      if (sub_command != NO_SUB_CMD) {
        this->cmd_frame_append_data_(data, data_length, &sub_command, 1);
      } else {
        for (uint16_t i = 0; i < 16; i++) {
          this->cmd_frame_append_data_(data, data_length, &i, 1);
        }
      }
      break;

    case GATE_THRESHOLD_TRIGGER_WRITE_CMD:
      if (sub_command != NO_SUB_CMD) {
        this->cmd_frame_append_data_(data, data_length, &sub_command, 1);
        this->cmd_frame_append_data_(data, data_length, &this->thresholds_trigger_[sub_command], 1);
      } else {
        for (uint16_t i = 0; i < 16; i++) {
          this->cmd_frame_append_data_(data, data_length, &i, 1);
          this->cmd_frame_append_data_(data, data_length, &this->thresholds_trigger_[i], 1);
        }
      }
      break;

    case GATE_THRESHOLD_HOLD_WRITE_CMD:
      if (sub_command != NO_SUB_CMD) {
        this->cmd_frame_append_data_(data, data_length, &sub_command, 1);
        this->cmd_frame_append_data_(data, data_length, &this->thresholds_hold_[sub_command], 1);
      } else {
        for (uint16_t i = 0; i < 16; i++) {
          this->cmd_frame_append_data_(data, data_length, &i, 1);
          this->cmd_frame_append_data_(data, data_length, &this->thresholds_hold_[i], 1);
        }
      }
      break;

    case GATE_THRESHOLD_SNR_WRITE_CMD:
      if (sub_command != NO_SUB_CMD) {
        this->cmd_frame_append_data_(data, data_length, &sub_command, 1);
        this->cmd_frame_append_data_(data, data_length, &this->thresholds_snr_[sub_command], 1);
      } else {
        for (uint16_t i = 0; i < 16; i++) {
          this->cmd_frame_append_data_(data, data_length, &i, 1);
          this->cmd_frame_append_data_(data, data_length, &this->thresholds_snr_[i], 1);
        }
      }
      break;

    default:

      break;
  }

  uint8_t frame[RX_TX_BUFFER_SIZE];
  uint16_t frame_length = 0;

  this->cmd_frame_append_data_(frame, frame_length, &CMD_FRAME_HEADER, 1);
  this->cmd_frame_append_data_(frame, frame_length, &data_length, 1);

  this->cmd_frame_append_data_(frame, frame_length, &data, data_length, 1);
  this->cmd_frame_append_data_(frame, frame_length, &CMD_FRAME_FOOTER, 1);

  this->tx_.schedule_append(command, frame, frame_length);
}
// append append_data to data, returns true if not overflow
template<typename T>
bool LD2410S::cmd_frame_append_data_(uint8_t *data, uint16_t &insert_position, const T *append_data,
                                     uint16_t append_data_size, uint16_t actual_size) {
  size_t data_object_size = actual_size;
  if (data_object_size == 0) {
    data_object_size = sizeof(T);
  }
  auto bytes_to_copy = append_data_size * data_object_size;
  if (insert_position + bytes_to_copy > RX_TX_BUFFER_SIZE) {
    ESP_LOGE(TAG, "cmd_frame_append_data_ overflow: insert_position:%d + append_data_size:%d + object_size:%d > %d",
             insert_position, append_data_size, data_object_size, RX_TX_BUFFER_SIZE);
    return false;
  }

  auto write_ptr = &data[0] + insert_position;
  memcpy(write_ptr, append_data, bytes_to_copy);

  insert_position += bytes_to_copy;

  return true;
}

// prepares scheduled frames for sending
// executes actual data sending
void LD2410S::send_() {
  if (this->tx_.get_error() && !this->init_done_) {
    ESP_LOGI(TAG, "Setup failed, no more scheduled commands, re-initializing...");
    this->init_();
  } else {
    if (this->tx_.send_available()) {
      uint8_t *scheduled_frame = this->tx_.scheduled_frame();
      uint16_t scheduled_frame_length = this->tx_.scheduled_frame_length();

      for (uint16_t index = 0; index < scheduled_frame_length; index++) {
        this->write_byte(scheduled_frame[index]);
      }
      this->flush();

      hex_diag(">", scheduled_frame, scheduled_frame_length);
    }
  }
}

// receives frames and starts processing
bool LD2410S::receive_() {
  bool received = false;
  if (this->available()) {
    received = true;
    ESP_LOGD(TAG, "receiving loop:%d", this->loop_count_);
  }

  int rx_bytes_count = 0;
  while (this->available() && rx_bytes_count < RX_MAX_BYTES_PER_LOOP) {
    if (this->rx_.receive_byte((int8_t) this->read()) == RxEvaluationResult::OK) {
      this->process_();
    }
    rx_bytes_count++;
  }

  return received;
}
// starts received frame decoding, and handling received data
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
  uint16_t command_word = encode_uint16(this->rx_.payload_data()[1], this->rx_.payload_data()[0]);
  uint16_t ack = encode_uint16(this->rx_.payload_data()[3], this->rx_.payload_data()[2]);
  uint8_t *data = &this->rx_.payload_data()[4];

  if (ack != 0x0000) {
    ESP_LOGW(TAG, "Command %x failed, ack: %x", command_word, ack);
  }

  this->tx_.schedule_verify_response(command_word);
  if (this->tx_.schedule_check_empty() && !this->init_done_) {
    ESP_LOGI(TAG, "Setup done");
    this->init_done_ = true;
  }

  switch (command_word) {
    // Process acknowledgements

#ifdef LD2410S_V2

    case CONFIG_MODE_START_CMD | CMD_CONFIRMATION:
      ESP_LOGD(TAG, "Config mode enabled");
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
