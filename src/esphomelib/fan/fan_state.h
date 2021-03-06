//
// Created by Otto Winter on 29.12.17.
//

#ifndef ESPHOMELIB_FAN_FAN_STATE_H
#define ESPHOMELIB_FAN_FAN_STATE_H

#include <functional>

#include "esphomelib/helpers.h"
#include "esphomelib/component.h"
#include "esphomelib/automation.h"
#include "esphomelib/fan/fan_traits.h"
#include "esphomelib/defines.h"

#ifdef USE_FAN

ESPHOMELIB_NAMESPACE_BEGIN

namespace fan {

/// Simple enum to represent the speed of a fan.
enum FanSpeed {
  FAN_SPEED_OFF = 0, ///< The fan is OFF (this option combined with state ON should make the fan be off.)
  FAN_SPEED_LOW, ///< The fan is running on low speed.
  FAN_SPEED_MEDIUM, ///< The fan is running on medium speed.
  FAN_SPEED_HIGH  ///< The fan is running on high/full speed.
};

template<typename T>
class TurnOnAction;
template<typename T>
class TurnOffAction;
template<typename T>
class ToggleAction;

/** This class is shared between the hardware backend and the MQTT frontend to share state.
 *
 * A fan state has several variables that determine the current state: state (ON/OFF),
 * speed (OFF, LOW, MEDIUM, HIGH), oscillating (ON/OFF) and traits (what features are supported).
 * Both the frontend and the backend can register callbacks whenever a state is changed from the
 * frontend and whenever a state is actually changed and should be pushed to the frontend
 */
class FanState : public Nameable {
 public:
  /// Construct the fan state with name.
  explicit FanState(const std::string &name);

  /// Register a callback that will be called each time the state changes.
  void add_on_state_change_callback(std::function<void()> &&update_callback);

  /// Get the current ON/OFF state of this fan.
  bool get_state() const;
  /// Set the current ON/OFF state of this fan.
  void set_state(bool state);
  /// Get the current oscillating state of this fan.
  bool is_oscillating() const;
  /// Set the current oscillating state of this fan.
  void set_oscillating(bool oscillating);
  /// Get the current speed of this fan.
  FanSpeed get_speed() const;
  /// Set the current speed of this fan.
  void set_speed(FanSpeed speed);
  bool set_speed(const char *speed);
  /// Get the traits of this fan (i.e. what features it supports).
  const FanTraits &get_traits() const;
  /// Set the traits of this fan (i.e. what features it supports).
  void set_traits(const FanTraits &traits);

  /// Load a fan state from the preferences into this object.
  void load_from_preferences();
  /// Save the fan state from this object into the preferences.
  void save_to_preferences();

  template<typename T>
  TurnOnAction<T> *make_turn_on_action();
  template<typename T>
  TurnOffAction<T> *make_turn_off_action();
  template<typename T>
  ToggleAction<T> *make_toggle_action();

 protected:
  bool state_{false};
  bool oscillating_{false};
  FanSpeed speed_{FAN_SPEED_HIGH};
  FanTraits traits_{};
  CallbackManager<void()> state_callback_{};
};

template<typename T>
class TurnOnAction : public Action<T> {
 public:
  explicit TurnOnAction(FanState *state);

  void set_oscillating(std::function<bool(T)> &&oscillating);
  void set_oscillating(bool oscillating);
  void set_speed(std::function<FanSpeed(T)> &&speed);
  void set_speed(FanSpeed speed);

  void play(T x) override;

 protected:
  FanState *state_;
  TemplatableValue<bool, T> oscillating_;
  TemplatableValue<FanSpeed , T> speed_;
};

template<typename T>
class TurnOffAction : public Action<T> {
 public:
  explicit TurnOffAction(FanState *state);

  void play(T x) override;
 protected:
  FanState *state_;
};

template<typename T>
class ToggleAction : public Action<T> {
 public:
  explicit ToggleAction(FanState *state);

  void play(T x) override;
 protected:
  FanState *state_;
};

template<typename T>
ToggleAction<T>::ToggleAction(FanState *state) : state_(state) {

}
template<typename T>
void ToggleAction<T>::play(T x) {
  this->state_->set_state(!this->state_->get_state());
  this->play_next(x);
}

template<typename T>
TurnOnAction<T>::TurnOnAction(FanState *state) : state_(state) {

}
template<typename T>
void TurnOnAction<T>::set_oscillating(std::function<bool(T)> &&oscillating) {
  this->oscillating_ = std::move(oscillating);
}
template<typename T>
void TurnOnAction<T>::set_oscillating(bool oscillating) {
  this->oscillating_ = oscillating;
}
template<typename T>
void TurnOnAction<T>::set_speed(std::function<FanSpeed(T)> &&speed) {
  this->speed_ = std::move(speed);
}
template<typename T>
void TurnOnAction<T>::set_speed(FanSpeed speed) {
  this->speed_ = speed;
}
template<typename T>
void TurnOnAction<T>::play(T x) {
  this->state_->set_state(true);
  if (this->oscillating_.has_value()) {
    this->state_->set_oscillating(this->oscillating_.value(x));
  }
  if (this->speed_.has_value()) {
    this->state_->set_speed(this->speed_.value(x));
  }
  this->play_next(x);
}

template<typename T>
TurnOffAction<T>::TurnOffAction(FanState *state) : state_(state) {

}
template<typename T>
void TurnOffAction<T>::play(T x) {
  this->state_->set_state(false);
  this->play_next(x);
}

template<typename T>
TurnOnAction<T> *FanState::make_turn_on_action() {
  return new TurnOnAction<T>(this);
}
template<typename T>
TurnOffAction<T> *FanState::make_turn_off_action() {
  return new TurnOffAction<T>(this);
}
template<typename T>
ToggleAction<T> *FanState::make_toggle_action() {
  return new ToggleAction<T>(this);
}

} // namespace fan

ESPHOMELIB_NAMESPACE_END

#endif //USE_FAN

#endif //ESPHOMELIB_FAN_FAN_STATE_H
