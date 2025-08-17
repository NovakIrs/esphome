#include "ld2410s.h"

namespace esphome {
namespace ld2410s {

void LD2410Shelp::four_byte_to_int_array(uint8_t *in, uint32_t *out, uint8_t out_len) {
  for (uint8_t i = 0; i < out_len; i++) {
    out[i] = encode_uint32(in[i * 4 + 3], in[i * 4 + 2], in[i * 4 + 1], in[i * 4 + 0]);
  }
}
void LD2410Shelp::hex_diag(const char *msg, const uint8_t *data, size_t length) {
  char output[length * 3 + 1];

  for (size_t i = 0; i < length; i++) {
    if (i > 0) {  // Add a space before each byte except the first one
      sprintf(output + (i * 3 - 1), " ");
    }
    sprintf(output + (i * 3), "%02X", data[i]);
  }

  output[length * 3 - 1] = '\0';  // Null-terminate the string

  ESP_LOGD(TAG, "%s %s ", msg, output);
}
int LD2410Shelp::read_int(const uint8_t *buffer, size_t pos, size_t len) {
  unsigned int ret = 0;
  int shift = 0;
  for (size_t i = 0; i < len; i++) {
    ret |= static_cast<unsigned int>(buffer[pos + i]) << shift;
    shift += 8;
  }
  return ret;
};
#ifdef LD2410S_V2
std::string LD2410Shelp::format_int(uint32_t *in, uint8_t len, uint8_t min_w) {
  if (len == 0)
    return "";

  std::string result;
  int sum = 0;
  for (uint8_t i = 0; i < len; ++i) {
    sum += in[i];

    if (i > 0)
      result += ',';

    std::string num = std::to_string(in[i]);

    if (num.length() < min_w)
      result += std::string(min_w - num.length(), '0');

    result += num;
  }

  if (sum == 0) {
    result = "";
  }

  return result;
}
#endif
}  // namespace ld2410s
}  // namespace esphome
