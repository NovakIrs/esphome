
#include "ld2410s.h"

namespace esphome {
namespace ld2410s {

// Appends new task to the end of schedule
void LD2410Stx::schedule_append(uint16_t command, uint8_t *frame, uint16_t frame_length) {
  if (!frame) {
    ESP_LOGE(TAG, "schedule_append: no frame data !!!");
    return;
  }
  if (frame_length == 0 || frame_length > RX_TX_BUFFER_SIZE) {
    ESP_LOGE(TAG, "schedule_append: frame_length:%d !!!", frame_length);
    return;
  }

  TxTaskT *task = &this->commands_[last_];
  if (task->state != TxCmdState::EMPTY) {
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
  task->state = TxCmdState::SCHEDULED;
  task->time_started = 0;
  task->retry = 0;

  this->last_++;
  if (this->last_ >= CMD_EXEC_BUFFER_SIZE) {
    this->last_ = 0;
  }

  ESP_LOGD(TAG, "Scheduled command %x at position %d, size:%d", frame_length, this->last_, frame_length);
}
// Returns true if schedule is empty
bool LD2410Stx::schedule_check_empty() const {
  return this->commands_[this->active_].state == TxCmdState::EMPTY && this->active_ == 0 && this->last_ == 0;
}
// Verifies if received response matches expected, if so procedes to next scheduled command
void LD2410Stx::schedule_verify_response(uint16_t response) {
  int16_t sent = this->commands_[this->active_].command;
  int16_t expected = sent | CMD_CONFIRMATION;
  if (response != expected) {
    ESP_LOGD(TAG, "Command response %x received, but expected response was %x", response, expected);

  } else {
    ESP_LOGD(TAG, "Command response %x received, confirmed command %x", response, sent);

    this->commands_[this->active_].state = TxCmdState::EMPTY;

    this->active_++;
    if (this->active_ >= CMD_EXEC_BUFFER_SIZE) {
      this->active_ = 0;
    }

    if (this->commands_[this->active_].state == TxCmdState::EMPTY) {
      this->schedule_reset_();
    }
  }
}
// Resets schedule buffer
void LD2410Stx::schedule_reset_() {
  ESP_LOGW(TAG, "Command buffer reset");
  this->active_ = 0;
  this->last_ = 0;
  for (uint8_t index = 0; index < CMD_EXEC_BUFFER_SIZE; index++) {
    this->commands_[index].state = TxCmdState::EMPTY;
  }
  this->commands_[this->active_].state = TxCmdState::EMPTY;
}
// Returns true if there is command frame ready for sending
bool LD2410Stx::send_available() {
  TxTaskT *cmd = &commands_[this->active_];
  uint32_t now = App.get_loop_component_start_time();

  switch (cmd->state) {
    case TxCmdState::SCHEDULED:
      ESP_LOGD(TAG, "SCHEDULED: Send scheduled command:%4x, active:%d, last:%d, time_started:%d, size:%d", cmd->command,
               this->active_, this->last_, cmd->time_started, cmd->frame_length);
      cmd->time_started = now;
      cmd->retry = 0;
      cmd->state = TxCmdState::SENT;
      return true;
      break;

    case TxCmdState::SENT:
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

    case TxCmdState::EMPTY:
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
