// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/widget/widget.h"

#include <SDL_events.h>
#include <SDL_video.h>

#include <mutex>

#include "base/exceptions/exception.h"
#include "base/worker/run_loop.h"

namespace ui {

const char kNativeWidgetKey[] = "UIBase::Widget";

Widget* Widget::FromWindowID(uint32_t window_id) {
  SDL_Window* sdl_window = SDL_GetWindowFromID(window_id);
  return static_cast<Widget*>(SDL_GetProperty(
      SDL_GetWindowProperties(sdl_window), kNativeWidgetKey, nullptr));
}

Widget::Widget() : window_(nullptr) {
  ui_dispatcher_binding_ = base::RunLoop::BindEventDispatcher(
      base::BindRepeating(&Widget::UIEventDispatcher, base::Unretained(this)));
}

Widget::~Widget() {
  if (window_)
    SDL_DestroyWindow(window_);
}

void Widget::Init(InitParams params) {
  auto property_id = SDL_CreateProperties();

  SDL_SetBooleanProperty(property_id, "opengl", SDL_TRUE);
  SDL_SetBooleanProperty(property_id, "fullscreen", params.fullscreen);
  SDL_SetBooleanProperty(property_id, "focusable", params.activitable);
  SDL_SetBooleanProperty(property_id, "resizable", params.resizable);

  if (params.window_state == WindowPlacement::Show)
    SDL_SetBooleanProperty(property_id, "hidden", SDL_FALSE);
  else if (params.window_state == WindowPlacement::Hide)
    SDL_SetBooleanProperty(property_id, "hidden", SDL_TRUE);
  else if (params.window_state == WindowPlacement::Maximum)
    SDL_SetBooleanProperty(property_id, "maximized", SDL_TRUE);
  else if (params.window_state == WindowPlacement::Minimum)
    SDL_SetBooleanProperty(property_id, "minimized", SDL_TRUE);

  SDL_SetStringProperty(property_id, "title", params.title.c_str());

  base::Vec2i centered_pos =
      base::Vec2i(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
  SDL_SetNumberProperty(property_id, "x",
                        params.position.value_or(centered_pos).x);
  SDL_SetNumberProperty(property_id, "y",
                        params.position.value_or(centered_pos).y);

  SDL_SetNumberProperty(property_id, "width", params.size.x);
  SDL_SetNumberProperty(property_id, "height", params.size.y);

  SDL_SetBooleanProperty(property_id, "mouse-grabbed", params.initial_grab);
  SDL_SetBooleanProperty(property_id, "transparent", params.transparent);

  SDL_SetBooleanProperty(property_id, "menu", params.menu_window);
  SDL_SetBooleanProperty(property_id, "tooltip", params.tooltip_window);
  SDL_SetBooleanProperty(property_id, "utility", params.utility_window);
  if (params.parent_window)
    SDL_SetProperty(property_id, "parent", params.parent_window->AsSDLWindow());

  SDL_SetBooleanProperty(property_id, "high-pixel-density",
                         params.hpixeldensity);
  SDL_SetBooleanProperty(property_id, "borderless", params.borderless);
  SDL_SetBooleanProperty(property_id, "always-on-top", params.always_on_top);

  window_ = SDL_CreateWindowWithProperties(property_id);
  SDL_SetProperty(SDL_GetWindowProperties(window_), kNativeWidgetKey, this);

  SDL_DestroyProperties(property_id);
}

void Widget::SetFullscreen(bool fullscreen) {
  SDL_SetWindowFullscreen(window_, fullscreen ? 0 : SDL_WINDOW_FULLSCREEN);
}

void Widget::SetTitle(const std::string& window_title) {
  SDL_SetWindowTitle(window_, window_title.c_str());
}

std::string Widget::GetTitle() const {
  return std::string(SDL_GetWindowTitle(window_));
}

bool Widget::IsFullscreen() const {
  return SDL_GetWindowFlags(window_) & SDL_WINDOW_FULLSCREEN;
}

base::Vec2i Widget::GetPosition() {
  base::Vec2i pos;
  SDL_GetWindowPosition(window_, &pos.x, &pos.y);
  return pos;
}

base::Vec2i Widget::GetSize() {
  base::Vec2i size;
  SDL_GetWindowSize(window_, &size.x, &size.y);
  return size;
}

bool Widget::GetKeyState(::SDL_Scancode scancode) const {
  return key_states_[scancode];
}

void Widget::UIEventDispatcher(const SDL_Event& sdl_event) {
  uint32_t window_id = SDL_GetWindowID(window_);

  switch (sdl_event.type) {
    case SDL_EVENT_KEY_DOWN:
      if (sdl_event.key.windowID == window_id) {
        key_states_[sdl_event.key.keysym.scancode] = true;
      }
      break;
    case SDL_EVENT_KEY_UP:
      if (sdl_event.key.windowID == window_id) {
        key_states_[sdl_event.key.keysym.scancode] = false;
      }
      break;
    default:
      break;
  }
}

}  // namespace ui
