#include "aircore_gauge_number.h"
#include "esphome/core/log.h"

namespace esphome::aircore_gauge {

static const char *const TAG = "aircore_gauge.number";

void AircoreGaugeNumber::setup() { this->publish_state(this->parent_->get_min_value()); }

void AircoreGaugeNumber::control(float value) {
  this->parent_->set_value(value);
  this->publish_state(value);
}

void AircoreGaugeNumber::dump_config() { LOG_NUMBER("", "Aircore Gauge Number", this); }

}  // namespace esphome::aircore_gauge
