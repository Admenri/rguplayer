#include "ui/widget/widget.h"

#include <SDL_video.h>

#include "base/debug/debugwriter.h"
#include "base/exceptions/exception.h"

namespace ui {

Widget::Widget() : window_(nullptr) {}

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
}

void Widget::Close() {
  if (window_) SDL_DestroyWindow(window_);
}

void Widget::SetFullscreen(bool fullscreen) {
  SDL_SetWindowFullscreen(window_, fullscreen ? 0 : SDL_WINDOW_FULLSCREEN);
}

bool Widget::IsFullscreen() {
  return SDL_GetWindowFlags(window_) & SDL_WINDOW_FULLSCREEN;
}

}  // namespace ui
