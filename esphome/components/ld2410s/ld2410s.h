#pragma once

#define LD2410S_V2x

#include "esphome/core/application.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

#include "esphome/components/uart/uart.h"
// #include "esphome/components/ld24xx/ld24xx.h"

#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif
#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif
#ifdef USE_BUTTON
#include "esphome/components/button/button.h"
#endif
#ifdef USE_SWITCH
#include "esphome/components/switch/switch.h"
#endif
#ifdef USE_SELECT
#include "esphome/components/select/select.h"
#endif
#ifdef USE_NUMBER
#include "esphome/components/number/number.h"
#endif

#include <functional>
#include <iomanip>

#include "ld2410s_const.h"

namespace esphome {
namespace ld2410s {

// using namespace ld24xx;

// Constants

static const char *const TAG = "ld2410s";

static const uint16_t NO_SUB_CMD = 0xffff;
static const uint8_t CMD_EXEC_BUFFER_SIZE = 32;
static const size_t RCV_BUFFER_SIZE = 128;

struct ThresholdsT {
  uint32_t trigger[16];
  uint32_t hold[16];
  uint32_t snr[16];
  uint8_t selected_gate{0};
};

struct CmdFrameT {
  uint8_t data[128];
  uint32_t header;
  uint32_t footer;
  uint16_t length;
  uint16_t command;
  uint16_t data_length;
};

struct CmdAckT {
  uint8_t data[128];
  uint16_t command{0};
  uint16_t length{0};
  bool result{false};
};
enum class PackageType { SHORT_DATA_FRAME, STD_DATA_FRAME, CMD_FRAME, UNKNOWN, BAD_SIZE };

enum class CmdState { EMPTY, SCHEDULED, SENT };
struct CmdT {
  uint32_t time_started;
  uint8_t retry;
  CmdState state = CmdState::EMPTY;
  CmdFrameT *cmd_frame;
};

class LD2410S : public Component, public uart::UARTDevice {
#ifdef USE_SENSOR
  SUB_SENSOR(calibration_progress)
  SUB_SENSOR(distance)
#endif
#ifdef USE_BINARY_SENSOR
  SUB_BINARY_SENSOR(presence)
  SUB_BINARY_SENSOR(calibration_runing)
#endif
#ifdef USE_TEXT_SENSOR
  SUB_TEXT_SENSOR(fw_version)
  SUB_TEXT_SENSOR(threshold_trigger)
  SUB_TEXT_SENSOR(threshold_hold)
  SUB_TEXT_SENSOR(threshold_snr)
  SUB_TEXT_SENSOR(energy_values)
#endif
#ifdef USE_BUTTON
  SUB_BUTTON(calibration)
  SUB_BUTTON(factory_reset)
#endif
#ifdef USE_SWITCH
  SUB_SWITCH(minimal_output)
#endif
#ifdef USE_SELECT
  SUB_SELECT(response_speed)
#endif
#ifdef USE_NUMBER
  SUB_NUMBER(max_distance)
  SUB_NUMBER(min_distance)
  SUB_NUMBER(no_delay)
  SUB_NUMBER(status_reporting_freq)
  SUB_NUMBER(distance_reporting_freq)
  SUB_NUMBER(threshold_trigger)
  SUB_NUMBER(threshold_hold)
  SUB_NUMBER(threshold_snr)
  SUB_NUMBER(threshold_selected_gate)
#endif

 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override;

  void set_delay(float delay);
  void set_distance_reporting_freq(float distance_reporting_freq);
  void set_max_distance(float max_distance);
  void set_min_distance(float min_distance);
  void set_status_reporting_freq(float status_reporting_freq);
  void set_response_speed_select(const std::string &response_speed_select);

  void calibration();
  void factory_reset();

#ifdef LD2410S_V2
  void read_all_thresholds_();

  void set_minimal_output(bool state);
  void set_threshold_selected_gate(float threshold_selected_gate);
  void set_threshold_trigger(float threshold_trigger);
  void set_threshold_hold(float threshold_hold);
  void set_threshold_snr(float threshold_snr);
#endif

 protected:
  size_t rcv_end_pos_ = 0;
  uint32_t max_dist_{0};
  uint32_t min_dist_{0};
  uint32_t delay_{0};
  uint32_t status_freq_{0};
  uint32_t dist_freq_{0};
  uint32_t resp_speed_{0};
  uint32_t energy_values_[16];
  uint8_t rcv_buffer_[RCV_BUFFER_SIZE];
  uint8_t active_ = 0;
  uint8_t last_ = 0;
  uint8_t init_status_ = 0;
  bool minimal_output_{true};
  bool cmd_active_{false};

  std::string energy_values_str_ = "";
  CmdT commands_[CMD_EXEC_BUFFER_SIZE];
  ThresholdsT thresholds_;

  void init_();

  void schedule_cmd_(const char *msg, uint16_t command, uint16_t sub_command = NO_SUB_CMD);
  void schedule_cmd_frame_(uint16_t command, uint16_t sub_command = NO_SUB_CMD);
  void cmd_frame_append_data_(CmdFrameT *cmd_frame, const uint8_t *append_data, size_t append_data_size);
  void cmd_frame_append_data_(CmdFrameT *cmd_frame, const uint16_t *append_data, size_t append_data_size);
  void cmd_frame_append_data_(CmdFrameT *cmd_frame, const uint32_t *append_data, size_t append_data_size);

  void cmd_buffer_insert_(CmdFrameT *cmd_frame);
  void cmd_buffer_finished_();
  void cmd_buffer_inc_(uint8_t &index);

  void loop_send_command_();
  void send_command_(CmdFrameT *cmd_frame);

  void receive_();
  PackageType get_frame_type_(uint8_t *buffer, size_t pos);
  size_t get_frame_start_(uint8_t *buffer, size_t end_pos, PackageType type);
  size_t get_payload_size_(uint8_t *buffer, size_t end_pos, PackageType type, size_t start_pos);

  void process_short_data_frame_(uint8_t *data);
  void process_data_frame_(uint8_t *data, size_t data_size);
  void process_cmd_frame_(uint8_t *buffer, size_t len);
  CmdAckT parse_cms_frame_(uint8_t *buffer, size_t length);

  void process_ack_config_read_(uint8_t *data);

  void publish_distance_(uint16_t distance, bool force_publish = false);
  void publish_calibration_progress_(uint16_t calibration_progress, bool force_publish = false);
  void publish_presence_(bool presence, bool force_publish = false);
  void publish_calibration_runing_(bool running, bool force_publish = false);

  static void four_byte_to_int_array(uint8_t *in, uint32_t *out, uint8_t out_len);
  static void hex_diag(const char *msg, const uint8_t *data, size_t length);
  static int read_int(const uint8_t *buffer, size_t pos, size_t len);

#ifdef LD2410S_V2
  void process_ack_fw_read_(const uint8_t *data);
  void process_ack_threshold_trigger_read_(uint8_t *data);
  void process_ack_threshold_hold_read_(uint8_t *data);
  void process_ack_threshold_snr_read_(uint8_t *data);
  void process_ack_minimal_output_(uint8_t *data);

  void process_data_energy_values_read_(uint8_t *data);

  void publish_fw_version_(const std::string &version, bool force_publish = false);

  void publish_threshold_trigger_(bool force_publish = false);
  void publish_threshold_hold_(bool force_publish = false);
  void publish_threshold_snr_(bool force_publish = false);

  void publish_energy_values_(bool force_publish = false);

  static std::string format_int(uint32_t *in, uint8_t len, uint8_t min_w);
#endif

#ifdef LD2410S_V2
#endif
};

}  // namespace ld2410s
}  // namespace esphome
