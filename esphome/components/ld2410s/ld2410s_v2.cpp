#include "ld2410s.h"

namespace esphome {
namespace ld2410s {

int LD2410S::read_int(const uint8_t *buffer, size_t pos, size_t len) {
  unsigned int ret = 0;
  int shift = 0;
  for (size_t i = 0; i < len; i++) {
    ret |= static_cast<unsigned int>(buffer[pos + i]) << shift;
    shift += 8;
  }
  return ret;
};

}  // namespace ld2410s
}  // namespace esphome
