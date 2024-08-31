// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_GAMEPAD_H_
#define CONTENT_PUBLIC_GAMEPAD_H_

#include "base/memory/ref_counted.h"
#include "ui/widget/widget.h"

#include "SDL_gamepad.h"

#include <array>
#include <vector>

namespace content {

class Gamepad : public base::RefCounted<Gamepad> {
 public:
  using ButtonState = struct {
    bool pressed;
    bool trigger;
    bool repeat;
    int repeat_count;
  };

  using GamepadDevice = struct {
    SDL_JoystickID id;
    std::string device_name;
    std::string device_path;
    int player_index;
    std::string guid;
  };

  static void GetAvailableDevices(std::vector<GamepadDevice>& out);

  Gamepad(SDL_JoystickID dev_id);
  ~Gamepad();

  Gamepad(const Gamepad&) = delete;
  Gamepad& operator=(const Gamepad&) = delete;

  SDL_JoystickConnectionState GetConnectState();
  SDL_JoystickID GetInstanceID() const { return id_; }

  int GetPlayerIndex();
  void SetPlayerIndex(int index);

  void Update();

  int GetAxisValue(SDL_GamepadAxis axis_id);
  bool IsButtonPressed(SDL_GamepadButton button_id);
  bool IsButtonTriggered(SDL_GamepadButton button_id);
  bool IsButtonRepeated(SDL_GamepadButton button_id);

  void GetPressedButtons(std::vector<SDL_GamepadButton>& out);

 private:
  SDL_JoystickID id_;
  SDL_Gamepad* device_;
  std::array<ButtonState, SDL_GAMEPAD_BUTTON_MAX> button_states_;
};

}  // namespace content

#endif  //! CONTENT_PUBLIC_GAMEPAD_H_
