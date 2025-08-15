#pragma once

#define LD2410S_V2

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
#include "ld2410s_help.h"
#include "ld2410s_rx.h"
#include "ld2410s_tx.h"

namespace esphome {
namespace ld2410s {

// using namespace ld24xx;

// Constants
static const uint16_t RX_MAX_BYTES_PER_LOOP = 500;

class LD2410S : public Component, public uart::UARTDevice, LD2410Shelp {
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
  LD2410S() : settings_(), tx_(settings_) {}

  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override;

  void calibration();
  void factory_reset();

#ifdef LD2410S_V2
  // number
  void set_delay(float delay);
  void set_distance_reporting_freq(float distance_reporting_freq);
  void set_max_distance(float max_distance);
  void set_min_distance(float min_distance);
  void set_status_reporting_freq(float status_reporting_freq);
  void set_threshold_hold(float threshold_hold);
  void set_threshold_selected_gate(float threshold_selected_gate);
  void set_threshold_snr(float threshold_snr);
  void set_threshold_trigger(float threshold_trigger);
  // select
  void set_response_speed_select(const std::string &response_speed_select);
  // switch
  void set_minimal_output(bool state);
#endif

 protected:
  LD2410Stx tx_;
  LD2410Srx rx_;

  SettingsT settings_;

  uint8_t init_status_{0};

  uint32_t energy_values_[16];
  std::string energy_values_str_ = "";

  void init_();

  void send_();
  bool receive_();

  void process_();
  void process_short_data_frame_();
  void process_data_frame_();
  void process_cmd_frame_();

  void publish_distance_(uint16_t distance, bool force_publish = false);
  void publish_calibration_progress_(uint16_t calibration_progress, bool force_publish = false);
  void publish_presence_(bool presence, bool force_publish = false);
  void publish_calibration_runing_(bool running, bool force_publish = false);

#ifdef LD2410S_V2
  void read_all_thresholds_();

  void process_ack_config_read_(uint8_t *data);
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
