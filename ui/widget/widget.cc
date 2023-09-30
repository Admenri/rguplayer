// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/widget/widget.h"

#include <SDL_events.h>
#include <SDL_video.h>

#include <mutex>

#include "base/debug/debugwriter.h"
#include "base/exceptions/exception.h"
#include "base/worker/run_loop.h"

namespace ui {

const char kNativeWidgetKey[] = "UIBase::Widget";

Widget* Widget::FromWindowID(uint32_t window_id) {
  SDL_Window* sdl_window = SDL_GetWindowFromID(window_id);
  return static_cast<Widget*>(SDL_GetWindowData(sdl_window, kNativeWidgetKey));
}

Widget::Widget() : window_(nullptr) {}

Widget::~Widget() {
  if (window_) SDL_DestroyWindow(window_);
}

void Widget::Init(InitParams params) {
  uint32_t window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI;

  if (params.fullscreen) window_flags |= SDL_WINDOW_FULLSCREEN;
  if (params.activitable)
    window_flags =
        window_flags | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS;
  if (params.resizable) window_flags |= SDL_WINDOW_RESIZABLE;

  if (params.window_state == WindowPlacement::Show)
    window_flags |= SDL_WINDOW_SHOWN;
  else if (params.window_state == WindowPlacement::Hide)
    window_flags |= SDL_WINDOW_HIDDEN;
  else if (params.window_state == WindowPlacement::Maximum)
    window_flags |= SDL_WINDOW_MAXIMIZED;
  else if (params.window_state == WindowPlacement::Minimum)
    window_flags |= SDL_WINDOW_MINIMIZED;

  base::Vec2i centered_pos =
      base::Vec2i(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

  window_ = SDL_CreateWindow(params.title.c_str(),
                             params.position.value_or(centered_pos).x,
                             params.position.value_or(centered_pos).y,
                             params.size.x, params.size.y, window_flags);
  SDL_SetWindowData(window_, kNativeWidgetKey, this);

  delegate_ = std::move(params.delegate);
}

void Widget::SetFullscreen(bool fullscreen) {
  SDL_SetWindowFullscreen(window_, fullscreen ? 0 : SDL_WINDOW_FULLSCREEN);
}

bool Widget::IsFullscreen() {
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

}  // namespace ui
