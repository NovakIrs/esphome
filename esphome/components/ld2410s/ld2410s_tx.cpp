
#include "ld2410s_tx.h"

namespace esphome {
namespace ld2410s {

void LD2410Stx::cmd_buffer_insert(TxFrameT *cmd_frame) {
  if (!cmd_frame) {
    return;
  }
  if (this->commands_[last_].state != CmdState::EMPTY) {
    ESP_LOGE(TAG, "Inserting into non-empty command buffer location, reseting buffer !!!");
    this->cmd_buffer_reset_();
    this->error_ = true;
    return;
  }
  if (this->error_) {
    this->error_ = false;
  }

  ESP_LOGE(TAG, "Inserting command %x into command buffer at position %d", cmd_frame->command, this->last_);
  this->commands_[this->last_].state = CmdState::SCHEDULED;
  this->commands_[this->last_].time_started = 0;
  this->commands_[this->last_].retry = 0;

  if (cmd_frame) {
    this->commands_[this->last_].cmd_frame = new TxFrameT(*cmd_frame);  // Deep copy
  } else {
    this->commands_[this->last_].cmd_frame = nullptr;
  }

  this->last_++;
  if (this->last_ >= CMD_EXEC_BUFFER_SIZE) {
    this->last_ = 0;
  }
}
void LD2410Stx::cmd_buffer_verify_response(uint16_t command_word) {
  int16_t expected_command = this->commands_[this->active_].cmd_frame->command | CMD_CONFIRMATION;
  if (command_word != expected_command) {
    ESP_LOGD(TAG, "Command response %x received, but expected response was %x", command_word, expected_command);

  } else {
    ESP_LOGD(TAG, "Command response %x received, confirmed command %x", command_word, expected_command);

    this->commands_[this->active_].state = CmdState::EMPTY;

    this->active_++;
    if (this->active_ >= CMD_EXEC_BUFFER_SIZE) {
      this->active_ = 0;
    }

    if (this->commands_[this->active_].state == CmdState::EMPTY) {
      this->cmd_buffer_reset_();
    }
  }
}
void LD2410Stx::cmd_buffer_reset_() {
  ESP_LOGD(TAG, "Command buffer reset");
  this->active_ = 0;
  this->last_ = 0;
  this->commands_[this->active_].state = CmdState::EMPTY;
}

// Returns true if there are commands to send
bool LD2410Stx::send() {
  TxTaskT *cmd = &commands_[this->active_];
  uint32_t now = App.get_loop_component_start_time();

  switch (cmd->state) {
    case CmdState::SCHEDULED:
      ESP_LOGD(TAG, "SCHEDULED: Send scheduled command:%4x, active:%d, last:%d, time_started:%d",
               cmd->cmd_frame->command, this->active_, this->last_, cmd->time_started);
      cmd->time_started = now;
      cmd->retry = 0;
      this->send_frame_(cmd->cmd_frame);
      cmd->state = CmdState::SENT;
      return true;
      break;

    case CmdState::SENT:
      if (now > cmd->time_started + CMD_EXEC_TIMEOUT) {
        ESP_LOGD(TAG, "SENT: Send Timeout Expired !!! , now:%d > time_started:%d + CMD_EXEC_TIMEOUT:%d", now,
                 cmd->time_started, CMD_EXEC_TIMEOUT);

        if (cmd->retry > CMD_EXEC_REPEAT) {
          ESP_LOGD(TAG, "  ... Retry limit reached, giving up !!! , active:%d, last:%d, retry:%d > CMD_EXEC_REPEAT:%d",
                   this->active_, this->last_, cmd->retry, CMD_EXEC_REPEAT);
          this->cmd_buffer_reset_();
          this->error_ = true;
          return false;

        } else {
          ESP_LOGD(TAG, "  ... Retry send !!! , active:%d, last:%d", this->active_, this->last_);
          cmd->retry++;
          cmd->time_started = now;
          this->send_frame_(cmd->cmd_frame);
          return true;
        }
      }
      break;

    case CmdState::EMPTY:
    default:
      // ESP_LOGD(TAG, "EMPTY: , active:%d, last:%d, retry:%d", this->active_, this->last_, cmd->retry);
      if (this->active_ == this->last_ && this->active_ != 0) {
        this->cmd_buffer_reset_();
        return false;  // No commands to send, buffer is empty
      }
      break;
  }

  return false;
}
void LD2410Stx::send_frame_(TxFrameT *frame) {
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
