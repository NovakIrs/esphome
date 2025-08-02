#pragma once

#include "esphome/components/button/button.h"
#include "../ld2410s.h"

namespace esphome {
namespace ld2410s {

class LD2410SReadAllButton : public button::Button, public Parented<LD2410S> {
 public:
  LD2410SReadAllButton() = default;

 protected:
  void press_action() override;
};

class LD2410SApplyConfigButton : public button::Button, public Parented<LD2410S> {
 public:
  LD2410SApplyConfigButton() = default;

 protected:
  void press_action() override;
};

class LD2410SCalibrationButton : public button::Button, public Parented<LD2410S> {
 public:
  LD2410SCalibrationButton() = default;

 protected:
  void press_action() override;
};

class LD2410SFactoryResetButton : public button::Button, public Parented<LD2410S> {
 public:
  LD2410SFactoryResetButton() = default;

 protected:
  void press_action() override;
};

class LD2410SMinimalOutputButton : public button::Button, public Parented<LD2410S> {
 public:
  LD2410SMinimalOutputButton() = default;

 protected:
  void press_action() override;
};

}  // namespace ld2410s
}  // namespace esphome
