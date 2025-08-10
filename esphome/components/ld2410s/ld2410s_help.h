#pragma once

#include "esphome/core/application.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

#include "esphome/components/uart/uart.h"

#include "ld2410s_const.h"

namespace esphome {
namespace ld2410s {

class LD2410Shelp {
 public:
 protected:
  static void four_byte_to_int_array(uint8_t *in, uint32_t *out, uint8_t out_len);
  static void hex_diag(const char *msg, const uint8_t *data, size_t length);
  static int read_int(const uint8_t *buffer, size_t pos, size_t len);
};

}  // namespace ld2410s
}  // namespace esphome
