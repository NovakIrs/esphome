
#include "ld2410s_tx.h"

namespace esphome {
namespace ld2410s {

void LD2410Stx::schedule_cmd_sequence_(const char *msg, uint16_t command, uint16_t sub_command) {
  ESP_LOGD(TAG, "schedule_cmd_sequence_: %s : %x : %x", msg, command, sub_command);

  this->schedule_cmd_frame_(CONFIG_MODE_START_CMD);
  this->schedule_cmd_frame_(command, sub_command);
  this->schedule_cmd_frame_(CONFIG_MODE_END_CMD);
}
// builds CMD_FRAME as TxFrameT and appends it to the command buffer
void LD2410Stx::schedule_cmd_frame_(uint16_t command, uint16_t sub_command) {
  ESP_LOGD(TAG, "schedule_cmd_frame %x : %x", command, sub_command);

  TxFrameT cmd_frame = {.header = CMD_FRAME_HEADER, .footer = CMD_FRAME_FOOTER, .command = command, .data_length = 0};

  switch (command) {
    case OUTPUT_MODE_SWITCH_CMD: {
      if (this->settings_.minimal_output) {
        this->cmd_frame_append_data_(&cmd_frame, OUTPUT_MODE_VALUE_MIN, 6);
      } else {
        this->cmd_frame_append_data_(&cmd_frame, OUTPUT_MODE_VALUE_STD, 6);
      }
    } break;

    case CONFIG_MODE_START_CMD:
      this->cmd_frame_append_data_(&cmd_frame, &CONFIG_MODE_START_VALUE, 1);
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
      if (this->settings_.resp_speed == 0) {
        ESP_LOGD(TAG, "PARAMS_WRITE_CMD Error, bad new_config");
        return;
      } else {
        switch (sub_command) {
          case CFG_MAX_DETECTION_VALUE:
            this->cmd_frame_append_data_(&cmd_frame, &CFG_MAX_DETECTION_VALUE, 1);
            this->cmd_frame_append_data_(&cmd_frame, &this->settings_.max_dist, 1);
            break;

          case CFG_MIN_DETECTION_VALUE:
            this->cmd_frame_append_data_(&cmd_frame, &CFG_MIN_DETECTION_VALUE, 1);
            this->cmd_frame_append_data_(&cmd_frame, &this->settings_.min_dist, 1);
            break;

          case CFG_NO_DELAY_VALUE:
            this->cmd_frame_append_data_(&cmd_frame, &CFG_NO_DELAY_VALUE, 1);
            this->cmd_frame_append_data_(&cmd_frame, &this->settings_.delay, 1);
            break;

          case CFG_STATUS_FREQ_VALUE:
            this->cmd_frame_append_data_(&cmd_frame, &CFG_STATUS_FREQ_VALUE, 1);
            this->cmd_frame_append_data_(&cmd_frame, &this->settings_.status_freq, 1);
            break;

          case CFG_DISTANCE_FREQ_VALUE:
            this->cmd_frame_append_data_(&cmd_frame, &CFG_DISTANCE_FREQ_VALUE, 1);
            this->cmd_frame_append_data_(&cmd_frame, &this->settings_.dist_freq, 1);
            break;

          case CFG_RESPONSE_SPEED_VALUE:
            this->cmd_frame_append_data_(&cmd_frame, &CFG_RESPONSE_SPEED_VALUE, 1);
            this->cmd_frame_append_data_(&cmd_frame, &this->settings_.resp_speed, 1);
            break;

          default:

            this->cmd_frame_append_data_(&cmd_frame, &CFG_MAX_DETECTION_VALUE, 1);
            this->cmd_frame_append_data_(&cmd_frame, &this->settings_.max_dist, 1);

            this->cmd_frame_append_data_(&cmd_frame, &CFG_MIN_DETECTION_VALUE, 1);
            this->cmd_frame_append_data_(&cmd_frame, &this->settings_.min_dist, 1);

            this->cmd_frame_append_data_(&cmd_frame, &CFG_NO_DELAY_VALUE, 1);
            this->cmd_frame_append_data_(&cmd_frame, &this->settings_.delay, 1);

            this->cmd_frame_append_data_(&cmd_frame, &CFG_STATUS_FREQ_VALUE, 1);
            this->cmd_frame_append_data_(&cmd_frame, &this->settings_.status_freq, 1);

            this->cmd_frame_append_data_(&cmd_frame, &CFG_DISTANCE_FREQ_VALUE, 1);
            this->cmd_frame_append_data_(&cmd_frame, &this->settings_.dist_freq, 1);

            this->cmd_frame_append_data_(&cmd_frame, &CFG_RESPONSE_SPEED_VALUE, 1);
            this->cmd_frame_append_data_(&cmd_frame, &this->settings_.resp_speed, 1);

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
        this->cmd_frame_append_data_(&cmd_frame, &this->settings_.thresholds.trigger[sub_command], 1);
      } else {
        for (uint16_t i = 0; i < 16; i++) {
          this->cmd_frame_append_data_(&cmd_frame, &i, 1);
          this->cmd_frame_append_data_(&cmd_frame, &this->settings_.thresholds.trigger[i], 1);
        }
      }
      break;

    case GATE_THRESHOLD_HOLD_WRITE_CMD:
      if (sub_command != NO_SUB_CMD) {
        this->cmd_frame_append_data_(&cmd_frame, &sub_command, 1);
        this->cmd_frame_append_data_(&cmd_frame, &this->settings_.thresholds.hold[sub_command], 1);
      } else {
        for (uint16_t i = 0; i < 16; i++) {
          this->cmd_frame_append_data_(&cmd_frame, &i, 1);
          this->cmd_frame_append_data_(&cmd_frame, &this->settings_.thresholds.hold[i], 1);
        }
      }
      break;

    case GATE_THRESHOLD_SNR_WRITE_CMD:
      if (sub_command != NO_SUB_CMD) {
        this->cmd_frame_append_data_(&cmd_frame, &sub_command, 1);
        this->cmd_frame_append_data_(&cmd_frame, &this->settings_.thresholds.snr[sub_command], 1);
      } else {
        for (uint16_t i = 0; i < 16; i++) {
          this->cmd_frame_append_data_(&cmd_frame, &i, 1);
          this->cmd_frame_append_data_(&cmd_frame, &this->settings_.thresholds.snr[i], 1);
        }
      }
      break;

    default:

      break;
  }

  this->cmd_buffer_insert_(&cmd_frame);
}

template<typename T>
void LD2410Stx::cmd_frame_append_data_(TxFrameT *cmd_frame, const T *append_data, size_t append_data_size) {
  auto bytes_to_copy = append_data_size * sizeof(T);
  auto write_ptr = &cmd_frame->data[0] + cmd_frame->data_length;

  memcpy(write_ptr, append_data, bytes_to_copy);
  cmd_frame->data_length += bytes_to_copy;
}

void LD2410Stx::cmd_buffer_insert_(TxFrameT *cmd_frame) {
  if (!cmd_frame) {
    return;
  }

  if (this->error_) {
    this->error_ = false;
  }

  TxTaskT cmd;
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
    this->commands_[this->last_].cmd_frame = new TxFrameT(*cmd.cmd_frame);  // Deep copy
  } else {
    this->commands_[this->last_].cmd_frame = nullptr;
  }
}
void LD2410Stx::cmd_buffer_verify_response(uint16_t command_word = 0xFFFF) {
  int16_t expected_command = this->commands_[this->active_].cmd_frame->command | CMD_CONFIRMATION;
  if (command_word != expected_command && command_word != 0xFFFF) {
    ESP_LOGD(TAG, "Command response %x received, but expected response was %x", command_word, expected_command);
    return;
  }
  ESP_LOGD(TAG, "Command response %x received, confirmed command %x", command_word, expected_command);

  this->commands_[this->active_].state = CmdState::EMPTY;

  if (this->commands_[this->active_ + 1].state != CmdState::EMPTY) {
    this->cmd_buffer_inc_(this->active_);
  }
}
void LD2410Stx::cmd_buffer_inc_(uint8_t &index) {
  index++;
  if (index >= CMD_EXEC_BUFFER_SIZE) {
    index = 0;
  }
}
void LD2410Stx::cmd_buffer_reset_() {
  this->active_ = 0;
  this->last_ = 0;
  this->commands_[this->active_].state = CmdState::EMPTY;
}
bool LD2410Stx::loop_send_command_() {
  TxTaskT *cmd = &commands_[this->active_];
  uint32_t now = App.get_loop_component_start_time();

  switch (cmd->state) {
    case CmdState::SCHEDULED:
      // sending scheduled command, waiting for response
      ESP_LOGD(TAG, "Send scheduled command:%x4, active:%d, last:%d", cmd->cmd_frame->command, this->active_,
               this->last_);
      cmd->time_started = now;
      this->send_command_(cmd->cmd_frame);
      cmd->state = CmdState::SENT;
      return true;
      break;

    case CmdState::SENT:
      if (now >= cmd->time_started + CMD_EXEC_TIMEOUT) {
        // send timeout expired

        if (cmd->retry < CMD_EXEC_REPEAT) {
          ESP_LOGD(TAG, "SendCmd timeout expired, active:%d, last:%d", this->active_, this->last_);
          // retry sending command
          ESP_LOGD(TAG, "SendCmd Retry, active:%d, last:%d", this->active_, this->last_);
          cmd->retry++;
          cmd->time_started = now;
          this->send_command_(cmd->cmd_frame);
          return true;
        }
      } else {
        // retry limit reached, giving up, reset buffer, do init
        ESP_LOGD(TAG, "SendCmd GivingUp  active:%d, last:%d", this->active_, this->last_);
        this->cmd_buffer_reset_();
        this->error_ = true;
        return false;  // Command send failed, reset buffer, do init
      }
      break;

    case CmdState::EMPTY:
      if (this->active_ == this->last_ && this->active_ != 0) {
        this->cmd_buffer_reset_();
        return false;  // No commands to send, buffer is empty
      }
      break;
  }

  return false;
}
void LD2410Stx::send_command_(TxFrameT *frame) {
  char output[64];
  sprintf(output, "SendingCommand: %02X", frame->command);

  frame->length = 0;
  uint16_t frame_data_bytes = frame->data_length + 2;
  // HEADER
  memcpy(&this->tx_buffer[frame->length], &frame->header, sizeof(frame->header));
  frame->length += sizeof(frame->header);
  // SIZE
  memcpy(&this->tx_buffer[frame->length], &frame_data_bytes, sizeof(frame->data_length));
  frame->length += sizeof(frame->data_length);
  // COMMAND
  memcpy(&this->tx_buffer[frame->length], &frame->command, sizeof(frame->command));
  frame->length += sizeof(frame->command);
  // DATA
  for (uint16_t index = 0; index < frame->data_length; index++) {
    memcpy(&this->tx_buffer[frame->length], &frame->data[index], sizeof(frame->data[index]));
    frame->length += sizeof(frame->data[index]);
  }
  // FOOTER
  memcpy(tx_buffer + frame->length, &frame->footer, sizeof(frame->footer));
  frame->length += sizeof(frame->footer);

  this->data_length = frame->length;
}

}  // namespace ld2410s
}  // namespace esphome
