#include "aircore_gauge.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

#include <cmath>

namespace esphome::aircore_gauge {

static const char *const TAG = "aircore_gauge";

void AircoreGauge::set_four_wire(output::FloatOutput *cos_pos, output::FloatOutput *cos_neg,
                                 output::FloatOutput *sin_pos, output::FloatOutput *sin_neg) {
  this->three_wire_ = false;
  this->cos_pos_ = cos_pos;
  this->cos_neg_ = cos_neg;
  this->sin_pos_ = sin_pos;
  this->sin_neg_ = sin_neg;
}

void AircoreGauge::set_three_wire(output::FloatOutput *cos, output::FloatOutput *sin, output::FloatOutput *common) {
  this->three_wire_ = true;
  this->cos_pos_ = cos;
  this->sin_pos_ = sin;
  this->common_ = common;
}

void AircoreGauge::setup() {
  this->target_angle_ = this->current_angle_ = this->min_angle_;
  this->write_(this->current_angle_);
  this->last_ms_ = millis();
#ifdef USE_SENSOR
  if (this->sensor_ != nullptr) {
    this->sensor_->add_on_state_callback([this](float state) {
      if (!std::isnan(state))
        this->set_value(state);
    });
  }
#endif
}

void AircoreGauge::set_value(float value) {
  float lo = std::min(this->min_value_, this->max_value_);
  float hi = std::max(this->min_value_, this->max_value_);
  float t = (clamp(value, lo, hi) - this->min_value_) / (this->max_value_ - this->min_value_);
  this->set_angle(this->min_angle_ + t * (this->max_angle_ - this->min_angle_));
}

void AircoreGauge::set_angle(float angle) {
  float lo = std::min(this->min_angle_, this->max_angle_);
  float hi = std::max(this->min_angle_, this->max_angle_);
  this->target_angle_ = clamp(angle, lo, hi);
}

// Deplacement progressif : un saut de plus de 180 deg ferait tourner l'aiguille par le mauvais cote
void AircoreGauge::loop() {
  uint32_t now = millis();
  float dt = (now - this->last_ms_) / 1000.0f;
  this->last_ms_ = now;

  float diff = this->target_angle_ - this->current_angle_;
  if (diff == 0.0f)
    return;

  float step = this->speed_ * dt;
  if (this->speed_ <= 0.0f || std::fabs(diff) <= step) {
    this->current_angle_ = this->target_angle_;
  } else {
    this->current_angle_ += std::copysign(step, diff);
  }
  this->write_(this->current_angle_);
}

void AircoreGauge::write_(float angle) {
  float rad = (angle + this->zero_offset_) * static_cast<float>(M_PI) / 180.0f;
  float c = std::cos(rad);
  float s = std::sin(rad);

  if (this->three_wire_) {
    this->common_->set_level(0.5f);
    this->cos_pos_->set_level(0.5f + 0.5f * c * this->max_power_);
    this->sin_pos_->set_level(0.5f + 0.5f * s * this->max_power_);
  } else {
    this->drive_coil_(this->cos_pos_, this->cos_neg_, c);
    this->drive_coil_(this->sin_pos_, this->sin_neg_, s);
  }
}

// Une bobine = un pont en H : une borne en PWM, l'autre a 0 selon le signe
void AircoreGauge::drive_coil_(output::FloatOutput *pos, output::FloatOutput *neg, float level) {
  float duty = std::fabs(level) * this->max_power_;
  if (level >= 0.0f) {
    neg->set_level(0.0f);
    pos->set_level(duty);
  } else {
    pos->set_level(0.0f);
    neg->set_level(duty);
  }
}

void AircoreGauge::dump_config() {
  ESP_LOGCONFIG(TAG,
                "Aircore Gauge:\n"
                "  Wiring: %s\n"
                "  Zero offset: %.1f deg\n"
                "  Max power: %.0f%%\n"
                "  Speed: %.1f deg/s\n"
                "  Value range: %.2f .. %.2f\n"
                "  Angle range: %.1f .. %.1f deg",
                this->three_wire_ ? "3 wires (cos/sin/common)" : "4 wires (cos+/cos-/sin+/sin-)", this->zero_offset_,
                this->max_power_ * 100.0f, this->speed_, this->min_value_, this->max_value_, this->min_angle_,
                this->max_angle_);
}

}  // namespace esphome::aircore_gauge
