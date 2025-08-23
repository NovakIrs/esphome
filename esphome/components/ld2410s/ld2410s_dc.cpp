#include "ld2410s.h"

namespace esphome {
namespace ld2410s {

#ifdef LD2410S_DEBUG_UART
void LD2410Sdc::receive_byte(uint8_t byte) {
  this->rcv_buffer_[this->end_pos_] = byte;

  this->end_pos_++;
  if (this->end_pos_ >= RX_DC_BUFFER_SIZE) {
    this->flush();
  }
}
void LD2410Sdc::flush() {
  if (this->end_pos_ > 0) {
    ESP_LOGI(TAG, "<<< %s", format_hex_pretty(this->rcv_buffer_, end_pos_ - 1, ' ').c_str());
    this->end_pos_ = 0;
  }
}
#endif
}  // namespace ld2410s
}  // namespace esphome
