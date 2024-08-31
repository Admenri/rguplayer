// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/gamepad.h"

#include "base/exceptions/exception.h"

namespace content {

void Gamepad::GetAvailableDevices(std::vector<GamepadDevice>& out) {
  out.clear();

  int device_size;
  auto* available_pads = SDL_GetGamepads(&device_size);
  for (int i = 0; i < device_size; ++i) {
    GamepadDevice item;
    item.id = available_pads[i];
    item.device_name = SDL_GetGamepadNameForID(available_pads[i]);
    item.device_path = SDL_GetGamepadPathForID(available_pads[i]);
    item.player_index = SDL_GetGamepadPlayerIndexForID(available_pads[i]);

    auto guid = SDL_GetGamepadGUIDForID(available_pads[i]);
    char pszGUID[33];
    SDL_GUIDToString(guid, pszGUID, sizeof(pszGUID));
    item.guid = pszGUID;

    out.push_back(item);
  }

  SDL_free(available_pads);
}

Gamepad::Gamepad(SDL_JoystickID dev_id)
    : id_(dev_id), device_(SDL_OpenGamepad(dev_id)), button_states_() {
  if (!device_)
    throw base::Exception(base::Exception::ContentError,
                          "Failed to create gamepad device: %s",
                          SDL_GetError());
}

Gamepad::~Gamepad() {
  if (device_)
    SDL_CloseGamepad(device_);
}

SDL_JoystickConnectionState Gamepad::GetConnectState() {
  if (!SDL_GamepadConnected(device_))
    return SDL_JOYSTICK_CONNECTION_INVALID;

  return SDL_GetGamepadConnectionState(device_);
}

int Gamepad::GetPlayerIndex() {
  return SDL_GetGamepadPlayerIndex(device_);
}

void Gamepad::SetPlayerIndex(int index) {
  SDL_SetGamepadPlayerIndex(device_, index);
}

void Gamepad::Update() {
  for (int i = 0; i < SDL_GAMEPAD_BUTTON_MAX; ++i) {
    bool button_pressed =
        SDL_GetGamepadButton(device_, static_cast<SDL_GamepadButton>(i));

    /* Update key state with elder state */
    button_states_[i].trigger = !button_states_[i].pressed && button_pressed;

    /* After trigger set, set press state */
    button_states_[i].pressed = button_pressed;

    /* Based on press state update the repeat state */
    button_states_[i].repeat = false;
    if (button_states_[i].pressed) {
      ++button_states_[i].repeat_count;

      bool repeated = false;
      repeated = button_states_[i].repeat_count == 1 ||
                 (button_states_[i].repeat_count >= 23 &&
                  (button_states_[i].repeat_count + 1) % 6 == 0);

      button_states_[i].repeat = repeated;
    } else {
      button_states_[i].repeat_count = 0;
    }
  }
}

int Gamepad::GetAxisValue(SDL_GamepadAxis axis_id) {
  return SDL_GetGamepadAxis(device_, axis_id);
}

bool Gamepad::IsButtonPressed(SDL_GamepadButton button_id) {
  return button_states_[button_id].pressed;
}

bool Gamepad::IsButtonTriggered(SDL_GamepadButton button_id) {
  return button_states_[button_id].trigger;
}

bool Gamepad::IsButtonRepeated(SDL_GamepadButton button_id) {
  return button_states_[button_id].repeat;
}

void Gamepad::GetPressedButtons(std::vector<SDL_GamepadButton>& out) {
  out.clear();
  for (int i = 0; i < SDL_GAMEPAD_BUTTON_MAX; ++i)
    if (button_states_[i].pressed)
      out.push_back(static_cast<SDL_GamepadButton>(i));
}

}  // namespace content
