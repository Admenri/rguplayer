// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_INPUT_H_
#define CONTENT_PUBLIC_INPUT_H_

#include <array>

#include "base/memory/ref_counted.h"
#include "ui/widget/widget.h"

namespace content {

class Input final : public base::RefCounted<Input> {
 public:
  using KeySymMap = std::array<std::string, SDL_NUM_SCANCODES>;
  using KeyState = struct {
    bool pressed;
    bool trigger;
    bool repeat;
    int repeat_count;
  };

  Input(base::WeakPtr<ui::Widget> input_device);
  ~Input();

  Input(const Input&) = delete;
  Input& operator=(const Input&) = delete;

  void ApplyKeySymBinding(const KeySymMap& keysyms);

  void Update();

  bool IsPressed(const std::string& keysym);
  bool IsTriggered(const std::string& keysym);
  bool IsRepeated(const std::string& keysym);

  bool KeyPressed(int scancode);
  bool KeyTriggered(int scancode);
  bool KeyRepeated(int scancode);

  std::string GetKeyName(int scancode);
  void GetKeysFromFlag(const std::string& flag, std::vector<int>& out);
  void SetKeysFromFlag(const std::string& flag, const std::vector<int>& keys);

  void GetRecentPressed(std::vector<int>& out);
  void GetRecentTriggered(std::vector<int>& out);
  void GetRecentRepeated(std::vector<int>& out);

  int Dir4();
  int Dir8();

 private:
  KeySymMap key_bindings_;
  std::array<KeyState, SDL_NUM_SCANCODES> key_states_;
  std::array<KeyState, SDL_NUM_SCANCODES> recent_key_states_;

  base::WeakPtr<ui::Widget> window_;
};

}  // namespace content

#endif  //! CONTENT_PUBLIC_INPUT_H_
