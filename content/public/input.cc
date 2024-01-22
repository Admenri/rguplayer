// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/input.h"

namespace content {

namespace {

const Input::KeyBinding kDefaultKeyboardBindings[] = {
    {"DOWN", SDL_SCANCODE_DOWN},    {"LEFT", SDL_SCANCODE_LEFT},
    {"RIGHT", SDL_SCANCODE_RIGHT},  {"UP", SDL_SCANCODE_UP},

    {"F5", SDL_SCANCODE_F5},        {"F6", SDL_SCANCODE_F6},
    {"F7", SDL_SCANCODE_F7},        {"F8", SDL_SCANCODE_F8},
    {"F9", SDL_SCANCODE_F9},

    {"SHIFT", SDL_SCANCODE_LSHIFT}, {"SHIFT", SDL_SCANCODE_RSHIFT},
    {"CTRL", SDL_SCANCODE_LCTRL},   {"CTRL", SDL_SCANCODE_RCTRL},
    {"ALT", SDL_SCANCODE_LALT},     {"ALT", SDL_SCANCODE_RALT},

    {"A", SDL_SCANCODE_LSHIFT},     {"B", SDL_SCANCODE_ESCAPE},
    {"B", SDL_SCANCODE_KP_0},       {"B", SDL_SCANCODE_X},
    {"C", SDL_SCANCODE_SPACE},      {"C", SDL_SCANCODE_RETURN},
    {"C", SDL_SCANCODE_Z},          {"X", SDL_SCANCODE_A},
    {"Y", SDL_SCANCODE_S},          {"Z", SDL_SCANCODE_D},
};

const int kDefaultKeyboardBindingsSize =
    sizeof(kDefaultKeyboardBindings) / sizeof(kDefaultKeyboardBindings[0]);

}  // namespace

Input::Input(base::WeakPtr<ui::Widget> input_device) : window_(input_device) {
  memset(key_states_.data(), 0, key_states_.size() * sizeof(KeyState));
  memset(recent_key_states_.data(), 0,
         recent_key_states_.size() * sizeof(KeyState));

  /* Apply default keyboard bindings */
  for (int i = 0; i < kDefaultKeyboardBindingsSize; ++i) {
    key_bindings_.push_back(kDefaultKeyboardBindings[i]);
  }
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
  for (auto& it : key_bindings_) {
    if (it.sym == keysym)
      return key_states_[it.scancode].pressed;
  }

  return false;
}

bool Input::IsTriggered(const std::string& keysym) {
  for (auto& it : key_bindings_) {
    if (it.sym == keysym)
      return key_states_[it.scancode].trigger;
  }

  return false;
}

bool Input::IsRepeated(const std::string& keysym) {
  for (auto& it : key_bindings_) {
    if (it.sym == keysym)
      return key_states_[it.scancode].repeat;
  }

  return false;
}

bool Input::KeyPressed(int scancode) {
  return key_states_[scancode].pressed;
}

bool Input::KeyTriggered(int scancode) {
  return key_states_[scancode].trigger;
}

bool Input::KeyRepeated(int scancode) {
  return key_states_[scancode].repeat;
}

std::string Input::GetKeyName(int scancode) {
  SDL_Keycode key = SDL_GetKeyFromScancode(static_cast<SDL_Scancode>(scancode));
  return std::string(SDL_GetKeyName(key));
}

void Input::GetKeysFromFlag(const std::string& flag, std::vector<int>& out) {
  out.clear();

  for (auto& it : key_bindings_) {
    if (it.sym == flag) {
      out.push_back(it.scancode);
    }
  }
}

void Input::SetKeysFromFlag(const std::string& flag,
                            const std::vector<int>& keys) {
  auto iter = std::remove_if(
      key_bindings_.begin(), key_bindings_.end(),
      [&](const KeyBinding& binding) { return binding.sym == flag; });
  key_bindings_.erase(iter, key_bindings_.end());

  for (const auto& i : keys) {
    KeyBinding binding;
    binding.sym = flag;
    binding.scancode = static_cast<SDL_Scancode>(i);

    key_bindings_.push_back(std::move(binding));
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
