#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/components/number/number.h"
#include "../aircore_gauge.h"

namespace esphome::aircore_gauge {

// Entite Home Assistant qui pilote la jauge dans son echelle de valeurs
class AircoreGaugeNumber final : public number::Number, public Component, public Parented<AircoreGauge> {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

 protected:
  void control(float value) override;
};

}  // namespace esphome::aircore_gauge
