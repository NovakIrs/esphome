
#include "ld2410s_rx.h"

namespace esphome {
namespace ld2410s {

bool LD2410Srx::receive() {
  if (!this->available()) {
    return false;
  }

  if (this->frame_type_ != RxFrameType::UNKNOWN) {
    this->reset_();
  }

  while (this->available()) {
    this->rcv_buffer_[this->end_pos_] = this->read();

    // switch (this->evaluate_()) {
    //   case EvaluationResult::OK:
    //     ESP_LOGD(TAG, "Received correct frame: %d", this->end_pos_);
    //     return true;

    //   case EvaluationResult::NOK:
    //     ESP_LOGD(TAG, "Error evaluating received frame: %d", this->end_pos_);
    //     this->reset_();
    //     this->frame_type_ != RxFrameType::NOK;
    //     return false;

    //   case EvaluationResult::UNKNOWN:
    //   default:
    //     break;
    // }

    this->end_pos_++;
    if (this->end_pos_ > RCV_BUFFER_SIZE) {
      ESP_LOGD(TAG, "Received data buffer overflow, resetting");
      this->reset_();
    }
  }

  return false;
}

EvaluationResult LD2410Srx::evaluate_() {
  switch (this->evaluate_header_()) {
    case EvaluationResult::NOK:  // header does not match known frame type ie bad header
      ESP_LOGD(TAG, "header does not match known frame type ie bad header: %d", this->end_pos_);
      return EvaluationResult::NOK;

    case EvaluationResult::UNKNOWN:  // not enough data yet to determine frame type
      return EvaluationResult::UNKNOWN;

    case EvaluationResult::OK:
    default:  // header ok, known type
      break;
  }

  switch (this->evaluate_size_()) {
    case EvaluationResult::NOK:  // known size, but greater then expected size for frame type
      ESP_LOGD(TAG, "correct header, but passed expected frame end: %d", this->end_pos_);
      return EvaluationResult::NOK;

    case EvaluationResult::UNKNOWN:  // not enough data yet to determine correct size
      return EvaluationResult::UNKNOWN;

    case EvaluationResult::OK:  // correct size for frame type
    default:
      break;
  }

  switch (this->evaluate_footer_()) {
    case EvaluationResult::NOK:  // size matches expected size, but footer does not match expected footer for frame type
      ESP_LOGD(TAG, "correct header and size, but footer does not match expected: %d", this->end_pos_);
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
    ;
    return EvaluationResult::OK;
  }

  if (end_pos_ + 1 == sizeof(STD_DATA_FRAME_HEADER) &&
      memcmp(&rcv_buffer_[0], &STD_DATA_FRAME_HEADER, sizeof(STD_DATA_FRAME_HEADER)) == 0) {
    this->frame_type_ = RxFrameType::SHORT_DATA_FRAME;
    this->header_footer_size_ = sizeof(STD_DATA_FRAME_HEADER);
    ;
    return EvaluationResult::OK;
  }

  if (end_pos_ + 1 == sizeof(CMD_FRAME_HEADER) &&
      memcmp(&rcv_buffer_[0], &CMD_FRAME_HEADER, sizeof(CMD_FRAME_HEADER)) == 0) {
    this->frame_type_ = RxFrameType::CMD_FRAME;
    this->header_footer_size_ = sizeof(CMD_FRAME_HEADER);
    ;
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
    case RxFrameType::CMD_FRAME:
      if (this->expected_frame_size_ == 0) {
        this->size_field_size_ = 2;  // size field is 2 bytes
        if (this->end_pos_ >= this->header_footer_size_ + this->size_field_size_) {
          this->payload_size_ = read_int(this->rcv_buffer_, this->header_footer_size_, 2);
          //          this->payload_size_ = encode_uint16(this->rcv_buffer_[this->header_footer_size_ + 1],
          //          this->rcv_buffer_[this->header_footer_size_]);;
          this->payload_pos_ = this->header_footer_size_ + +this->size_field_size_;
          this->expected_frame_size_ = 2 * this->header_footer_size_ + this->size_field_size_ + this->payload_size_;
        }
      }
      break;

    default:
      return EvaluationResult::NOK;  // unknown header type
  }

  if (this->expected_frame_size_ > this->end_pos_ + 1) {
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
  this->end_pos_ = 0;
  this->header_footer_size_ = 0;
  this->size_field_size_ = 0;
  this->frame_type_ = RxFrameType::UNKNOWN;
  this->payload_pos_ = 0;
  this->payload_size_ = 0;
  this->expected_frame_size_ = 0;
}

// bool LD2410Srx::receive_x() {
//   if (!this->available()) {
//     return false;
//   }

//   if (this->frame_ok_ == EvaluationResult::OK) {
//     this->reset_();
//   }

//   while (this->available()) {
//     this->rcv_buffer_[this->end_pos_] = this->read();

//     switch (this->evaluate_()) {
//       case EvaluationResult::OK:
//         this->received_ok = true;
//         this->frame_ok_ = EvaluationResult::OK;
//         return true;

//       case EvaluationResult::NOK:
//         ESP_LOGD(TAG, "Error evaluating received data %d", this->rcv_end_pos_);
//         this->reset_();

//       case else:
//         break;
//     }

//     FrameType type = this->get_frame_type_(this->rcv_buffer_, this->rcv_end_pos_);
//     // FrameType based on frame footer.

//     size_t start_pos = this->get_frame_start_(this->rcv_buffer_, this->rcv_end_pos_, type);
//     // Frame start position based on frame header search, starting from the frame end.
//     if (start_pos == this->rcv_end_pos_) {
//       type = FrameType::UNKNOWN;
//     }

//     size_t payload_size = this->get_payload_size_(this->rcv_buffer_, this->rcv_end_pos_, type, start_pos);
//     // Payload size = frame size - header - footer
//     if (payload_size == 0) {
//       type = FrameType::UNKNOWN;
//     }

//     if (type != FrameType::UNKNOWN) {
//       esphome::ld2410s::LD2410S::hex_diag("<", &this->rcv_buffer_[start_pos], this->rcv_end_pos_ + 1 - start_pos);
//       if (start_pos > 0) {
//         ESP_LOGW(TAG, "Frame starting at %x", start_pos);
//       }

//       // ToDo
//       return true;

//       this->rcv_end_pos_ = 0;

//     } else {
//       this->rcv_end_pos_++;

//       if (this->rcv_end_pos_ >= RCV_BUFFER_SIZE - 1) {
//         this->rcv_end_pos_ = 0;
//         ESP_LOGW(TAG, "Buffer overflow, resetting rcv_end_pos_ to 0");
//       }
//     }
//   }
// }
// FrameType LD2410Srx::get_frame_type_(uint8_t *buffer, size_t end_pos) {
//   if (end_pos < 4) {
//     return FrameType::UNKNOWN;
//   }
//   if (buffer[end_pos] == SHORT_DATA_FRAME_FOOTER && buffer[end_pos - 4] == SHORT_DATA_FRAME_HEADER) {
//     return FrameType::SHORT_DATA_FRAME;
//   }
//   if (end_pos < 12) {
//     return FrameType::UNKNOWN;
//   }
//   if (memcmp(&buffer[end_pos - 3], &STD_DATA_FRAME_FOOTER, sizeof(STD_DATA_FRAME_FOOTER)) == 0) {
//     return FrameType::STD_DATA_FRAME;
//   }
//   if (memcmp(&buffer[end_pos - 3], &CMD_FRAME_FOOTER, sizeof(CMD_FRAME_FOOTER)) == 0) {
//     return FrameType::CMD_FRAME;
//   }
//   return FrameType::UNKNOWN;
// }
// size_t LD2410Srx::get_frame_start_(uint8_t *buffer, size_t end_pos, FrameType type) {
//   if (type == FrameType::UNKNOWN) {
//     return end_pos;
//   }

//   size_t min_length = 0;
//   uint32_t header_frame = 0;
//   int header_frame_len = 0;

//   switch (type) {
//     case FrameType::SHORT_DATA_FRAME:
//       min_length = 4;
//       header_frame = SHORT_DATA_FRAME_HEADER;
//       header_frame_len = sizeof(SHORT_DATA_FRAME_HEADER);
//       break;

//     case FrameType::STD_DATA_FRAME:
//       min_length = 12;
//       header_frame = STD_DATA_FRAME_HEADER;
//       header_frame_len = sizeof(STD_DATA_FRAME_HEADER);
//       break;

//     case FrameType::CMD_FRAME:
//       min_length = 12;
//       header_frame = CMD_FRAME_HEADER;
//       header_frame_len = sizeof(CMD_FRAME_HEADER);
//       break;

//     default:
//       return end_pos;
//       break;
//   }

//   if (end_pos + 1 < min_length) {
//     return end_pos;
//   }

//   for (uint8_t i = end_pos - min_length; i >= 0; i--) {
//     if (header_frame == esphome::ld2410s::LD2410S::read_int(buffer, i, header_frame_len)) {
//       return i;
//     }
//   }

//   return end_pos;
// }
// size_t LD2410Srx::get_payload_size_(uint8_t *buffer, size_t end_pos, FrameType type, size_t start_pos) {
//   if (type == FrameType::UNKNOWN || end_pos == start_pos) {
//     return 0;
//   }

//   size_t payload_size = 0;
//   size_t expected_full_frame_size = 0;

//   switch (type) {
//     case FrameType::SHORT_DATA_FRAME:
//       payload_size = 3;
//       expected_full_frame_size = 1 + payload_size + 1;
//       break;

//     case FrameType::STD_DATA_FRAME:
//     case FrameType::CMD_FRAME:
//       payload_size = esphome::ld2410s::LD2410S::read_int(buffer, start_pos + 4, 2);
//       expected_full_frame_size = 4 + 2 + payload_size + 4;
//       break;

//     default:
//       break;
//   }

//   if (payload_size == 0) {
//     return 0;
//   }

//   if (expected_full_frame_size != end_pos - start_pos + 1) {
//     return 0;
//   }

//   return payload_size;
// }

}  // namespace ld2410s
}  // namespace esphome
