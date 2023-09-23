// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_WIDGET_WIDGET_H_
#define UI_WIDGET_WIDGET_H_

#include <optional>
#include <string>

#include "base/math/math.h"

struct SDL_Window;

namespace ui {

class Widget {
 public:
  enum class WindowPlacement {
    Show = 0,
    Hide,
    Maximum,
    Minimum,
  };

  struct InitParams {
    InitParams() = default;

    InitParams(InitParams&&) = default;
    InitParams(const InitParams&) = delete;
    InitParams& operator=(const InitParams&) = delete;

    std::string title;

    bool resizable = false;

    bool fullscreen = false;

    std::optional<base::Vec2i> position;
    base::Vec2i size;

    bool activitable = true;

    WindowPlacement window_state = WindowPlacement::Show;
  };

  Widget();
  virtual ~Widget();

  Widget(const Widget&) = delete;
  Widget& operator=(const Widget&) = delete;

  void Init(InitParams params);

  void Close();

  void SetFullscreen(bool fullscreen);
  bool IsFullscreen();

  SDL_Window* AsSDLWindow() { return window_; }

 private:
  SDL_Window* window_;
};

}  // namespace ui

#endif  // UI_WIDGET_WIDGET_H_