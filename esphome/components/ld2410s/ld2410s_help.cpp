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
  // // append variable sized append_data to data, returns true if not overflow
  // template<typename T>
  // bool LD2410Shelp::append_seq_data_(uint8_t *data, uint16_t &insert_position, const T *append_data,
  //                                    uint16_t append_array_size, uint16_t actual_size) {
  //   size_t data_object_size = actual_size;
  //   if (data_object_size == 0) {
  //     data_object_size = sizeof(T);
  //   }
  //   auto bytes_to_copy = append_array_size * data_object_size;
  //   if (insert_position + bytes_to_copy > RX_TX_BUFFER_SIZE) {
  //     ESP_LOGE(TAG, "append_seq_data_ overflow: insert_position:%d + append_array_size:%d + object_size:%d > %d",
  //              insert_position, append_array_size, data_object_size, RX_TX_BUFFER_SIZE);
  //     return false;
  //   }

  //   auto write_ptr = &data[0] + insert_position;
  //   memcpy(write_ptr, append_data, bytes_to_copy);

  //   insert_position += bytes_to_copy;

  //   return true;
  // }
  // // read variable sized uint from data and move read_position
  // template<typename T>
  // bool LD2410Shelp::read_seq_data_(const uint8_t *data, uint16_t &read_position, T *out_data, uint16_t
  // out_array_size,
  //                                  uint16_t actual_size) {
  //   size_t data_object_size = (actual_size == 0 ? sizeof(T) : actual_size);
  //   size_t bytes_to_read = out_array_size * data_object_size;

  //   if (read_position + bytes_to_read > RX_TX_BUFFER_SIZE) {
  //     ESP_LOGE(TAG, "read_seq_data_ overflow: read_position:%d + bytes_to_read:%d > %d", read_position,
  //     bytes_to_read,
  //              RX_TX_BUFFER_SIZE);
  //     return false;
  //   }

  //   const uint8_t *read_ptr = &data[0] + read_position;

  //   memcpy(out_data, read_ptr, bytes_to_read);

  //   read_position += bytes_to_read;
  //   return true;
  // }

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
