
#include "ld2410s.h"

namespace esphome {
namespace ld2410s {

// Apends sequence of tasks: config start, actual command and config end
void LD2410Sschedule::append_sequence(const char *msg, uint16_t command, uint16_t sub_command) {
  ESP_LOGD(TAG, "append_sequence: %s : %04x : %04x", msg, command, sub_command);

  this->append(CONFIG_MODE_START_CMD);
  this->append(command, sub_command);
  this->append(CONFIG_MODE_END_CMD);
}
// Appends new task to the end of schedule
void LD2410Sschedule::append(uint16_t command, uint16_t sub_command) {
  ESP_LOGD(TAG, "append: %04x:%04x, last:%d", command, sub_command, this->last_);
  if (this->last_ == 0) {
    if (command != CONFIG_MODE_START_CMD) {
      ESP_LOGI(TAG, "Config start is missing. Apending. command:%04x, last:%d", command, this->last_);
      this->append_(CONFIG_MODE_START_CMD);
    }

  } else {
    if (this->commands_[this->last_ - 1].command == CONFIG_MODE_END_CMD &&
        this->commands_[this->last_ - 1].state == TxCmdState::SCHEDULED) {
      this->last_--;
      this->commands_[this->last_].state = TxCmdState::EMPTY;

      if (command == CONFIG_MODE_START_CMD) {
        ESP_LOGI(TAG,
                 "Config start is requested just after Confing end. Deleting Config end and skipping Config start. "
                 "skipped command:%04x, new last:%d",
                 command, this->last_);
        return;

      } else {
        ESP_LOGI(TAG,
                 "Config start is missing after previous Confing end. Deleting Config end and proceeding with append. "
                 "command:%04x, new last:%d",
                 command, this->last_);
      }
    }
  }

  this->append_(command, sub_command);
}
// Appends new task to the end of schedule
void LD2410Sschedule::append_(uint16_t command, uint16_t sub_command) {
  if (this->commands_[this->last_].state != TxCmdState::EMPTY) {
    ESP_LOGE(TAG,
             "Inserting into non-empty command buffer location, reseting buffer !!! command:%04x, sub_command:%04x, at "
             "position %d",
             command, sub_command, this->last_);
    this->reset();
    this->commands_[this->last_].state = TxCmdState::ERROR;
    return;
  }

  this->commands_[this->last_].command = command;
  this->commands_[this->last_].sub_command = sub_command;
  this->commands_[this->last_].state = TxCmdState::SCHEDULED;
  this->commands_[this->last_].time_started = 0;
  this->commands_[this->last_].retry = 0;

  ESP_LOGI(TAG, "Scheduled command:%04x, sub_command:%04x, at position %d", command, sub_command, this->last_);

  this->last_++;
  if (this->last_ >= TX_SCHEDULE_BUFFER_SIZE) {
    this->last_ = 0;
  }
}
// Resets schedule buffer
void LD2410Sschedule::reset() {
  this->active_ = 0;
  this->last_ = 0;
  this->restart_count_ = 0;
  for (auto &command : this->commands_) {
    command.state = TxCmdState::EMPTY;
  }
}

// Returns active scheduled task status
TxCmdState LD2410Sschedule::check_state() {
  switch (this->commands_[this->active_].state) {
    case TxCmdState::SCHEDULED:
      this->commands_[this->active_].retry = 0;
      ESP_LOGD(TAG, "Send scheduled command:%4x, retry:%d, active:%d, last:%d", this->commands_[this->active_].command,
               this->commands_[this->active_].retry, this->active_, this->last_);
      break;

    case TxCmdState::SENT:
      if (App.get_loop_component_start_time() > this->commands_[this->active_].time_started + TX_CONFIRMATION_TIMEOUT) {
        if (this->commands_[this->active_].retry < TX_MAX_RESEND) {
          this->commands_[this->active_].retry++;
          ESP_LOGD(TAG, "Send Timeout Expired, Resend! command:%4x, retry:%d, active:%d, last:%d",
                   this->commands_[this->active_].command, this->commands_[this->active_].retry, this->active_,
                   this->last_);
          this->commands_[this->active_].state = TxCmdState::SEND;

        } else {
          if (this->restart_count_ < TX_MAX_RESTART) {
            this->restart_count_++;
            for (uint8_t i = 0; i <= this->active_; i++) {
              this->commands_[i].state = TxCmdState::SCHEDULED;
            }
            this->active_ = 0;
            ESP_LOGD(TAG,
                     "Send Timeout Expired, Resend limit reached, Restart sequence!! command:%4x, active:%d, last:%d",
                     this->commands_[this->active_].command, this->active_, this->last_);

          } else {
            this->reset();
            ESP_LOGD(TAG,
                     "Send Timeout Expired, Resend limit reached, Restart limit reached, Giving up, Reseting buffer!!! "
                     "command:%4x, active:%d, last:%d",
                     this->commands_[this->active_].command, this->active_, this->last_);
            this->commands_[this->active_].state = TxCmdState::ERROR;
          }
        }
      }
      break;

    case TxCmdState::EMPTY:
      if (this->active_ == this->last_ && this->active_ > 0) {
        // schedule has reached the end
        if (this->commands_[this->active_ - 1].command != CONFIG_MODE_END_CMD) {
          // schedule nas reached the end but config was not closed
          this->append_(CONFIG_MODE_END_CMD);

        } else {
          this->reset();
        }
      }
      break;

    case TxCmdState::SEND:
    default:
      break;
  }

  return this->commands_[this->active_].state;
}
// Verifies if received response matches expected, if so procedes to next scheduled command
void LD2410Sschedule::verify_response(uint16_t command_word) {
  int16_t sent = this->commands_[this->active_].command;
  int16_t expected = sent | CMD_CONFIRMATION;
  if (command_word != expected) {
#ifdef LD2410S_DEBUG_UART
    ESP_LOGE(TAG, "Command response %x received, but expected response was %x", command_word, expected);
#endif

  } else {
    ESP_LOGI(TAG, "Command response %x received, confirmed command %x", command_word, sent);

    this->commands_[this->active_].state = TxCmdState::EMPTY;

    this->active_++;
    if (this->active_ >= TX_SCHEDULE_BUFFER_SIZE) {
      this->active_ = 0;
    }

    if (this->commands_[this->active_].state == TxCmdState::EMPTY) {
      this->reset();
    }
  }
}

// Confirm frame ready
void LD2410Sschedule::confirm_sent() {
  if (this->commands_[this->active_].state == TxCmdState::SCHEDULED) {
    TxTaskT *cmd = &commands_[this->active_];
    cmd->time_started = App.get_loop_component_start_time();
    cmd->state = TxCmdState::SENT;
  }
}

}  // namespace ld2410s
}  // namespace esphome
