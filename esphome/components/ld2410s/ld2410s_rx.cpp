
#include "ld2410s.h"

namespace esphome {
namespace ld2410s {

// appends one byte to rx buffer, and checks if that makes complete frame
RxEvaluationResult LD2410Srx::receive_byte(uint8_t byte) {
  if (this->payload_ready_) {
    this->reset_();
  }

  this->rcv_buffer_[this->end_pos_] = byte;
  RxEvaluationResult result = this->evaluate_();

  switch (result) {
    case RxEvaluationResult::OK:
      this->payload_ready_ = true;
      ESP_LOGI(TAG, "< %s", format_hex_pretty(this->rcv_buffer_, end_pos_ + 1, ' ').c_str());
      break;

    case RxEvaluationResult::UNKNOWN:
      this->end_pos_++;
      if (this->end_pos_ > RX_TX_BUFFER_SIZE) {
        ESP_LOGD(TAG, "Received data buffer overflow, resetting");
        this->reset_();
      } else {
      }
      break;

    case RxEvaluationResult::NOK:
    default:
      ESP_LOGI(TAG, "< %s", format_hex_pretty(this->rcv_buffer_, end_pos_ + 1, ' ').c_str());
      this->reset_();
      result = RxEvaluationResult::UNKNOWN;
      break;
  }

  return result;
}
// checks if current rx buffer is full frame
RxEvaluationResult LD2410Srx::evaluate_() {
  switch (this->evaluate_header_()) {
    case RxEvaluationResult::NOK:  // header does not match known frame type ie bad header
      // ESP_LOGD(TAG, "header does not match known frame type ie bad header: %d", this->end_pos_);
      return RxEvaluationResult::NOK;

    case RxEvaluationResult::UNKNOWN:  // not enough data yet to determine frame type
      return RxEvaluationResult::UNKNOWN;

    case RxEvaluationResult::OK:
    default:  // header ok, known type
      break;
  }

  switch (this->evaluate_size_()) {
    case RxEvaluationResult::NOK:  // known size, but greater then expected size for frame type
#ifdef LD2410S_DEBUG_UART
      ESP_LOGD(TAG, "correct header, but passed expected frame end: size:%d, expected:%d", this->end_pos_,
               this->expected_frame_size_);
#endif
      return RxEvaluationResult::NOK;

    case RxEvaluationResult::UNKNOWN:  // not enough data yet to determine correct size
      return RxEvaluationResult::UNKNOWN;

    case RxEvaluationResult::OK:  // correct size for frame type
    default:
      break;
  }

  switch (this->evaluate_footer_()) {
    case RxEvaluationResult::NOK:  // size matches expected size, but footer does not match expected footer for frame
                                   // type
      ESP_LOGD(TAG,
               "correct header and size, but footer does not match expected: real:%d, expected:%d, "
               "head/foot:%d, size:%d, payload:%d",
               this->end_pos_ + 1, this->expected_frame_size_, this->header_footer_size_, this->size_field_size_,
               this->payload_size_);
      switch (this->frame_type_) {
        case RxFrameType::SHORT_DATA_FRAME:
          ESP_LOGD(TAG, "SHORT_DATA_FRAME: %02X", SHORT_DATA_FRAME_HEADER);
          break;

        case RxFrameType::STD_DATA_FRAME:
          ESP_LOGD(TAG, "STD_DATA_FRAME: %08X", STD_DATA_FRAME_HEADER);
          break;

        case RxFrameType::CMD_FRAME:
          ESP_LOGD(TAG, "CMD_FRAME: %08X", CMD_FRAME_HEADER);
          break;

        default:
          break;
      }
      return RxEvaluationResult::NOK;

    case RxEvaluationResult::UNKNOWN:  // size less then expected size for frame type
      return RxEvaluationResult::UNKNOWN;

    default:  // size matches expected size, footer matches expected footer for frame type
      break;
  }

  return RxEvaluationResult::OK;  // full frame received and verified
}
// checks if current rx buffer contains header
RxEvaluationResult LD2410Srx::evaluate_header_() {
  switch (this->frame_type_) {
    case RxFrameType::CMD_FRAME:
    case RxFrameType::STD_DATA_FRAME:
    case RxFrameType::SHORT_DATA_FRAME:
      return RxEvaluationResult::OK;  // already determined frame type

    case RxFrameType::NOK:
      return RxEvaluationResult::NOK;  // already determined bad header

    case RxFrameType::UNKNOWN:
    default:
      break;  // need to determine frame type
  }

  if (this->end_pos_ + 1 == sizeof(SHORT_DATA_FRAME_HEADER) &&
      memcmp(&this->rcv_buffer_[0], &SHORT_DATA_FRAME_HEADER, sizeof(SHORT_DATA_FRAME_HEADER)) == 0) {
    this->frame_type_ = RxFrameType::SHORT_DATA_FRAME;
    this->header_footer_size_ = sizeof(SHORT_DATA_FRAME_HEADER);
    return RxEvaluationResult::OK;
  }

  if (this->end_pos_ + 1 == sizeof(STD_DATA_FRAME_HEADER) &&
      memcmp(&this->rcv_buffer_[0], &STD_DATA_FRAME_HEADER, sizeof(STD_DATA_FRAME_HEADER)) == 0) {
    this->frame_type_ = RxFrameType::STD_DATA_FRAME;
    this->header_footer_size_ = sizeof(STD_DATA_FRAME_HEADER);
    return RxEvaluationResult::OK;
  }

  if (this->end_pos_ + 1 == sizeof(CMD_FRAME_HEADER) &&
      memcmp(&this->rcv_buffer_[0], &CMD_FRAME_HEADER, sizeof(CMD_FRAME_HEADER)) == 0) {
    this->frame_type_ = RxFrameType::CMD_FRAME;
    this->header_footer_size_ = sizeof(CMD_FRAME_HEADER);
    return RxEvaluationResult::OK;
  }

  if (this->end_pos_ + 1 < sizeof(STD_DATA_FRAME_HEADER) &&
      memcmp(&this->rcv_buffer_[0], &STD_DATA_FRAME_HEADER, this->end_pos_ + 1) == 0) {
    this->frame_type_ =
        RxFrameType::UNKNOWN;  // not enough data yet to determine frame type, but it fits STD frame header
    this->header_footer_size_ = 0;
    return RxEvaluationResult::UNKNOWN;
  }

  if (this->end_pos_ + 1 < sizeof(CMD_FRAME_HEADER) &&
      memcmp(&this->rcv_buffer_[0], &CMD_FRAME_HEADER, this->end_pos_ + 1) == 0) {
    this->frame_type_ =
        RxFrameType::UNKNOWN;  // not enough data yet to determine frame type, but it fits CMD frame header
    this->header_footer_size_ = 0;
    return RxEvaluationResult::UNKNOWN;
  }

  this->frame_type_ = RxFrameType::NOK;  // bad header
#ifdef LD2410S_DEBUG_UART
  ESP_LOGE(TAG, "rx received unknown header, length:%d", end_pos_ + 1);
#endif
  return RxEvaluationResult::NOK;
}
// checks if current rx buffer has proper size for decoded header
RxEvaluationResult LD2410Srx::evaluate_size_() {
  switch (this->frame_type_) {
    case RxFrameType::UNKNOWN:
      return RxEvaluationResult::UNKNOWN;  // not enough data yet to determine size

    case RxFrameType::NOK:
      return RxEvaluationResult::NOK;  // already determined bad header

    case RxFrameType::SHORT_DATA_FRAME:
      if (this->expected_frame_size_ == 0) {
        this->size_field_size_ = 0;
        this->payload_size_ = 3;
        this->payload_pos_ = this->header_footer_size_;
        this->expected_frame_size_ = 2 * this->header_footer_size_ + 3;
      }
      break;

    case RxFrameType::STD_DATA_FRAME:
    case RxFrameType::CMD_FRAME:
      if (this->expected_frame_size_ == 0) {
        this->size_field_size_ = FRAME_DATA_LENGTH_SIZE;
        if (this->end_pos_ >= this->header_footer_size_ + this->size_field_size_) {
          this->payload_size_ = read_int(this->rcv_buffer_, this->header_footer_size_, 2);
          this->payload_pos_ = this->header_footer_size_ + this->size_field_size_;
          this->expected_frame_size_ = 2 * this->header_footer_size_ + this->size_field_size_ + this->payload_size_;
        }
      }
      break;

    default:
      return RxEvaluationResult::NOK;  // unknown header type
  }

  if (this->expected_frame_size_ == 0 || this->end_pos_ + 1 < this->expected_frame_size_) {
    return RxEvaluationResult::UNKNOWN;  // not enough data yet to determine size

  } else if (this->end_pos_ + 1 > this->expected_frame_size_) {
#ifdef LD2410S_DEBUG_UART
    ESP_LOGE(TAG, "rx passed the expected frame end, expected:%d, current:%d", this->expected_frame_size_,
             this->end_pos_);
#endif
    return RxEvaluationResult::NOK;  // passed the end of short data frame

  } else {
    return RxEvaluationResult::OK;  // correct size
  }
}
// checks if current rx buffer containts proper footer for decoded header
RxEvaluationResult LD2410Srx::evaluate_footer_() {
  switch (this->frame_type_) {
    case RxFrameType::SHORT_DATA_FRAME:  // footer matches expected for short data frame
      if (memcmp(&rcv_buffer_[this->end_pos_ - this->header_footer_size_ + 1], &SHORT_DATA_FRAME_FOOTER,
                 sizeof(SHORT_DATA_FRAME_FOOTER)) == 0) {
        return RxEvaluationResult::OK;
      }
      break;

    case RxFrameType::STD_DATA_FRAME:  // footer matches expected for standard data frame
      if (memcmp(&rcv_buffer_[this->end_pos_ - this->header_footer_size_ + 1], &STD_DATA_FRAME_FOOTER,
                 sizeof(STD_DATA_FRAME_FOOTER)) == 0) {
        return RxEvaluationResult::OK;
      }
      break;

    case RxFrameType::CMD_FRAME:  // footer matches expected for command frame
      if (memcmp(&rcv_buffer_[this->end_pos_ - this->header_footer_size_ + 1], &CMD_FRAME_FOOTER,
                 sizeof(CMD_FRAME_FOOTER)) == 0) {
        return RxEvaluationResult::OK;
      }
      break;

    case RxFrameType::UNKNOWN:  // not enough data yet to determine size
      return RxEvaluationResult::UNKNOWN;

    case RxFrameType::NOK:  // already known bad data frame
    default:                // unknown header type
      return RxEvaluationResult::NOK;
  }
#ifdef LD2410S_DEBUG_UART
  ESP_LOGE(TAG, "rx footer does not match expected footer for frame type");
#endif
  return RxEvaluationResult::NOK;  // footer does not match expected footer for frame type
}
// reset rx buffer
void LD2410Srx::reset_() {
  this->end_pos_ = 0;
  this->header_footer_size_ = 0;
  this->size_field_size_ = 0;
  this->frame_type_ = RxFrameType::UNKNOWN;
  this->payload_ready_ = false;
  this->payload_pos_ = 0;
  this->payload_size_ = 0;
  this->expected_frame_size_ = 0;
}

}  // namespace ld2410s
}  // namespace esphome
