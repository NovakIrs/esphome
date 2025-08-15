#pragma once

#include "esphome/core/application.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

#include "esphome/components/uart/uart.h"

#include "ld2410s_const.h"
#include "ld2410s_help.h"

namespace esphome {
namespace ld2410s {

enum class RxFrameType { UNKNOWN, SHORT_DATA_FRAME, STD_DATA_FRAME, CMD_FRAME, NOK };
enum class EvaluationResult { UNKNOWN, OK, NOK };

class LD2410Srx : public uart::UARTDevice, LD2410Shelp {
 public:
  EvaluationResult receive_one(int one);
  RxFrameType frame_type() const { return this->frame_type_; }
  bool payload_ready() const { return payload_ready_; }
  uint8_t *payload_data() { return &this->rcv_buffer_[this->payload_pos_]; }
  uint8_t payload_size() const { return this->payload_size_; }

 protected:
  uint8_t rcv_buffer_[RX_TX_BUFFER_SIZE];
  uint16_t end_pos_{0};

  uint16_t header_footer_size_{0};
  uint16_t expected_frame_size_{0};
  uint16_t size_field_size_{0};

  RxFrameType frame_type_{RxFrameType::UNKNOWN};
  bool payload_ready_{false};
  uint16_t payload_pos_{0};
  uint16_t payload_size_{0};

  EvaluationResult evaluate_();
  EvaluationResult evaluate_header_();
  EvaluationResult evaluate_size_();
  EvaluationResult evaluate_footer_();
  void reset_();
};

}  // namespace ld2410s
}  // namespace esphome
