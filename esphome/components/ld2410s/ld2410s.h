#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/core/defines.h"
#include "esphome/core/automation.h"

#include "esphome/components/uart/uart.h"
// #include "esphome/components/ld24xx/ld24xx.h"

#ifdef USE_NUMBER
#include "esphome/components/number/number.h"
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
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif
#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif

#include <functional>
#include <iomanip>

namespace esphome {
namespace ld2410s {

// using namespace ld24xx;

// Constants
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

class LD2410SListener {
 public:
  virtual void on_presence(bool presence){};
  virtual void on_distance(uint16_t distance){};
  virtual void on_calibration_update(bool running){};
  virtual void on_calibration_progress(uint16_t progress){};
  virtual void on_fw_version(std::string &val){};
  virtual void on_threshold_trigger(std::string &val){};
  virtual void on_threshold_hold(std::string &val){};
  virtual void on_threshold_snr(std::string &val){};
  virtual void on_energy_values(std::string &val){};
};

class LD2410S : public Component, public uart::UARTDevice {
#ifdef USE_BINARY_SENSOR
  SUB_BINARY_SENSOR(presence)
  SUB_BINARY_SENSOR(calibration_update)
#endif
#ifdef USE_BUTTON
  SUB_BUTTON(calibration)
  SUB_BUTTON(factory_reset)
#endif
#ifdef USE_SWITCH
  SUB_SWITCH(minimal_output)
#endif

 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override;

  void register_listener(LD2410SListener *listener) { this->listeners_.push_back(listener); };

  void calibration();
  void factory_reset();

  void set_minimal_output(bool state);
  void set_delay(float delay);
  void set_distance_reporting_freq(float distance_reporting_freq);
  void set_max_distance(float max_distance);
  void set_min_distance(float min_distance);
  void set_status_reporting_freq(float status_reporting_freq);
  void set_threshold_selected_gate(float threshold_selected_gate);
  void set_threshold_trigger(float threshold_trigger);
  void set_threshold_hold(float threshold_hold);
  void set_threshold_snr(float threshold_snr);
  void set_response_speed_select(const std::string &response_speed_select);

#ifdef USE_NUMBER
  void set_max_distance_number(number::Number *max_distance_number) {
    this->max_distance_number_ = max_distance_number;
  };
  void set_min_distance_number(number::Number *min_distance_number) {
    this->min_distance_number_ = min_distance_number;
  };
  void set_no_delay_number(number::Number *delay_number) { this->no_delay_number_ = delay_number; };
  void set_status_reporting_freq_number(number::Number *status_reporting_freq_number) {
    this->status_reporting_freq_number_ = status_reporting_freq_number;
  };
  void set_distance_reporting_freq_number(number::Number *distance_reporting_freq_number) {
    this->distance_reporting_freq_number_ = distance_reporting_freq_number;
  };

  void set_threshold_trigger_number(number::Number *threshold_trigger_number) {
    this->threshold_trigger_number_ = threshold_trigger_number;
  };
  void set_threshold_hold_number(number::Number *threshold_hold_number) {
    this->threshold_hold_number_ = threshold_hold_number;
  };
  void set_threshold_snr_number(number::Number *threshold_snr_number) {
    this->threshold_snr_number_ = threshold_snr_number;
  };
  void set_threshold_selected_gate_number(number::Number *threshold_selected_gate_number) {
    this->threshold_selected_gate_number_ = threshold_selected_gate_number;
    this->threshold_selected_gate_number_->publish_state(this->thresholds_.selected_gate);
    this->threshold_trigger_number_->publish_state(this->thresholds_.trigger[this->thresholds_.selected_gate]);
    this->threshold_hold_number_->publish_state(this->thresholds_.hold[this->thresholds_.selected_gate]);
    this->threshold_snr_number_->publish_state(this->thresholds_.snr[this->thresholds_.selected_gate]);
  };
#endif

#ifdef USE_SELECT
  void set_response_speed_select(select::Select *selector) { this->response_speed_select_ = selector; };
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
  std::vector<LD2410SListener *> listeners_{};
  CmdT commands_[CMD_EXEC_BUFFER_SIZE];
  ThresholdsT thresholds_;

  void init_();
  void read_all_thresholds_();
  void loop_send_command_();
  void cmd_buffer_insert_(CmdFrameT *cmd_frame);
  void cmd_buffer_finished_();
  void cmd_buffer_inc_(uint8_t &index);

#ifdef USE_NUMBER
  number::Number *max_distance_number_{nullptr};
  number::Number *min_distance_number_{nullptr};
  number::Number *no_delay_number_{nullptr};
  number::Number *status_reporting_freq_number_{nullptr};
  number::Number *distance_reporting_freq_number_{nullptr};

  number::Number *threshold_trigger_number_{nullptr};
  number::Number *threshold_hold_number_{nullptr};
  number::Number *threshold_snr_number_{nullptr};
  number::Number *threshold_selected_gate_number_{nullptr};
#endif

#ifdef USE_SELECT
  select::Select *response_speed_select_{nullptr};
#endif

  void receive_();
  PackageType get_frame_type_(uint8_t *buffer, size_t pos);
  size_t get_frame_start_(uint8_t *buffer, size_t end_pos, PackageType type);
  size_t get_data_size_(uint8_t *buffer, size_t end_pos, PackageType type, size_t start_pos);

  void process_short_data_frame_(uint8_t *data);
  void process_data_frame_(uint8_t *data);
  void process_cmd_frame_(uint8_t *buffer, size_t len);

  CmdAckT parse_cms_frame_(uint8_t *buffer, size_t length);

  void schedule_cmd_(const char *msg, uint16_t command, uint16_t sub_command = NO_SUB_CMD);
  void schedule_cmd_frame_(uint16_t command, uint16_t sub_command = NO_SUB_CMD);
  void cmd_frame_append_data_(CmdFrameT *cmd_frame, const uint8_t *append_data, size_t append_data_size);
  void cmd_frame_append_data_(CmdFrameT *cmd_frame, const uint16_t *append_data, size_t append_data_size);
  void cmd_frame_append_data_(CmdFrameT *cmd_frame, const uint32_t *append_data, size_t append_data_size);

  void send_command_(CmdFrameT *cmd_frame);

  void process_ack_config_read_(uint8_t *data);
  void process_ack_fw_read_(const uint8_t *data);
  void process_ack_threshold_trigger_read_(uint8_t *data);
  void process_ack_threshold_hold_read_(uint8_t *data);
  void process_ack_threshold_snr_read_(uint8_t *data);
  void process_ack_minimal_output_(uint8_t *data);

  void process_data_energy_values_read_(uint8_t *data);

  void publish_state_ts_thresholds_();
  void publish_state_ts_holds_();
  void publish_state_ts_snrs_();
  void publish_state_ts_energy_values_();

  static std::string format_int(uint32_t *in, uint8_t len, uint8_t min_w);
  static void four_byte_to_int_array(uint8_t *in, uint32_t *out, uint8_t out_len);
  static void hex_diag(const char *msg, const uint8_t *data, size_t length);
  static int read_int(const uint8_t *buffer, size_t pos, size_t len);
};

}  // namespace ld2410s
}  // namespace esphome
