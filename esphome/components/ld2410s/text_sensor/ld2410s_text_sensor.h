#pragma once

#include "../ld2410s.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace ld2410s {
// class LD2410STextSensor : public LD2410SListener, public Component, text_sensor::TextSensor {
//  public:
//   void set_fw_version_text_sensor(text_sensor::TextSensor *tsensor) { this->fw_version_text_sensor_ = tsensor; };
//   void on_fw_version(std::string &val) override {
//     if (this->fw_version_text_sensor_ != nullptr) {
//       this->fw_version_text_sensor_->publish_state(val);
//     }
//   }

//   void set_threshold_trigger_text_sensor(text_sensor::TextSensor *tsensor) {
//     this->threshold_trigger_text_sensor_ = tsensor;
//   };
//   void on_threshold_trigger(std::string &val) override {
//     if (this->threshold_trigger_text_sensor_ != nullptr) {
//       this->threshold_trigger_text_sensor_->publish_state(val);
//     }
//   }

//   void set_threshold_hold_text_sensor(text_sensor::TextSensor *tsensor) {
//     this->threshold_hold_text_sensor_ = tsensor;
//   };
//   void on_threshold_hold(std::string &val) override {
//     if (this->threshold_hold_text_sensor_ != nullptr) {
//       this->threshold_hold_text_sensor_->publish_state(val);
//     }
//   }

//   void set_threshold_snr_text_sensor(text_sensor::TextSensor *tsensor) { this->threshold_snr_text_sensor_ = tsensor;
//   }; void on_threshold_snr(std::string &val) override {
//     if (this->threshold_snr_text_sensor_ != nullptr) {
//       this->threshold_snr_text_sensor_->publish_state(val);
//     }
//   }

//   void set_energy_values_text_sensor(text_sensor::TextSensor *tsensor) { this->energy_values_text_sensor_ = tsensor;
//   }; void on_energy_values(std::string &val) override {
//     if (this->energy_values_text_sensor_ != nullptr) {
//       this->energy_values_text_sensor_->publish_state(val);
//     }
//   }

//  private:
//   text_sensor::TextSensor *fw_version_text_sensor_{nullptr};
//   text_sensor::TextSensor *threshold_trigger_text_sensor_{nullptr};
//   text_sensor::TextSensor *threshold_hold_text_sensor_{nullptr};
//   text_sensor::TextSensor *threshold_snr_text_sensor_{nullptr};
//   text_sensor::TextSensor *energy_values_text_sensor_{nullptr};
// };

}  // namespace ld2410s
}  // namespace esphome
