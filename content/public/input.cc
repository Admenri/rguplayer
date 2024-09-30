// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/input.h"

#include "base/worker/run_loop.h"
#include "content/common/command_ids.h"
#include "third_party/imgui/imgui.h"

#include "SDL_events.h"

#include <fstream>

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
    {"X", SDL_SCANCODE_A},          {"Y", SDL_SCANCODE_S},
    {"Z", SDL_SCANCODE_D},          {"L", SDL_SCANCODE_Q},
    {"R", SDL_SCANCODE_W},
};

const int kDefaultKeyboardBindingsSize =
    sizeof(kDefaultKeyboardBindings) / sizeof(kDefaultKeyboardBindings[0]);

const Input::KeyBinding kKeyboardBindings1[] = {
    {"A", SDL_SCANCODE_Z},
    {"C", SDL_SCANCODE_C},
};

const int kKeyboardBindings1Size =
    sizeof(kKeyboardBindings1) / sizeof(kKeyboardBindings1[0]);

const Input::KeyBinding kKeyboardBindings2[] = {
    {"C", SDL_SCANCODE_Z},
};

const int kKeyboardBindings2Size =
    sizeof(kKeyboardBindings2) / sizeof(kKeyboardBindings2[0]);

const std::string kArrowDirsSymbol[] = {
    "DOWN",
    "LEFT",
    "RIGHT",
    "UP",
};

const int kArrowDirsSymbolSize =
    sizeof(kArrowDirsSymbol) / sizeof(kArrowDirsSymbol[0]);

const std::array<std::string, 12> kButtonItems = {
    "A", "B", "C", "X", "Y", "Z", "L", "R", "DOWN", "LEFT", "RIGHT", "UP",
};

}  // namespace

Input::Input(WorkerShareData* share_data) : share_data_(share_data) {
  window_ = share_data_->window;
  memset(key_states_.data(), 0, key_states_.size() * sizeof(KeyState));
  memset(recent_key_states_.data(), 0,
         recent_key_states_.size() * sizeof(KeyState));

  /* Apply default keyboard bindings */
  for (int i = 0; i < kDefaultKeyboardBindingsSize; ++i)
    key_bindings_.push_back(kDefaultKeyboardBindings[i]);

  if (share_data_->config->content_version() == RGSSVersion::RGSS1)
    for (int i = 0; i < kKeyboardBindings1Size; ++i)
      key_bindings_.push_back(kKeyboardBindings1[i]);

  if (share_data_->config->content_version() >= RGSSVersion::RGSS2)
    for (int i = 0; i < kKeyboardBindings2Size; ++i)
      key_bindings_.push_back(kKeyboardBindings2[i]);

  TryReadBindingsInternal();

  setting_bindings_ = key_bindings_;
  share_data_->create_button_settings_gui = base::BindRepeating(
      &Input::ShowButtonSettingsGUI, weak_ptr_factory_.GetWeakPtr());
}

Input::~Input() {
  StorageBindingsInternal();
}

void Input::ApplyKeySymBinding(const KeySymMap& keysyms) {
  key_bindings_ = keysyms;
}

void Input::Update() {
  if (share_data_->menu_window_focused)
    return;

  for (int i = 0; i < SDL_SCANCODE_COUNT; ++i) {
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
      repeated = key_states_[i].repeat_count == 1 ||
                 (key_states_[i].repeat_count >= 23 &&
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

  UpdateDir4Internal();
  UpdateDir8Internal();
}

bool Input::IsPressed(const std::string& keysym) {
  if (keysym.empty())
    return false;

  for (auto& it : key_bindings_) {
    if (it.sym == keysym)
      if (key_states_[it.scancode].pressed)
        return true;
  }

  return false;
}

bool Input::IsTriggered(const std::string& keysym) {
  if (keysym.empty())
    return false;

  for (auto& it : key_bindings_) {
    if (it.sym == keysym)
      if (key_states_[it.scancode].trigger)
        return true;
  }

  return false;
}

bool Input::IsRepeated(const std::string& keysym) {
  if (keysym.empty())
    return false;

  for (auto& it : key_bindings_) {
    if (it.sym == keysym)
      if (key_states_[it.scancode].repeat)
        return true;
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
  SDL_Keycode key = SDL_GetKeyFromScancode(static_cast<SDL_Scancode>(scancode),
                                           SDL_KMOD_NONE, false);
  return std::string(SDL_GetKeyName(key));
}

void Input::GetKeysFromFlag(const std::string& flag, std::vector<int>& out) {
  out.clear();
  if (flag.empty())
    return;

  for (auto& it : key_bindings_) {
    if (it.sym == flag) {
      out.push_back(it.scancode);
    }
  }
}

void Input::SetKeysFromFlag(const std::string& flag,
                            const std::vector<int>& keys) {
  if (flag.empty())
    return;

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

  for (size_t i = 0; i < recent_key_states_.size(); ++i) {
    if (recent_key_states_[i].pressed) {
      out.push_back(i);
    }
  }
}

void Input::GetRecentTriggered(std::vector<int>& out) {
  out.clear();

  for (size_t i = 0; i < recent_key_states_.size(); ++i) {
    if (recent_key_states_[i].trigger) {
      out.push_back(i);
    }
  }
}

void Input::GetRecentRepeated(std::vector<int>& out) {
  out.clear();

  for (size_t i = 0; i < recent_key_states_.size(); ++i) {
    if (recent_key_states_[i].repeat) {
      out.push_back(i);
    }
  }
}

int Input::Dir4() {
  return dir4_state_.active;
}

int Input::Dir8() {
  return dir8_state_.active;
}

void Input::EmulateKeyState(int scancode, bool pressed) {
  window_->EmulateKeyState((::SDL_Scancode)scancode, pressed);
}

void Input::SetTextInput(bool enable) {
  share_data_->event_runner->PostTask(base::BindOnce(
      [](SDL_Window* window, bool enable_input) {
        enable_input ? SDL_StartTextInput(window) : SDL_StopTextInput(window);
      },
      window_->AsSDLWindow(), enable));
  share_data_->event_runner->WaitForSync();

  FetchText();
}

bool Input::IsTextInput() {
  return SDL_TextInputActive(window_->AsSDLWindow());
}

std::string Input::FetchText() {
  return window_->FetchInputText();
}

void Input::SetTextInputRect(scoped_refptr<Rect> region) {
  auto rt = region->AsBase();
  SDL_Rect sdlrt{rt.x, rt.y, rt.width, rt.height};

  share_data_->event_runner->PostTask(base::BindOnce(
      [](SDL_Window* window, SDL_Rect* rect) {
        SDL_SetTextInputArea(window, rect, 0);
      },
      window_->AsSDLWindow(), &sdlrt));
  share_data_->event_runner->WaitForSync();
}

void Input::UpdateDir4Internal() {
  bool key_states[kArrowDirsSymbolSize] = {0};
  for (auto& it : key_bindings_)
    for (int i = 0; i < kArrowDirsSymbolSize; ++i)
      if (it.sym == kArrowDirsSymbol[i])
        key_states[i] |= key_states_[it.scancode].pressed;

  int dir_flag = 0;
  const int dir_flags_fix[] = {
      1 << 1,
      1 << 2,
      1 << 3,
      1 << 4,
  };

  const int block_dir_flags[] = {dir_flags_fix[0] | dir_flags_fix[3],
                                 dir_flags_fix[1] | dir_flags_fix[2]};

  const int other_dirs[][3] = {
      {1, 2, 3},
      {0, 3, 2},
      {0, 3, 1},
      {1, 2, 0},
  };

  for (size_t i = 0; i < 4; ++i)
    dir_flag |= (key_states[i] ? dir_flags_fix[i] : 0);

  if (dir_flag == block_dir_flags[0] || dir_flag == block_dir_flags[1]) {
    dir4_state_.active = 0;
    return;
  }

  if (dir4_state_.previous) {
    if (key_states[dir4_state_.previous / 2 - 1]) {
      for (size_t i = 0; i < 3; ++i) {
        int other_key = other_dirs[dir4_state_.previous / 2 - 1][i];
        if (!key_states[other_key])
          continue;

        dir4_state_.active = (other_key + 1) * 2;
        return;
      }
    }
  }

  for (size_t i = 0; i < 4; ++i) {
    if (!key_states[i])
      continue;

    dir4_state_.active = (i + 1) * 2;
    dir4_state_.previous = (i + 1) * 2;
    return;
  }

  dir4_state_.active = 0;
  dir4_state_.previous = 0;
}

void Input::UpdateDir8Internal() {
  bool key_states[kArrowDirsSymbolSize] = {0};
  for (auto& it : key_bindings_)
    for (int i = 0; i < kArrowDirsSymbolSize; ++i)
      if (it.sym == kArrowDirsSymbol[i])
        key_states[i] |= key_states_[it.scancode].pressed;

  static const int combos[4][4] = {
      {2, 1, 3, 0}, {1, 4, 0, 7}, {3, 0, 6, 9}, {0, 7, 9, 8}};

  const int other_dirs[][3] = {
      {1, 2, 3},
      {0, 3, 2},
      {0, 3, 1},
      {1, 2, 0},
  };

  dir8_state_.active = 0;

  for (size_t i = 0; i < 4; ++i) {
    if (!key_states[i])
      continue;

    for (int j = 0; j < 3; ++j) {
      int other_key = other_dirs[i][j];
      if (!key_states[other_key])
        continue;

      dir8_state_.active = combos[i][other_key];
      return;
    }

    dir8_state_.active = (i + 1) * 2;
    return;
  }
}

void Input::ShowButtonSettingsGUI() {
  scoped_refptr<CoreConfigure> config = share_data_->config;
  static int selected_button = 0, selected_binding = -1;
  share_data_->disable_gui_key_input = (selected_binding != -1);

  if (ImGui::CollapsingHeader(
          config->GetI18NString(IDS_SETTINGS_BUTTON, "Button").c_str())) {
    auto list_height = 6 * ImGui::GetTextLineHeightWithSpacing();
    // Button name list box
    if (ImGui::BeginListBox(
            "##button_list",
            ImVec2(ImGui::CalcItemWidth() / 2.0f, list_height + 64))) {
      for (size_t i = 0; i < kButtonItems.size(); ++i) {
        if (ImGui::Selectable(kButtonItems[i].c_str(),
                              static_cast<int>(i) == selected_button)) {
          selected_button = i;
          selected_binding = -1;
        }
      }

      ImGui::EndListBox();
    }

    ImGui::SameLine();
    std::string button_name = kButtonItems[selected_button];

    // Binding list box
    {
      ImGui::BeginGroup();
      if (ImGui::BeginListBox("##binding_list",
                              ImVec2(-FLT_MIN, list_height))) {
        for (size_t i = 0; i < setting_bindings_.size(); ++i) {
          auto& it = setting_bindings_[i];
          if (it.sym == button_name) {
            const bool is_select = (selected_binding == static_cast<int>(i));

            // Generate button sign
            std::string display_button_name = SDL_GetScancodeName(it.scancode);
            if (it.scancode == SDL_SCANCODE_UNKNOWN)
              display_button_name = "<x>";
            if (is_select)
              display_button_name = "<...>";
            display_button_name += "##";
            display_button_name.push_back(i);

            // Find conflict bindings
            int conflict_count = -1;
            for (auto& item : setting_bindings_)
              if (item == it)
                conflict_count++;

            // Draw selectable
            const bool is_conflict = !!conflict_count;
            if (is_conflict)
              ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));

            if (ImGui::Selectable(display_button_name.c_str(), is_select)) {
              selected_binding = (is_select ? -1 : i);
            }

            if (is_conflict)
              ImGui::PopStyleColor();
          }
        }

        ImGui::EndListBox();
      }

      // Get any keys state for binding
      if (share_data_->disable_gui_key_input) {
        for (int code = 0; code < SDL_SCANCODE_COUNT; ++code) {
          bool key_pressed =
              window_->GetKeyState(static_cast<SDL_Scancode>(code));
          if (key_pressed) {
            setting_bindings_[selected_binding].scancode =
                static_cast<SDL_Scancode>(code);
            selected_binding = -1;
          }
        }
      }

      // Add binding
      if (ImGui::Button(config->GetI18NString(IDS_BUTTON_ADD, "Add").c_str())) {
        selected_binding = -1;
        setting_bindings_.push_back(
            KeyBinding{button_name, SDL_SCANCODE_UNKNOWN});
      }

      ImGui::SameLine();

      // Remove binding
      if (selected_binding >= 0 &&
          ImGui::Button(
              config->GetI18NString(IDS_BUTTON_REMOVE, "Remove").c_str())) {
        auto it = setting_bindings_.begin();
        for (int i = 0; i < selected_binding; ++i)
          it++;

        setting_bindings_.erase(it);
        selected_binding = -1;
      }

      ImGui::EndGroup();
    }

    if (ImGui::Button(
            config->GetI18NString(IDS_BUTTON_SAVE_SETTINGS, "Save Settings")
                .c_str())) {
      key_bindings_ = setting_bindings_;
      selected_binding = -1;
    }

    ImGui::SameLine();
    if (ImGui::Button(
            config->GetI18NString(IDS_BUTTON_RESET_SETTINGS, "Reset Settings")
                .c_str())) {
      setting_bindings_ = key_bindings_;
      selected_binding = -1;
    }
  }
}

void Input::TryReadBindingsInternal() {
  std::string filepath = share_data_->config->executable_file();
  filepath += ".cg";
  filepath += std::to_string((int)share_data_->config->content_version());

  std::ifstream fs(filepath, std::ios::binary);
  if (fs.is_open()) {
    key_bindings_.clear();

    uint32_t item_size;
    fs.read(reinterpret_cast<char*>(&item_size), sizeof(item_size));
    for (uint32_t i = 0; i < item_size; ++i) {
      uint32_t token_size;
      fs.read(reinterpret_cast<char*>(&token_size), sizeof(token_size));
      std::string token(token_size, 0);
      fs.read(token.data(), token_size);
      SDL_Scancode scancode;
      fs.read(reinterpret_cast<char*>(&scancode), sizeof(scancode));

      key_bindings_.push_back({token, scancode});
    }

    fs.close();
  }
}

void Input::StorageBindingsInternal() {
  std::string filepath = share_data_->config->executable_file();
  filepath += ".cg";
  filepath += std::to_string((int)share_data_->config->content_version());

  std::ofstream fs(filepath, std::ios::binary);
  if (fs.is_open()) {
    uint32_t item_size = key_bindings_.size();
    fs.write(reinterpret_cast<const char*>(&item_size), sizeof(item_size));
    for (const auto& it : key_bindings_) {
      uint32_t token_size = it.sym.size();
      fs.write(reinterpret_cast<const char*>(&token_size), sizeof(token_size));
      fs.write(it.sym.data(), token_size);

      SDL_Scancode scancode = it.scancode;
      fs.write(reinterpret_cast<const char*>(&scancode), sizeof(scancode));
    }

    fs.close();
  }
}

}  // namespace content
