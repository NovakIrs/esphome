#pragma once

#include "../ld2410s.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

namespace esphome {
namespace ld2410s {

// class LD2410SPresenceBinarySensor : public LD2410SListener, public Component, public binary_sensor::BinarySensor {
//  public:
//   void set_presence_sensor(binary_sensor::BinarySensor *bsensor) { this->presence_bsensor_ = bsensor; };
//   void on_presence(bool presence) override {
//     if (this->presence_bsensor_ != nullptr) {
//       if (this->presence_bsensor_->state != presence) {
//         this->presence_bsensor_->publish_state(presence);
//       }
//     }
//   };

//  private:
//   binary_sensor::BinarySensor *presence_bsensor_{nullptr};
// };

}  // namespace ld2410s
}  // namespace esphome
