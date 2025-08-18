#include "ld2410s.h"

namespace esphome {
namespace ld2410s {

void LD2410Sdc::receive_byte(uint8_t byte) {
  this->rcv_buffer_[this->end_pos_] = byte;

  this->end_pos_++;
  if (this->end_pos_ >= DC_BUFFER_SIZE) {
    this->flush();
  }
}
void LD2410Sdc::flush() {
  const char msg[] = "<<<";
  hex_diag(msg, this->rcv_buffer_, this->end_pos_);
  this->end_pos_ = 0;
}
}  // namespace ld2410s
}  // namespace esphome
