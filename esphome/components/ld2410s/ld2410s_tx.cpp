
#include "ld2410s_tx.h"

namespace esphome {
namespace ld2410s {

void LD2410Stx::schedule_insert(uint16_t command, uint8_t *frame, uint16_t frame_length) {
  if (!frame) {
    return;
  }
  if (frame_length = 0 || frame_length > RX_TX_BUFFER_SIZE) {
    return;
  }
  TxTaskT *task = &this->commands_[last_];
  if (task->state != CmdState::EMPTY) {
    ESP_LOGE(TAG, "Inserting into non-empty command buffer location, reseting buffer !!!");
    this->schedule_reset_();
    this->error_ = true;
    return;
  }

  if (this->error_) {
    this->error_ = false;
  }

  task->command = command;
  memcpy(task->frame, frame, frame_length);
  task->frame_length = frame_length;
  task->state = CmdState::SCHEDULED;
  task->time_started = 0;
  task->retry = 0;

  ESP_LOGD(TAG, "Inserted command %x into command buffer at position %d", frame_length, this->last_);

  this->last_++;
  if (this->last_ >= CMD_EXEC_BUFFER_SIZE) {
    this->last_ = 0;
  }
}
void LD2410Stx::schedule_verify_response(uint16_t command_word) {
  int16_t expected_command = this->commands_[this->active_].command;
  if (command_word != expected_command | CMD_CONFIRMATION) {
    ESP_LOGD(TAG, "Command response %x received, but expected response was %x", command_word,
             expected_command | CMD_CONFIRMATION);

  } else {
    ESP_LOGD(TAG, "Command response %x received, confirmed command %x", command_word, expected_command);

    this->commands_[this->active_].state = CmdState::EMPTY;

    this->active_++;
    if (this->active_ >= CMD_EXEC_BUFFER_SIZE) {
      this->active_ = 0;
    }

    if (this->commands_[this->active_].state == CmdState::EMPTY) {
      this->schedule_reset_();
    }
  }
}
void LD2410Stx::schedule_reset_() {
  ESP_LOGD(TAG, "Command buffer reset");
  this->active_ = 0;
  this->last_ = 0;
  for (uint8_t index = 0; index < CMD_EXEC_BUFFER_SIZE; index++) {
    this->commands_[index].state = CmdState::EMPTY;
  }
  this->commands_[this->active_].state = CmdState::EMPTY;
}
bool LD2410Stx::schedule_check_empty() const {
  ESP_LOGI(TAG, "schedule_check_empty: active:%d, last:%d, empty:%d", this->active_, this->last_,
           this->commands_[this->active_].state == CmdState::EMPTY);
  return this->commands_[this->active_].state == CmdState::EMPTY && this->active_ == 0 && this->last_ == 0;
}

// Returns true if there are commands to send
bool LD2410Stx::send_available() {
  TxTaskT *cmd = &commands_[this->active_];
  uint32_t now = App.get_loop_component_start_time();

  switch (cmd->state) {
    case CmdState::SCHEDULED:
      ESP_LOGD(TAG, "SCHEDULED: Send scheduled command:%4x, active:%d, last:%d, time_started:%d", cmd->command,
               this->active_, this->last_, cmd->time_started);
      cmd->time_started = now;
      cmd->retry = 0;
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
          this->schedule_reset_();
          this->error_ = true;
          return false;

        } else {
          ESP_LOGD(TAG, "  ... Retry send !!! , active:%d, last:%d", this->active_, this->last_);
          cmd->retry++;
          cmd->time_started = now;
          return true;
        }
      }
      break;

    case CmdState::EMPTY:
    default:
      // ESP_LOGD(TAG, "EMPTY: , active:%d, last:%d, retry:%d", this->active_, this->last_, cmd->retry);
      if (this->active_ == this->last_ && this->active_ != 0) {
        this->schedule_reset_();
        return false;  // No commands to send, buffer is empty
      }
      break;
  }

  return false;
}

}  // namespace ld2410s
}  // namespace esphome
