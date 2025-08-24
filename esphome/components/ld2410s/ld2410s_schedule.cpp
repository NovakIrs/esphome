
#include "ld2410s.h"

namespace esphome {
namespace ld2410s {

// Appends new task to schedule
void LD2410Sschedule::append(uint16_t command, uint16_t sub_command) {
  if (this->last_ >= TX_SCHEDULE_BUFFER_SIZE) {
    ESP_LOGE(TAG, "++: pos:[%d], cmd:%04x:%04x, Buffer overflow, reseting buffer !!!", this->last_ - 1, command,
             sub_command);

    this->reset();
    this->state_ = TxCmdState::ERROR;
    return;
  }

  if (this->last_ <= 0) {
    // first cmd must be config start
    if (command != CONFIG_MODE_START_CMD)
      this->append(CONFIG_MODE_START_CMD);
  } else {
    // if last cmd is config end it won't be possible tu just append new command
    if (this->commands_[this->last_ - 1].command == CONFIG_MODE_END_CMD) {
      // If config end is not already sent - another config start must be appended
      if (this->active_ == this->last_ - 1 && this->state_ != TxCmdState::SCHEDULED &&
          command != CONFIG_MODE_START_CMD) {
        ESP_LOGD(TAG, "Last cmd is config end and it's already executing => appending config start");
        this->append(CONFIG_MODE_START_CMD);
      }

      // ... otherwise previous config end can be deleted
      else {
        ESP_LOGD(TAG, "Last cmd was config end and it's not executing executing yet => deleting config end");
        this->last_--;
      }
    }
  }

  this->commands_[this->last_].command = command;
  this->commands_[this->last_].sub_command = sub_command;

  if (this->state_ == TxCmdState::EMPTY) {
    this->state_ = TxCmdState::SCHEDULED;
  }

  ESP_LOGI(TAG, "++: pos:[%d], cmd:%04x:%04x", this->last_, command, sub_command);

  this->last_++;
}
// Returns active scheduled task status
TxCmdState LD2410Sschedule::check_state() {
  switch (this->state_) {
    case TxCmdState::SCHEDULED:
      this->schedule_();
      break;

    case TxCmdState::SENT:
      if (App.get_loop_component_start_time() > this->time_started_ + TX_CONFIRMATION_TIMEOUT) {
        if (this->retry_count_ < TX_MAX_RESEND) {
          this->resend_();

        } else {
          if (this->restart_count_ < TX_MAX_RESTART)
            this->restart_();

          else
            this->give_up_();
        }
      }
      break;

    case TxCmdState::EMPTY:

      // schedule has passed the end
      if (!this->check_append_config_end_())
        this->check_clear_();
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
  if (command_word == expected) {
#ifdef LD2410S_DEBUG_UART
    ESP_LOGV(TAG, "::< pos:%d[%d], cmd:%04x, Sending confirmed, rx:%x", this->active_, this->last_ - 1,
             this->get_command(), command_word);
#endif

    switch (command_word) {
      // config start confirmed
      case CONFIG_MODE_START_CMD | CMD_CONFIRMATION:
        this->config_mode_ = true;
        break;

      // config end confirmed
      case CONFIG_MODE_END_CMD | CMD_CONFIRMATION:
        this->config_mode_ = false;
        break;

      default:
        break;
    }

    if (!this->check_append_config_end_())
      if (check_clear_())
        return;

    // procede to next task
    this->active_++;
    this->state_ = TxCmdState::SCHEDULED;
    if (this->active_ >= TX_SCHEDULE_BUFFER_SIZE) {
      ESP_LOGE(TAG, "::: Schedule overflow, Reseting");
      this->reset();
    }

  } else {
#ifdef LD2410S_DEBUG_UART
    ESP_LOGD(TAG, "::< pos:%d[%d], cmd:%04x, received:%x, Unexpected response", this->active_, this->last_,
             this->get_command(), command_word);
#endif
  }
}

// Confirm frame ready
void LD2410Sschedule::confirm_sent() {
  if (this->state_ == TxCmdState::SCHEDULED || this->state_ == TxCmdState::SEND) {
    this->time_started_ = App.get_loop_component_start_time();
    this->state_ = TxCmdState::SENT;
    this->config_mode_ = true;
  } else {
    ESP_LOGE(TAG, ":>> pos:%d[%d], cmd:%04x, Sending NOT CONFIRMED", this->active_, this->last_, this->get_command());
  }
}

uint16_t LD2410Sschedule::get_command() { return this->get_active_()->command; }
uint16_t LD2410Sschedule::get_sub_command() { return this->get_active_()->sub_command; }
TxTaskT *LD2410Sschedule::get_active_() { return &this->commands_[this->active_]; }
// Resets schedule buffer
void LD2410Sschedule::reset() {
  this->last_ = 0;
  this->active_ = 0;
  this->time_started_ = App.get_loop_component_start_time();
  this->retry_count_ = 0;
  this->restart_count_ = 0;
  this->state_ = TxCmdState::EMPTY;
  ESP_LOGI(TAG, "::: Schedule cleared");
}
void LD2410Sschedule::schedule_() {
  this->time_started_ = App.get_loop_component_start_time();
  this->retry_count_ = 0;
  this->restart_count_ = 0;
  ESP_LOGD(TAG, "::> pos:%d[%d], cmd:%04x, Scheduled", this->active_, this->last_ - 1, this->get_command());
}
void LD2410Sschedule::resend_() {
  this->time_started_ = App.get_loop_component_start_time();
  this->retry_count_++;
  this->state_ = TxCmdState::SEND;
  ESP_LOGW(TAG, ":>> pos:%d[%d], cmd:%04x, retry:%d, restart:%d, Send Timeout Expired, Resend!", this->active_,
           this->last_ - 1, this->get_command(), this->retry_count_, this->restart_count_);
}
void LD2410Sschedule::restart_() {
  this->active_ = 0;
  this->time_started_ = App.get_loop_component_start_time();
  this->retry_count_ = 0;
  this->restart_count_++;
  this->state_ = TxCmdState::SCHEDULED;
  ESP_LOGW(TAG, ":>> pos:%d[:%d], cmd:%04x, retry:%d, restart:%d, Resend limit reached, Restart sequence!!",
           this->active_, this->last_ - 1, this->get_command(), this->retry_count_, this->restart_count_);
}
void LD2410Sschedule::give_up_() {
  ESP_LOGE(
      TAG,
      ":>> pos:%d[:%d], cmd:%04x, retry:%d, restart:%d, Restart sequence limit reached, Giving up, Reseting buffer!!!",
      this->active_, this->last_ - 1, this->get_command(), this->retry_count_, this->restart_count_);
  this->last_ = 0;
  this->active_ = 0;
  this->time_started_ = App.get_loop_component_start_time();
  this->retry_count_ = 0;
  this->restart_count_ = 0;
  this->state_ = TxCmdState::ERROR;
}
bool LD2410Sschedule::check_append_config_end_() {
  if (this->active_ < this->last_ - 1 || this->last_ <= 0 || !this->config_mode_)
    return false;
  ESP_LOGD(TAG, "+:< pos:%d[%d], Appending config end", this->active_, this->last_ - 1);
  this->append(CONFIG_MODE_END_CMD);
  return true;
}
bool LD2410Sschedule::check_clear_() {
  if (this->active_ < this->last_ - 1 || this->last_ <= 0 || this->config_mode_)
    return false;
  this->reset();
  return true;
}
}  // namespace ld2410s
}  // namespace esphome
