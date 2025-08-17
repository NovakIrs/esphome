#pragma once

#include "esphome/core/application.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

#include "esphome/components/uart/uart.h"

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "ld2410s_const.h"
#include "ld2410s_help.h"

namespace esphome {
namespace ld2410s {

// struct TxFrameT {
//   uint8_t data[128];
//   uint16_t data_length;
//   uint16_t frame_length;
//   uint16_t command;
// };

struct TxTaskT {
  uint16_t command;
  uint8_t frame[128];
  uint16_t frame_length;
  CmdState state = CmdState::EMPTY;
  uint32_t time_started;
  uint8_t retry;
};

// Constants
static const uint16_t NO_SUB_CMD = 0xffff;
static const uint8_t CMD_EXEC_BUFFER_SIZE = 32;

static const uint32_t CMD_EXEC_TIMEOUT = 1000;  // timeout for waiting for cmd response
static const uint8_t CMD_EXEC_REPEAT = 3;

class LD2410Stx : uart::UARTDevice, LD2410Shelp {
 public:
  bool send_available();

  void schedule_insert(uint16_t command, uint8_t *frame, uint16_t frame_length);
  void schedule_verify_response(uint16_t command_word);

  bool schedule_check_empty() const;
  bool get_error() const { return this->error_; }

  uint8_t *scheduled_frame() { return this->commands_[this->active_].frame; }
  uint16_t scheduled_frame_length() { return this->commands_[this->active_].frame_length; }

 protected:
  TxTaskT commands_[CMD_EXEC_BUFFER_SIZE];
  uint8_t active_{0};
  uint8_t last_{0};

  bool error_{false};

  void schedule_reset_();
};

}  // namespace ld2410s
}  // namespace esphome
