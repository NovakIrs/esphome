
#include "ld2410s_rx.h"

namespace esphome {
namespace ld2410s {

EvaluationResult LD2410Srx::receive_one(int one) {
  if (this->payload_ready_) {
    this->reset_();
  }

  this->rcv_buffer_[this->end_pos_] = one;
  EvaluationResult result = this->evaluate_();

  switch (result) {
    case EvaluationResult::OK:
      this->payload_ready_ = true;
      this->hex_diag("<", &this->rcv_buffer_[0], this->end_pos_ + 1);
      break;

    case EvaluationResult::UNKNOWN:
      this->end_pos_++;
      if (this->end_pos_ > RCV_BUFFER_SIZE) {
        ESP_LOGD(TAG, "Received data buffer overflow, resetting");
        this->reset_();
      } else {
      }
      break;

    case EvaluationResult::NOK:
    default:
      this->reset_();
      result = EvaluationResult::UNKNOWN;
      break;
  }

  return result;
}

EvaluationResult LD2410Srx::evaluate_() {
  switch (this->evaluate_header_()) {
    case EvaluationResult::NOK:  // header does not match known frame type ie bad header
      // ESP_LOGD(TAG, "header does not match known frame type ie bad header: %d", this->end_pos_);
      return EvaluationResult::NOK;

    case EvaluationResult::UNKNOWN:  // not enough data yet to determine frame type
      return EvaluationResult::UNKNOWN;

    case EvaluationResult::OK:
    default:  // header ok, known type
      break;
  }

  switch (this->evaluate_size_()) {
    case EvaluationResult::NOK:  // known size, but greater then expected size for frame type
      this->hex_diag("<", &this->rcv_buffer_[0], this->end_pos_ + 1);
      ESP_LOGD(TAG, "correct header, but passed expected frame end: size:%d, expected:%d", this->end_pos_,
               this->expected_frame_size_);
      return EvaluationResult::NOK;

    case EvaluationResult::UNKNOWN:  // not enough data yet to determine correct size
      return EvaluationResult::UNKNOWN;

    case EvaluationResult::OK:  // correct size for frame type
    default:
      break;
  }

  switch (this->evaluate_footer_()) {
    case EvaluationResult::NOK:  // size matches expected size, but footer does not match expected footer for frame type
      this->hex_diag("<", &this->rcv_buffer_[0], this->end_pos_ + 1);
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
      return EvaluationResult::NOK;

    case EvaluationResult::UNKNOWN:  // size less then expected size for frame type
      return EvaluationResult::UNKNOWN;

    default:  // size matches expected size, footer matches expected footer for frame type
      break;
  }

  return EvaluationResult::OK;  // full frame received and verified
}

EvaluationResult LD2410Srx::evaluate_header_() {
  switch (this->frame_type_) {
    case RxFrameType::CMD_FRAME:
    case RxFrameType::STD_DATA_FRAME:
    case RxFrameType::SHORT_DATA_FRAME:
      return EvaluationResult::OK;  // already determined frame type

    case RxFrameType::NOK:
      return EvaluationResult::NOK;  // already determined bad header

    case RxFrameType::UNKNOWN:
    default:
      break;  // need to determine frame type
  }

  if (end_pos_ + 1 == sizeof(SHORT_DATA_FRAME_HEADER) &&
      memcmp(&rcv_buffer_[0], &SHORT_DATA_FRAME_HEADER, sizeof(SHORT_DATA_FRAME_HEADER)) == 0) {
    this->frame_type_ = RxFrameType::SHORT_DATA_FRAME;
    this->header_footer_size_ = sizeof(SHORT_DATA_FRAME_HEADER);
    return EvaluationResult::OK;
  }

  if (end_pos_ + 1 == sizeof(STD_DATA_FRAME_HEADER) &&
      memcmp(&rcv_buffer_[0], &STD_DATA_FRAME_HEADER, sizeof(STD_DATA_FRAME_HEADER)) == 0) {
    this->frame_type_ = RxFrameType::STD_DATA_FRAME;
    this->header_footer_size_ = sizeof(STD_DATA_FRAME_HEADER);
    return EvaluationResult::OK;
  }

  if (end_pos_ + 1 == sizeof(CMD_FRAME_HEADER) &&
      memcmp(&rcv_buffer_[0], &CMD_FRAME_HEADER, sizeof(CMD_FRAME_HEADER)) == 0) {
    this->frame_type_ = RxFrameType::CMD_FRAME;
    this->header_footer_size_ = sizeof(CMD_FRAME_HEADER);
    return EvaluationResult::OK;
  }

  if (end_pos_ + 1 < sizeof(STD_DATA_FRAME_HEADER) &&
      memcmp(&rcv_buffer_[0], &STD_DATA_FRAME_HEADER, end_pos_ + 1) == 0) {
    this->frame_type_ =
        RxFrameType::UNKNOWN;  // not enough data yet to determine frame type, but it fits STD frame header
    this->header_footer_size_ = 0;
    return EvaluationResult::UNKNOWN;
  }

  if (end_pos_ + 1 < sizeof(CMD_FRAME_HEADER) && memcmp(&rcv_buffer_[0], &CMD_FRAME_HEADER, end_pos_ + 1) == 0) {
    this->frame_type_ =
        RxFrameType::UNKNOWN;  // not enough data yet to determine frame type, but it fits CMD frame header
    this->header_footer_size_ = 0;
    return EvaluationResult::UNKNOWN;
  }

  this->frame_type_ = RxFrameType::NOK;  // bad header
  return EvaluationResult::NOK;
}

EvaluationResult LD2410Srx::evaluate_size_() {
  switch (this->frame_type_) {
    case RxFrameType::UNKNOWN:
      return EvaluationResult::UNKNOWN;  // not enough data yet to determine size

    case RxFrameType::NOK:
      return EvaluationResult::NOK;  // already determined bad header

    case RxFrameType::SHORT_DATA_FRAME:
      if (this->expected_frame_size_ == 0) {
        this->size_field_size_ = 0;
        this->payload_size_ = 3;
        this->payload_pos_ = this->header_footer_size_;
        this->expected_frame_size_ = 2 * this->header_footer_size_ + 3;
      }
      break;

    case RxFrameType::STD_DATA_FRAME:
      if (this->expected_frame_size_ == 0) {
        this->size_field_size_ = 2;  // size field is 2 bytes
        if (this->end_pos_ >= this->header_footer_size_ + this->size_field_size_) {
          this->payload_size_ = read_int(this->rcv_buffer_, this->header_footer_size_, 2);
          this->payload_pos_ = this->header_footer_size_ + +this->size_field_size_;
          this->expected_frame_size_ = 2 * this->header_footer_size_ + this->size_field_size_ + this->payload_size_;
        }
      }
      break;

    case RxFrameType::CMD_FRAME:
      if (this->expected_frame_size_ == 0) {
        this->size_field_size_ = 2;  // size field is 2 bytes
        if (this->end_pos_ >= this->header_footer_size_ + this->size_field_size_) {
          this->payload_size_ = read_int(this->rcv_buffer_, this->header_footer_size_, 2);
          this->payload_pos_ = this->header_footer_size_ + +this->size_field_size_;
          this->expected_frame_size_ = 2 * this->header_footer_size_ + this->size_field_size_ + this->payload_size_;
        }
      }
      break;

    default:
      return EvaluationResult::NOK;  // unknown header type
  }

  if (this->expected_frame_size_ == 0) {
    return EvaluationResult::UNKNOWN;  // not enough data yet to determine size

  } else if (this->expected_frame_size_ > this->end_pos_ + 1) {
    return EvaluationResult::UNKNOWN;  // not enough data yet to determine size

  } else if (this->expected_frame_size_ < this->end_pos_ + 1) {
    return EvaluationResult::NOK;  // passed the end of short data frame

  } else {
    return EvaluationResult::OK;  // correct size
  }
}

EvaluationResult LD2410Srx::evaluate_footer_() {
  switch (this->frame_type_) {
    case RxFrameType::SHORT_DATA_FRAME:
      if (memcmp(&rcv_buffer_[this->end_pos_ - this->header_footer_size_ + 1], &SHORT_DATA_FRAME_FOOTER,
                 sizeof(SHORT_DATA_FRAME_FOOTER)) == 0) {
        return EvaluationResult::OK;  // footer matches expected footer for short data frame
      }
      break;

    case RxFrameType::STD_DATA_FRAME:
      if (memcmp(&rcv_buffer_[this->end_pos_ - this->header_footer_size_ + 1], &STD_DATA_FRAME_FOOTER,
                 sizeof(STD_DATA_FRAME_FOOTER)) == 0) {
        return EvaluationResult::OK;  // footer matches expected footer for short data frame
      }
      break;

    case RxFrameType::CMD_FRAME:
      if (memcmp(&rcv_buffer_[this->end_pos_ - this->header_footer_size_ + 1], &CMD_FRAME_FOOTER,
                 sizeof(CMD_FRAME_FOOTER)) == 0) {
        return EvaluationResult::OK;  // footer matches expected footer for short data frame
      }
      break;

    case RxFrameType::UNKNOWN:
      return EvaluationResult::UNKNOWN;  // not enough data yet to determine size

    case RxFrameType::NOK:
    default:
      return EvaluationResult::NOK;  // unknown header type
  }

  return EvaluationResult::NOK;  // footer does not match expected footer for frame type
}

void LD2410Srx::reset_() {
  // ESP_LOGD(TAG, "rx reset, frame: %d", this->end_pos_);
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
