// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/script/input.h"

namespace content {

Input::Input(base::WeakPtr<ui::Widget> input_device) : window_(input_device) {
  memset(key_states_.data(), 0, key_states_.size() * sizeof(KeyState));
  memset(recent_key_states_.data(), 0,
         recent_key_states_.size() * sizeof(KeyState));
}

Input::~Input() {}

void Input::ApplyKeySymBinding(const KeySymMap& keysyms) {
  key_bindings_ = keysyms;
}

void Input::Update() {
  for (int i = 0; i < SDL_NUM_SCANCODES; ++i) {
    bool key_pressed = window_->GetKeyState(static_cast<SDL_Scancode>(i));

    /* Update key state with elder state */
    key_states_[i].trigger = !key_states_[i].pressed && key_pressed;

    /* After trigger set, set press state */
    key_states_[i].pressed = key_pressed;

    /* Based on press state update the repeat state */
    key_states_[i].repeat = false;
    if (key_states_[i].pressed) {
      ++key_states_[i].repeat_count;

      bool repeated = false;
      // TODO: RGSS 1/2/3 specific process
      repeated = (key_states_[i].repeat_count >= 23 &&
                  (key_states_[i].repeat_count + 1) % 6 == 0);

      key_states_[i].repeat = repeated;
    } else {
      key_states_[i].repeat_count = 0;
    }

    /* Update recent key infos */
    recent_key_states_[i].pressed = key_states_[i].pressed;
    recent_key_states_[i].trigger = key_states_[i].trigger;
    recent_key_states_[i].repeat = key_states_[i].repeat;
  }
}

bool Input::IsPressed(const std::string& keysym) {
  for (int i = 0; i < key_bindings_.size(); ++i) {
    if (key_bindings_[i] == keysym) return key_states_[i].pressed;
  }

  return false;
}

bool Input::IsTriggered(const std::string& keysym) {
  for (int i = 0; i < key_bindings_.size(); ++i) {
    if (key_bindings_[i] == keysym) return key_states_[i].trigger;
  }

  return false;
}

bool Input::IsRepeated(const std::string& keysym) {
  for (int i = 0; i < key_bindings_.size(); ++i) {
    if (key_bindings_[i] == keysym) return key_states_[i].repeat;
  }

  return false;
}

bool Input::KeyPressed(int scancode) { return key_states_[scancode].pressed; }

bool Input::KeyTriggered(int scancode) { return key_states_[scancode].trigger; }

bool Input::KeyRepeated(int scancode) { return key_states_[scancode].repeat; }

std::string Input::GetKeyName(int scancode) {
  SDL_Keycode key = SDL_GetKeyFromScancode(static_cast<SDL_Scancode>(scancode));
  return std::string(SDL_GetKeyName(key));
}

void Input::GetKeysFromFlag(const std::string& flag, std::vector<int>& out) {
  out.clear();

  for (int i = 0; i < key_bindings_.size(); ++i) {
    if (key_bindings_[i] == flag) {
      out.push_back(i);
    }
  }
}

void Input::SetKeysFromFlag(const std::string& flag,
                            const std::vector<int>& keys) {
  for (const auto& i : keys) {
    key_bindings_[i] = flag;
  }
}

void Input::GetRecentPressed(std::vector<int>& out) {
  out.clear();

  for (int i = 0; i < recent_key_states_.size(); ++i) {
    if (recent_key_states_[i].pressed) {
      out.push_back(i);
    }
  }
}

void Input::GetRecentTriggered(std::vector<int>& out) {
  out.clear();

  for (int i = 0; i < recent_key_states_.size(); ++i) {
    if (recent_key_states_[i].trigger) {
      out.push_back(i);
    }
  }
}

void Input::GetRecentRepeated(std::vector<int>& out) {
  out.clear();

  for (int i = 0; i < recent_key_states_.size(); ++i) {
    if (recent_key_states_[i].repeat) {
      out.push_back(i);
    }
  }
}

int Input::Dir4() {
  if (key_states_[SDL_SCANCODE_UP].pressed) {
    return 8;
  } else if (key_states_[SDL_SCANCODE_RIGHT].pressed) {
    return 6;
  } else if (key_states_[SDL_SCANCODE_LEFT].pressed) {
    return 4;
  } else if (key_states_[SDL_SCANCODE_DOWN].pressed) {
    return 2;
  }

  return 0;
}

int Input::Dir8() {
  if (key_states_[SDL_SCANCODE_UP].pressed &&
      key_states_[SDL_SCANCODE_RIGHT].pressed) {
    return 9;
  } else if (key_states_[SDL_SCANCODE_UP].pressed &&
             key_states_[SDL_SCANCODE_LEFT].pressed) {
    return 7;
  } else if (key_states_[SDL_SCANCODE_DOWN].pressed &&
             key_states_[SDL_SCANCODE_RIGHT].pressed) {
    return 3;
  } else if (key_states_[SDL_SCANCODE_DOWN].pressed &&
             key_states_[SDL_SCANCODE_LEFT].pressed) {
    return 1;
  }

  return Dir4();
}

}  // namespace content
