// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_WIDGET_WIDGET_H_
#define UI_WIDGET_WIDGET_H_

#include "SDL_keyboard.h"
#include "SDL_touch.h"

#include <array>
#include <memory>
#include <optional>
#include <string>

#include "base/bind/callback_list.h"
#include "base/math/math.h"
#include "base/memory/weak_ptr.h"

union SDL_Event;
struct SDL_Window;

namespace ui {

#define MAX_FINGERS 10
#define MOUSE_BUTTON_COUNT 32

class Widget {
 public:
  struct MouseState {
    // Ratio position in window
    float x, y;
    int scroll_x;
    int scroll_y;
    bool states[MOUSE_BUTTON_COUNT];
    uint8_t clicks[MOUSE_BUTTON_COUNT];
    bool visible;

    base::Vec2i screen_offset;
    base::Vec2 screen, resolution;

    bool in_window;
    bool focused;
  };

  struct FingerState {
    bool down;
    float x, y;
  };

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
    bool transparent = false;

    base::WeakPtr<Widget> parent_window = nullptr;
    bool tooltip_window = false;
    bool menu_window = false;
    bool utility_window = false;

    bool hpixeldensity = false;
    bool borderless = false;
    bool always_on_top = false;

    bool initial_grab = false;

    WindowPlacement window_state = WindowPlacement::Show;
  };

  Widget();
  virtual ~Widget();

  Widget(const Widget&) = delete;
  Widget& operator=(const Widget&) = delete;

  void Init(InitParams params);

  void SetFullscreen(bool fullscreen);
  bool IsFullscreen() const;

  void SetTitle(const std::string& window_title);
  std::string GetTitle() const;

  SDL_Window* AsSDLWindow() const { return window_; }
  base::WeakPtr<Widget> AsWeakPtr() { return weak_ptr_factory_.GetWeakPtr(); }

  base::Vec2i GetPosition();
  base::Vec2i GetSize();

  static Widget* FromWindowID(SDL_WindowID window_id);
  SDL_WindowID GetWindowID() const { return window_id_; }

  bool GetKeyState(::SDL_Scancode scancode) const;
  MouseState& GetMouseState() { return mouse_state_; }
  std::array<FingerState, MAX_FINGERS>& GetTouchState() {
    return finger_states_;
  }

 private:
  void UIEventDispatcher(const SDL_Event& sdl_event);

  SDL_Window* window_;
  SDL_WindowID window_id_;
  base::CallbackListSubscription ui_dispatcher_binding_;

  bool key_states_[SDL_NUM_SCANCODES]{0};
  MouseState mouse_state_{0};
  std::array<FingerState, MAX_FINGERS> finger_states_;

  base::WeakPtrFactory<Widget> weak_ptr_factory_{this};
};

}  // namespace ui

#endif  // UI_WIDGET_WIDGET_H_