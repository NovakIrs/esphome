
#include "ld2410s.h"

namespace esphome {
namespace ld2410s {

// Appends new task to schedule
void LD2410Sschedule::append(uint16_t command, uint16_t sub_command) {
  if (this->last_ >= TX_SCHEDULE_BUFFER_SIZE) {
    ESP_LOGE(TAG, "::: pos:%d, cmd:%04x:%04x, Buffer overflow, reseting buffer !!!", this->last_, command, sub_command);

    this->reset();
    this->state_ = TxCmdState::ERROR;
    return;
  }

  if (this->last_ == 0 && command != CONFIG_MODE_START_CMD) {
    this->append(CONFIG_MODE_START_CMD);
  }

  this->commands_[this->last_].command = command;
  this->commands_[this->last_].sub_command = sub_command;
  this->time_started_ = 0;
  this->retry_count_ = 0;
  if (this->state_ == TxCmdState::EMPTY) {
    this->state_ = TxCmdState::SCHEDULED;
  }

  ESP_LOGI(TAG, "::: pos:%d, cmd:%04x:%04x", this->last_, command, sub_command);

  this->last_++;
}
// Returns active scheduled task status
TxCmdState LD2410Sschedule::check_state() {
  TxTaskT *active = this->get_active_();

  switch (this->state_) {
    case TxCmdState::SCHEDULED:
      this->retry_count_ = 0;
      ESP_LOGD(TAG, "::> pos:%d[:%d], cmd:%04x:%04x, retry:%d, Scheduled", this->active_, this->last_, active->command,
               active->sub_command, this->retry_count_);
      break;

    case TxCmdState::SENT:
      if (App.get_loop_component_start_time() > this->time_started_ + TX_CONFIRMATION_TIMEOUT) {
        if (this->retry_count_ < TX_MAX_RESEND) {
          this->retry_count_++;
          this->state_ = TxCmdState::SEND;
          ESP_LOGE(TAG, ":>> pos:%d[:%d], cmd:%04x:%04x, retry:%d, Send Timeout Expired, Resend!", this->active_,
                   this->last_, active->command, active->sub_command, this->retry_count_);

        } else {
          if (this->restart_count_ < TX_MAX_RESTART) {
            this->active_ = 0;
            this->retry_count_ = 0;
            this->restart_count_++;
            this->state_ = TxCmdState::SCHEDULED;
            ESP_LOGD(TAG, ":>> pos:%d[:%d], cmd:%04x:%04x, retry:%d, Resend limit reached, Restart sequence!!",
                     this->active_, this->last_, this->get_command(), this->get_sub_command(), this->retry_count_);

          } else {
            active = &commands_[this->active_];
            ESP_LOGD(TAG,
                     ":>> pos:%d[:%d], cmd:%04x:%04x, retry:%d, Restart sequence limit reached, Giving up, Reseting "
                     "buffer!!!",
                     this->active_, this->last_, this->get_command(), this->get_sub_command(), this->retry_count_);
            this->reset();
            this->state_ = TxCmdState::ERROR;
          }
        }
      }
      break;

    case TxCmdState::EMPTY:

      // schedule nas reached the end but config was not closed
      if (this->active_ == this->last_ && !this->config_mode_closed_) {
        this->append(CONFIG_MODE_END_CMD);
      }

      if (this->active_ >= this->last_ && this->active_ > 0) {
        this->reset();
      }
      break;

    case TxCmdState::SEND:
    default:
      break;
  }

  return this->state_;
}
// Verifies if received response matches expected, if so procedes to next scheduled command
void LD2410Sschedule::verify_response(uint16_t command_word) {
  int16_t expected = this->get_command() | CMD_CONFIRMATION;
  if (command_word != expected) {
#ifdef LD2410S_DEBUG_UART
    ESP_LOGE(TAG, "Command response %x received, but expected response was %x", command_word, expected);
#endif

  } else {
    ESP_LOGI(TAG, "Command response %x received, confirmed command %x", command_word, this->get_command());

    if (command_word == CONFIG_MODE_END_CMD) {
      this->config_mode_closed_ = true;
    }
    if (this->active_ >= this->last_ - 1) {
      if (this->config_mode_closed_) {
        this->reset();
        return;
      } else {
        this->append(CONFIG_MODE_END_CMD);
        TxCmdState::SCHEDULED;
      }
    } else {
      this->active_++;
      this->state_ = TxCmdState::SCHEDULED;
      if (this->active_ >= TX_SCHEDULE_BUFFER_SIZE) {
        this->reset();
      }
    }
  }
}

// Confirm frame ready
void LD2410Sschedule::confirm_sent() {
  if (this->state_ == TxCmdState::SCHEDULED || this->state_ == TxCmdState::SEND) {
    this->time_started_ = App.get_loop_component_start_time();
    this->state_ = TxCmdState::SENT;
    this->config_mode_closed_ = false;
  }
}

uint16_t LD2410Sschedule::get_command() { return this->get_active_()->command; }
uint16_t LD2410Sschedule::get_sub_command() { return this->get_active_()->sub_command; }
TxTaskT *LD2410Sschedule::get_active_() { return &this->commands_[this->active_]; }
// Resets schedule buffer
void LD2410Sschedule::reset() {
  this->active_ = 0;
  this->last_ = 0;
  this->retry_count_ = 0;
  this->restart_count_ = 0;
  this->state_ = TxCmdState::EMPTY;
}

}  // namespace ld2410s
}  // namespace esphome
