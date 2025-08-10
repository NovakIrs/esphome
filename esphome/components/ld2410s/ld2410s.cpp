#include "ld2410s.h"

namespace esphome {
namespace ld2410s {

static const uint32_t CMD_EXEC_TIMEOUT = 1000;  // timeout for waiting for cmd response
static const uint8_t CMD_EXEC_REPEAT = 3;

void LD2410S::setup() {
  this->init_();

  this->publish_distance_(0, true);
  this->publish_presence_(false, true);

  this->publish_calibration_progress_(0, true);
  this->publish_calibration_runing_(false, true);

#ifdef LD2410S_V2
  this->set_threshold_selected_gate(0);
#endif
  ESP_LOGD(TAG, "setup done");
}
void LD2410S::loop() {
  if (!this->cmd_active_) {
    App.feed_wdt();
    if (this->rx_.receive()) {
      ESP_LOGD(TAG, "received==true");
      //   this->process_();
    } else if (this->commands_[this->active_].state == CmdState::EMPTY && this->active_ == 0 && this->last_ == 0 &&
               this->init_status_ != 0b11111111) {
      ESP_LOGE(TAG, "Setup failed! Retry...  %x", this->init_status_);
      this->init_();
    } else {
      this->loop_send_command_();
    }
  }
}
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
float LD2410S::get_setup_priority() const { return setup_priority::HARDWARE; }

void LD2410S::set_delay(float delay) {
  this->delay_ = delay;
  this->schedule_cmd_("set_delay\0", PARAMS_WRITE_CMD, CFG_NO_DELAY_VALUE);
}
void LD2410S::set_distance_reporting_freq(float distance_reporting_freq) {
  this->dist_freq_ = distance_reporting_freq * 10;
  this->schedule_cmd_("set_distance_reporting_freq\0", PARAMS_WRITE_CMD, CFG_DISTANCE_FREQ_VALUE);
}
void LD2410S::set_max_distance(float max_distance) {
  this->max_dist_ = static_cast<float>(max_distance) / 0.7f;
  this->schedule_cmd_("set_max_distance\0", PARAMS_WRITE_CMD, CFG_MAX_DETECTION_VALUE);
}
void LD2410S::set_min_distance(float min_distance) {
  this->min_dist_ = static_cast<float>(min_distance) / 0.7f;
  this->schedule_cmd_("set_min_distance\0", PARAMS_WRITE_CMD, CFG_MIN_DETECTION_VALUE);
}
void LD2410S::set_status_reporting_freq(float status_reporting_freq) {
  this->status_freq_ = status_reporting_freq * 10;
  this->schedule_cmd_("set_status_reporting_freq\0", PARAMS_WRITE_CMD, CFG_STATUS_FREQ_VALUE);
}
void LD2410S::set_response_speed_select(const std::string &response_speed_select) {
  this->resp_speed_ = response_speed_select == RESPONSE_SPEED_NORMAL ? 5 : 10;
  this->schedule_cmd_("set_response_speed_select\0", PARAMS_WRITE_CMD, CFG_RESPONSE_SPEED_VALUE);
}

void LD2410S::calibration() { this->schedule_cmd_("calibration\0", CALIBRATION_CMD); }
void LD2410S::factory_reset() {
  this->minimal_output_ = true;

  this->max_dist_ = 16;
  this->min_dist_ = 0;
  this->delay_ = 10;
  this->status_freq_ = 80;
  this->dist_freq_ = 80;
  this->resp_speed_ = 5;

  for (uint8_t i = 0; i < 16; i++) {
    this->thresholds_.trigger[i] = GATE_THRESHOLD_TRIGGER_WRITE_DATA[i];
    this->thresholds_.hold[i] = GATE_THRESHOLD_HOLD_WRITE_DATA[i];
    this->thresholds_.snr[i] = GATE_THRESHOLD_SNR_WRITE_DATA[i];
  }

  this->status_set_warning("factory_reset");
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
  this->status_clear_warning();
}

// PROTECTED

void LD2410S::init_() {
  App.feed_wdt();
  this->status_set_warning("setup");

  this->minimal_output_ = true;

  this->init_status_ = 0;
  this->active_ = 0;
  this->last_ = 0;

  this->schedule_cmd_frame_(CONFIG_MODE_START_CMD);
  this->schedule_cmd_frame_(OUTPUT_MODE_SWITCH_CMD);
  this->schedule_cmd_frame_(FW_READ_CMD);
  this->schedule_cmd_frame_(PARAMS_READ_CMD);
  this->schedule_cmd_frame_(GATE_THRESHOLD_TRIGGER_READ_CMD);
  this->schedule_cmd_frame_(GATE_THRESHOLD_HOLD_READ_CMD);
  this->schedule_cmd_frame_(GATE_THRESHOLD_SNR_READ_CMD);
  this->schedule_cmd_frame_(CONFIG_MODE_END_CMD);

  this->status_clear_warning();
}

void LD2410S::schedule_cmd_(const char *msg, uint16_t command, uint16_t sub_command) {
  this->status_set_warning(msg);

  this->schedule_cmd_frame_(CONFIG_MODE_START_CMD);
  this->schedule_cmd_frame_(command, sub_command);
  this->schedule_cmd_frame_(CONFIG_MODE_END_CMD);

  this->status_clear_warning();
}
void LD2410S::schedule_cmd_frame_(uint16_t command, uint16_t sub_command) {
  ESP_LOGD(TAG, "schedule_cmd_frame %x : %x", command, sub_command);

  CmdFrameT cmd_frame = {.header = CMD_FRAME_HEADER, .footer = CMD_FRAME_FOOTER, .command = command, .data_length = 0};

  switch (command) {
    case OUTPUT_MODE_SWITCH_CMD: {
      if (this->minimal_output_) {
        this->cmd_frame_append_data_(&cmd_frame, &OUTPUT_MODE_VALUE_MIN[0], 4);
      } else {
        this->cmd_frame_append_data_(&cmd_frame, &OUTPUT_MODE_VALUE_STD[0], 4);
      }
    } break;

    case CONFIG_MODE_START_CMD:
      this->cmd_frame_append_data_(&cmd_frame, &CONFIG_MODE_START_VALUE[0], 2);
      break;

    case CONFIG_MODE_END_CMD:
      break;

    case PARAMS_READ_CMD:

      switch (sub_command) {
        case CFG_MAX_DETECTION_VALUE:
          this->cmd_frame_append_data_(&cmd_frame, &CFG_MAX_DETECTION_VALUE, 1);
          break;

        case CFG_MIN_DETECTION_VALUE:
          this->cmd_frame_append_data_(&cmd_frame, &CFG_MIN_DETECTION_VALUE, 1);
          break;

        case CFG_NO_DELAY_VALUE:
          this->cmd_frame_append_data_(&cmd_frame, &CFG_NO_DELAY_VALUE, 1);
          break;

        case CFG_STATUS_FREQ_VALUE:
          this->cmd_frame_append_data_(&cmd_frame, &CFG_STATUS_FREQ_VALUE, 1);
          break;

        case CFG_DISTANCE_FREQ_VALUE:
          this->cmd_frame_append_data_(&cmd_frame, &CFG_DISTANCE_FREQ_VALUE, 1);
          break;

        case CFG_RESPONSE_SPEED_VALUE:
          this->cmd_frame_append_data_(&cmd_frame, &CFG_RESPONSE_SPEED_VALUE, 1);
          break;

        default:
          this->cmd_frame_append_data_(&cmd_frame, &CFG_MAX_DETECTION_VALUE, 1);
          this->cmd_frame_append_data_(&cmd_frame, &CFG_MIN_DETECTION_VALUE, 1);
          this->cmd_frame_append_data_(&cmd_frame, &CFG_NO_DELAY_VALUE, 1);
          this->cmd_frame_append_data_(&cmd_frame, &CFG_STATUS_FREQ_VALUE, 1);
          this->cmd_frame_append_data_(&cmd_frame, &CFG_DISTANCE_FREQ_VALUE, 1);
          this->cmd_frame_append_data_(&cmd_frame, &CFG_RESPONSE_SPEED_VALUE, 1);
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
            this->cmd_frame_append_data_(&cmd_frame, &CFG_MAX_DETECTION_VALUE, 1);
            this->cmd_frame_append_data_(&cmd_frame, &this->max_dist_, 1);
            break;

          case CFG_MIN_DETECTION_VALUE:
            this->cmd_frame_append_data_(&cmd_frame, &CFG_MIN_DETECTION_VALUE, 1);
            this->cmd_frame_append_data_(&cmd_frame, &this->min_dist_, 1);
            break;

          case CFG_NO_DELAY_VALUE:
            this->cmd_frame_append_data_(&cmd_frame, &CFG_NO_DELAY_VALUE, 1);
            this->cmd_frame_append_data_(&cmd_frame, &this->delay_, 1);
            break;

          case CFG_STATUS_FREQ_VALUE:
            this->cmd_frame_append_data_(&cmd_frame, &CFG_STATUS_FREQ_VALUE, 1);
            this->cmd_frame_append_data_(&cmd_frame, &this->status_freq_, 1);
            break;

          case CFG_DISTANCE_FREQ_VALUE:
            this->cmd_frame_append_data_(&cmd_frame, &CFG_DISTANCE_FREQ_VALUE, 1);
            this->cmd_frame_append_data_(&cmd_frame, &this->dist_freq_, 1);
            break;

          case CFG_RESPONSE_SPEED_VALUE:
            this->cmd_frame_append_data_(&cmd_frame, &CFG_RESPONSE_SPEED_VALUE, 1);
            this->cmd_frame_append_data_(&cmd_frame, &this->resp_speed_, 1);
            break;

          default:

            this->cmd_frame_append_data_(&cmd_frame, &CFG_MAX_DETECTION_VALUE, 1);
            this->cmd_frame_append_data_(&cmd_frame, &this->max_dist_, 1);

            this->cmd_frame_append_data_(&cmd_frame, &CFG_MIN_DETECTION_VALUE, 1);
            this->cmd_frame_append_data_(&cmd_frame, &this->min_dist_, 1);

            this->cmd_frame_append_data_(&cmd_frame, &CFG_NO_DELAY_VALUE, 1);
            this->cmd_frame_append_data_(&cmd_frame, &this->delay_, 1);

            this->cmd_frame_append_data_(&cmd_frame, &CFG_STATUS_FREQ_VALUE, 1);
            this->cmd_frame_append_data_(&cmd_frame, &this->status_freq_, 1);

            this->cmd_frame_append_data_(&cmd_frame, &CFG_DISTANCE_FREQ_VALUE, 1);
            this->cmd_frame_append_data_(&cmd_frame, &this->dist_freq_, 1);

            this->cmd_frame_append_data_(&cmd_frame, &CFG_RESPONSE_SPEED_VALUE, 1);
            this->cmd_frame_append_data_(&cmd_frame, &this->resp_speed_, 1);

            break;
        }
        break;
      }

    case CALIBRATION_CMD:
      this->cmd_frame_append_data_(&cmd_frame, &CALIBRATION_TRIGGER_VALUE, 1);
      this->cmd_frame_append_data_(&cmd_frame, &CALIBRATION_RETENTION_VALUE, 1);
      this->cmd_frame_append_data_(&cmd_frame, &CALIBRATION_TIME_VALUE, 1);
      break;

    case GATE_THRESHOLD_TRIGGER_READ_CMD:
    case GATE_THRESHOLD_HOLD_READ_CMD:
    case GATE_THRESHOLD_SNR_READ_CMD:
      if (sub_command != NO_SUB_CMD) {
        this->cmd_frame_append_data_(&cmd_frame, &sub_command, 1);
      } else {
        for (uint16_t i = 0; i < 16; i++) {
          this->cmd_frame_append_data_(&cmd_frame, &i, 1);
        }
      }
      break;

    case GATE_THRESHOLD_TRIGGER_WRITE_CMD:
      if (sub_command != NO_SUB_CMD) {
        this->cmd_frame_append_data_(&cmd_frame, &sub_command, 1);
        this->cmd_frame_append_data_(&cmd_frame, &this->thresholds_.trigger[sub_command], 1);
      } else {
        for (uint16_t i = 0; i < 16; i++) {
          this->cmd_frame_append_data_(&cmd_frame, &i, 1);
          this->cmd_frame_append_data_(&cmd_frame, &this->thresholds_.trigger[i], 1);
        }
      }
      break;

    case GATE_THRESHOLD_HOLD_WRITE_CMD:
      if (sub_command != NO_SUB_CMD) {
        this->cmd_frame_append_data_(&cmd_frame, &sub_command, 1);
        this->cmd_frame_append_data_(&cmd_frame, &this->thresholds_.hold[sub_command], 1);
      } else {
        for (uint16_t i = 0; i < 16; i++) {
          this->cmd_frame_append_data_(&cmd_frame, &i, 1);
          this->cmd_frame_append_data_(&cmd_frame, &this->thresholds_.hold[i], 1);
        }
      }
      break;

    case GATE_THRESHOLD_SNR_WRITE_CMD:
      if (sub_command != NO_SUB_CMD) {
        this->cmd_frame_append_data_(&cmd_frame, &sub_command, 1);
        this->cmd_frame_append_data_(&cmd_frame, &this->thresholds_.snr[sub_command], 1);
      } else {
        for (uint16_t i = 0; i < 16; i++) {
          this->cmd_frame_append_data_(&cmd_frame, &i, 1);
          this->cmd_frame_append_data_(&cmd_frame, &this->thresholds_.snr[i], 1);
        }
      }
      break;

    default:

      break;
  }

  this->cmd_buffer_insert_(&cmd_frame);
}
void LD2410S::cmd_frame_append_data_(CmdFrameT *cmd_frame, const uint8_t *append_data, size_t append_data_size) {
  memcpy(&cmd_frame->data[0] + cmd_frame->data_length * sizeof(cmd_frame->data[0]), append_data,
         append_data_size * sizeof(*append_data));

  cmd_frame->data_length = cmd_frame->data_length + append_data_size * sizeof(*append_data);
}
void LD2410S::cmd_frame_append_data_(CmdFrameT *cmd_frame, const uint16_t *append_data, size_t append_data_size) {
  memcpy(&cmd_frame->data[0] + cmd_frame->data_length * sizeof(cmd_frame->data[0]), append_data,
         append_data_size * sizeof(*append_data));

  cmd_frame->data_length = cmd_frame->data_length + append_data_size * sizeof(*append_data);
}
void LD2410S::cmd_frame_append_data_(CmdFrameT *cmd_frame, const uint32_t *append_data, size_t append_data_size) {
  memcpy(&cmd_frame->data[0] + cmd_frame->data_length * sizeof(cmd_frame->data[0]), append_data,
         append_data_size * sizeof(*append_data));

  cmd_frame->data_length = cmd_frame->data_length + append_data_size * sizeof(*append_data);
}

void LD2410S::cmd_buffer_insert_(CmdFrameT *cmd_frame) {
  if (!cmd_frame) {
    return;
  }

  CmdT cmd;
  cmd.state = CmdState::SCHEDULED;
  cmd.cmd_frame = cmd_frame;
  cmd.time_started = 0;
  cmd.retry = 0;

  if (this->commands_[this->last_].state != CmdState::EMPTY) {
    uint8_t next = this->last_;
    this->cmd_buffer_inc_(next);
    if (this->commands_[next].state != CmdState::EMPTY) {
      return;
    }
    this->last_ = next;
  }

  this->commands_[this->last_] = cmd;  // Shallow copy of state, time_started, retry

  if (cmd.cmd_frame) {
    this->commands_[this->last_].cmd_frame = new CmdFrameT(*cmd.cmd_frame);  // Deep copy
  } else {
    this->commands_[this->last_].cmd_frame = nullptr;
  }
}
void LD2410S::cmd_buffer_finished_() {
  this->commands_[this->active_].state = CmdState::EMPTY;

  if (this->commands_[this->active_ + 1].state != CmdState::EMPTY) {
    this->cmd_buffer_inc_(this->active_);
  }
}
void LD2410S::cmd_buffer_inc_(uint8_t &index) {
  index++;
  if (index >= CMD_EXEC_BUFFER_SIZE) {
    index = 0;
  }
}

void LD2410S::loop_send_command_() {
  CmdT *cmd = &commands_[this->active_];
  uint32_t now = App.get_loop_component_start_time();

  if (cmd->state == CmdState::SCHEDULED) {
    this->send_command_(cmd->cmd_frame);
    cmd->state = CmdState::SENT;
    cmd->time_started = now;

  } else if (cmd->state == CmdState::SENT) {
    if (now >= cmd->time_started + CMD_EXEC_TIMEOUT) {
      if (cmd->retry < CMD_EXEC_REPEAT) {
        ESP_LOGD(TAG, "SendCmd Retry active:%d, last:%d", this->active_, this->last_);
        cmd->retry++;
        cmd->time_started = now;
        this->send_command_(cmd->cmd_frame);
      } else {
        ESP_LOGD(TAG, "SendCmd GivingUp active:%d, last:%d", this->active_, this->last_);
        cmd->state = CmdState::EMPTY;
        this->cmd_buffer_finished_();
      }
    }
  } else if (cmd->state == CmdState::EMPTY && this->active_ == this->last_ && this->active_ != 0) {
    this->active_ = 0;
    this->last_ = 0;
  }
}
void LD2410S::send_command_(CmdFrameT *frame) {
  char output[64];
  sprintf(output, "SendingCommand: %02X", frame->command);
  this->status_set_warning(output);

  this->cmd_active_ = true;
  uint8_t cmd_buffer[128];

  frame->length = 0;
  uint16_t frame_data_bytes = frame->data_length + 2;
  // HEADER
  memcpy(&cmd_buffer[frame->length], &frame->header, sizeof(frame->header));
  frame->length += sizeof(frame->header);
  // SIZE
  memcpy(&cmd_buffer[frame->length], &frame_data_bytes, sizeof(frame->data_length));
  frame->length += sizeof(frame->data_length);
  // COMMAND
  memcpy(&cmd_buffer[frame->length], &frame->command, sizeof(frame->command));
  frame->length += sizeof(frame->command);
  // DATA
  for (uint16_t index = 0; index < frame->data_length; index++) {
    memcpy(&cmd_buffer[frame->length], &frame->data[index], sizeof(frame->data[index]));
    frame->length += sizeof(frame->data[index]);
  }
  // FOOTER
  memcpy(cmd_buffer + frame->length, &frame->footer, sizeof(frame->footer));
  frame->length += sizeof(frame->footer);

  esphome::ld2410s::LD2410S::hex_diag(">", cmd_buffer, frame->length);

  // WRITE
  for (uint16_t index = 0; index < frame->length; index++) {
    this->write_byte(cmd_buffer[index]);
  }

  this->flush();

  this->cmd_active_ = false;

  this->status_clear_warning();
}

void LD2410S::process_() {
  uint8_t *data = &this->rx_.payload_data()[0];
  switch (this->rx_.frame_type()) {
    case RxFrameType::SHORT_DATA_FRAME:
      this->process_short_data_frame_(&data[1]);  // ToDo
      break;

    case RxFrameType::STD_DATA_FRAME:
      this->process_data_frame_(&data[6], this->rx_.payload_size() - 2);  // ToDo
      break;

    case RxFrameType::CMD_FRAME:
      this->process_cmd_frame_(data, this->rx_.payload_size() + 1);  // ToDo
      this->cmd_buffer_finished_();
      break;

    default:
      ESP_LOGE(TAG, "Received Unknown package type!!!");
      break;
  }
}

void LD2410S::process_short_data_frame_(uint8_t *data) {
  const bool presence_state = data[0] > 1;
  uint16_t distance = encode_uint16(data[2], data[1]);
  if (!presence_state)
    distance = 0;

  this->publish_distance_(distance);
  this->publish_presence_(presence_state);
}
void LD2410S::process_data_frame_(uint8_t *data) {
  switch (data[0]) {
    case 0x01:  // standard data
    {
      const bool presence_state = data[1] > 1;
      uint16_t distance = encode_uint16(data[3], data[2]);
      if (!presence_state)
        distance = 0;

      this->publish_distance_(distance);
      this->publish_presence_(presence_state);

#ifdef LD2410S_V2
      this->process_data_energy_values_read_(&data[6]);
#endif

      break;
    }

    case 0x03:  // calibration progress
    {
      uint16_t progress = encode_uint16(data[2], data[1]);

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
void LD2410S::process_cmd_frame_(uint8_t *buffer, size_t len) {
  CmdAckT ack = this->parse_cms_frame_(buffer, len);
  int command_word = ack.command;
  bool result = ack.result;
  if (!result) {
    ESP_LOGW(TAG, "Command %x failed", command_word);
  }

  uint8_t *data = ack.data;

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
CmdAckT LD2410S::parse_cms_frame_(uint8_t *buffer, size_t length) {
  CmdAckT result;
  size_t start = -1;
  for (size_t i = 0; i < length; i++) {
    if (memcmp(&buffer[i], &CMD_FRAME_HEADER, sizeof(CMD_FRAME_HEADER)) == 0) {
      start = i;
      break;
    }
  }
  if (start == -1) {
    ESP_LOGE(TAG, "Can't find cmd header");
    result.result = false;
    return result;
  }
  uint16_t data_length = encode_uint16(buffer[start + 5], buffer[start + 4]);
  result.length = data_length;
  uint16_t command_word = encode_uint16(buffer[start + 7], buffer[start + 6]);
  result.command = command_word;
  bool ack = buffer[start + 8] == 0x00 && buffer[start + 9] == 0x00;
  result.result = ack;
  for (size_t idx = 0; idx < result.length; idx++) {
    memcpy(&result.data[idx], &buffer[idx + 10], sizeof(buffer[idx + 10]));
  }
  return result;
}

void LD2410S::process_ack_config_read_(uint8_t *data) {
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
