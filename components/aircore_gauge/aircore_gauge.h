#pragma once

#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/components/output/float_output.h"
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif

namespace esphome::aircore_gauge {

// Jauge air-core (bobines croisees sinus/cosinus) : l'aiguille s'aligne sur le champ
// des deux bobines, I(cos) ~ cos(angle) et I(sin) ~ sin(angle).
class AircoreGauge : public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  // apres les sorties PWM (setup_priority::HARDWARE)
  float get_setup_priority() const override { return setup_priority::DATA; }

  // 4 fils : chaque bobine a ses deux bornes, pilotee en pont en H
  void set_four_wire(output::FloatOutput *cos_pos, output::FloatOutput *cos_neg, output::FloatOutput *sin_pos,
                     output::FloatOutput *sin_neg);
  // 3 fils : borne commune tenue a mi-tension (PWM 50 %)
  void set_three_wire(output::FloatOutput *cos, output::FloatOutput *sin, output::FloatOutput *common);

  void set_zero_offset(float deg) { this->zero_offset_ = deg; }
  void set_max_power(float power) { this->max_power_ = power; }
  void set_speed(float deg_per_s) { this->speed_ = deg_per_s; }
  void set_value_range(float min_value, float max_value) {
    this->min_value_ = min_value;
    this->max_value_ = max_value;
  }
  void set_angle_range(float min_angle, float max_angle) {
    this->min_angle_ = min_angle;
    this->max_angle_ = max_angle;
  }
#ifdef USE_SENSOR
  void set_sensor(sensor::Sensor *sensor) { this->sensor_ = sensor; }
#endif

  // Valeur dans l'echelle min_value..max_value (ex: km/h), convertie en angle
  void set_value(float value);
  // Angle direct en degres, borne a min_angle..max_angle
  void set_angle(float angle);

  float get_min_value() const { return this->min_value_; }
  float get_max_value() const { return this->max_value_; }
  float get_angle() const { return this->current_angle_; }

 protected:
  void write_(float angle);
  void drive_coil_(output::FloatOutput *pos, output::FloatOutput *neg, float level);

  bool three_wire_{false};
  output::FloatOutput *cos_pos_{nullptr};
  output::FloatOutput *cos_neg_{nullptr};
  output::FloatOutput *sin_pos_{nullptr};
  output::FloatOutput *sin_neg_{nullptr};
  output::FloatOutput *common_{nullptr};
#ifdef USE_SENSOR
  sensor::Sensor *sensor_{nullptr};
#endif

  float zero_offset_{0};
  float max_power_{0.8f};
  float speed_{180};
  float min_value_{0};
  float max_value_{100};
  float min_angle_{0};
  float max_angle_{270};

  float target_angle_{0};
  float current_angle_{0};
  uint32_t last_ms_{0};
};

template<typename... Ts> class SetValueAction final : public Action<Ts...> {
 public:
  explicit SetValueAction(AircoreGauge *gauge) : gauge_(gauge) {}
  TEMPLATABLE_VALUE(float, value)

  void play(const Ts &...x) override { this->gauge_->set_value(this->value_.value(x...)); }

 protected:
  AircoreGauge *gauge_;
};

template<typename... Ts> class SetAngleAction final : public Action<Ts...> {
 public:
  explicit SetAngleAction(AircoreGauge *gauge) : gauge_(gauge) {}
  TEMPLATABLE_VALUE(float, angle)

  void play(const Ts &...x) override { this->gauge_->set_angle(this->angle_.value(x...)); }

 protected:
  AircoreGauge *gauge_;
};

}  // namespace esphome::aircore_gauge
