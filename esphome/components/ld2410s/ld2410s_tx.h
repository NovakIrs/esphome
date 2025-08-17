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

struct TxFrameT {
  uint8_t data[128];
  uint32_t header;
  uint32_t footer;
  uint16_t length;
  uint16_t command;
  uint16_t data_length;
};

struct TxTaskT {
  uint32_t time_started;
  uint8_t retry;
  CmdState state = CmdState::EMPTY;
  TxFrameT *cmd_frame;
};

// Constants
static const uint16_t NO_SUB_CMD = 0xffff;
static const uint8_t CMD_EXEC_BUFFER_SIZE = 32;

static const uint32_t CMD_EXEC_TIMEOUT = 1000;  // timeout for waiting for cmd response
static const uint8_t CMD_EXEC_REPEAT = 3;

class LD2410Stx : uart::UARTDevice, LD2410Shelp {
 public:
  bool send();

  void schedule_insert(TxFrameT *cmd_frame);
  void schedule_verify_response(uint16_t command_word);

  bool schedule_check_empty() const;
  bool get_error() const { return this->error_; }

  uint8_t tx_buffer[RX_TX_BUFFER_SIZE];
  uint16_t data_length{0};

 protected:
  TxTaskT commands_[CMD_EXEC_BUFFER_SIZE];
  uint8_t active_{0};
  uint8_t last_{0};

  bool error_{false};

  void schedule_reset_();

  void send_frame_(TxFrameT *cmd_frame);
};

}  // namespace ld2410s
}  // namespace esphome
