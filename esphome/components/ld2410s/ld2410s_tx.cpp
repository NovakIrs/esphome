
#include "ld2410s_tx.h"

namespace esphome {
namespace ld2410s {

void LD2410Stx::cmd_buffer_insert(TxFrameT *cmd_frame) {
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
          ESP_LOGD(TAG, "  ... Retry limit reached, giving up !!! , active:%d, last:%d, retry:%d < CMD_EXEC_REPEAT:%d",
                   this->active_, this->last_, cmd->retry, CMD_EXEC_REPEAT);
          this->cmd_buffer_reset_();
          this->error_ = true;
          return false;

        } else {
          ESP_LOGD(TAG, "  ... Retry send !!! , active:%d, last:%d, retry:%d < CMD_EXEC_REPEAT:%d", this->active_,
                   this->last_, cmd->retry, CMD_EXEC_REPEAT);
          cmd->retry++;
          cmd->time_started = now;
          this->send_frame_(cmd->cmd_frame);
          return true;
        }
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
