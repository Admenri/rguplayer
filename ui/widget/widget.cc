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

const char kNativeWidgetKey[] = "UIBase::WidgetRawPtr";

void Widget::UIEventDispatcher(const SDL_Event& sdl_event) {
  switch (sdl_event.type) {
    case SDL_WINDOWEVENT: {
      SDL_Window* sdl_window = SDL_GetWindowFromID(sdl_event.window.windowID);
      Widget* widget =
          static_cast<Widget*>(SDL_GetWindowData(sdl_window, kNativeWidgetKey));
      if (!widget) return;

      switch (sdl_event.window.event) {
        case SDL_WINDOWEVENT_CLOSE: {
          widget->Close();
          break;
        }
      }

      break;
    }
  }
}

Widget::Widget() : window_(nullptr) {
  static std::once_flag g_init_callback;
  std::call_once(g_init_callback, base::RunLoop::BindEventDispatcher,
                 SDL_WINDOWEVENT, base::BindRepeating(UIEventDispatcher));
}

Widget::~Widget() { Close(); }

void Widget::Init(InitParams params) {
  uint32_t window_flags = SDL_WINDOW_OPENGL;

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

void Widget::Close() {
  if (window_) {
    if (delegate_) delegate_->OnWidgetDestroying();

    SDL_DestroyWindow(window_);
    window_ = nullptr;

    delete this;
  }
}

void Widget::SetFullscreen(bool fullscreen) {
  SDL_SetWindowFullscreen(window_, fullscreen ? 0 : SDL_WINDOW_FULLSCREEN);
}

bool Widget::IsFullscreen() {
  return SDL_GetWindowFlags(window_) & SDL_WINDOW_FULLSCREEN;
}

}  // namespace ui
