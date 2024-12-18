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

Widget* Widget::FromWindowID(SDL_WindowID window_id) {
  SDL_Window* sdl_window = SDL_GetWindowFromID(window_id);
  return static_cast<Widget*>(SDL_GetPointerProperty(
      SDL_GetWindowProperties(sdl_window), kNativeWidgetKey, nullptr));
}

Widget::Widget() : window_(nullptr), window_id_(SDL_WindowID()) {
  ui_dispatcher_binding_ = base::RunLoop::BindEventDispatcher(
      base::BindRepeating(&Widget::UIEventDispatcher, base::Unretained(this)));
}

Widget::~Widget() {
  if (window_)
    SDL_DestroyWindow(window_);
}

void Widget::Init(InitParams params) {
  auto property_id = SDL_CreateProperties();

  SDL_SetBooleanProperty(property_id, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN,
                         params.opengl);
  SDL_SetBooleanProperty(property_id, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN,
                         params.fullscreen);
  SDL_SetBooleanProperty(property_id, SDL_PROP_WINDOW_CREATE_FOCUSABLE_BOOLEAN,
                         params.activitable);
  SDL_SetBooleanProperty(property_id, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN,
                         params.resizable);

  SDL_SetBooleanProperty(property_id, SDL_PROP_WINDOW_CREATE_HIDDEN_BOOLEAN,
                         true);
  if (params.window_state == WindowPlacement::Maximum)
    SDL_SetBooleanProperty(property_id,
                           SDL_PROP_WINDOW_CREATE_MAXIMIZED_BOOLEAN, true);
  else if (params.window_state == WindowPlacement::Minimum)
    SDL_SetBooleanProperty(property_id,
                           SDL_PROP_WINDOW_CREATE_MINIMIZED_BOOLEAN, true);

  SDL_SetStringProperty(property_id, SDL_PROP_WINDOW_CREATE_TITLE_STRING,
                        params.title.c_str());

  base::Vec2i centered_pos =
      base::Vec2i(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
  SDL_SetNumberProperty(property_id, SDL_PROP_WINDOW_CREATE_X_NUMBER,
                        params.position.value_or(centered_pos).x);
  SDL_SetNumberProperty(property_id, SDL_PROP_WINDOW_CREATE_Y_NUMBER,
                        params.position.value_or(centered_pos).y);

  SDL_SetNumberProperty(property_id, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER,
                        params.size.x);
  SDL_SetNumberProperty(property_id, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER,
                        params.size.y);

  SDL_SetBooleanProperty(property_id,
                         SDL_PROP_WINDOW_CREATE_MOUSE_GRABBED_BOOLEAN,
                         params.initial_grab);
  SDL_SetBooleanProperty(property_id,
                         SDL_PROP_WINDOW_CREATE_TRANSPARENT_BOOLEAN,
                         params.transparent);

  SDL_SetBooleanProperty(property_id, SDL_PROP_WINDOW_CREATE_MENU_BOOLEAN,
                         params.menu_window);
  SDL_SetBooleanProperty(property_id, SDL_PROP_WINDOW_CREATE_TOOLTIP_BOOLEAN,
                         params.tooltip_window);
  SDL_SetBooleanProperty(property_id, SDL_PROP_WINDOW_CREATE_UTILITY_BOOLEAN,
                         params.utility_window);
  if (params.parent_window)
    SDL_GetPointerProperty(property_id, SDL_PROP_WINDOW_CREATE_PARENT_POINTER,
                           params.parent_window->AsSDLWindow());

  SDL_SetBooleanProperty(property_id,
                         SDL_PROP_WINDOW_CREATE_HIGH_PIXEL_DENSITY_BOOLEAN,
                         params.hpixeldensity);
  SDL_SetBooleanProperty(property_id, SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN,
                         params.borderless);
  SDL_SetBooleanProperty(property_id,
                         SDL_PROP_WINDOW_CREATE_ALWAYS_ON_TOP_BOOLEAN,
                         params.always_on_top);

  window_ = SDL_CreateWindowWithProperties(property_id);
  if (!window_)
    LOG(INFO) << "[UI] " << SDL_GetError();

  if (params.hpixeldensity) {
    float dpi = SDL_GetWindowDisplayScale(window_);
    SDL_SetWindowSize(window_, params.size.x * dpi, params.size.y * dpi);
    SDL_SetWindowPosition(window_, SDL_WINDOWPOS_CENTERED,
                          SDL_WINDOWPOS_CENTERED);
  }

  SDL_SetPointerProperty(SDL_GetWindowProperties(window_), kNativeWidgetKey,
                         this);
  SDL_DestroyProperties(property_id);

  if (params.window_state == WindowPlacement::Show)
    SDL_ShowWindow(window_);
  else if (params.window_state == WindowPlacement::Hide)
    SDL_HideWindow(window_);

  window_id_ = SDL_GetWindowID(window_);
}

void Widget::SetFullscreen(bool fullscreen) {
  SDL_SetWindowFullscreen(window_, fullscreen);
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

void Widget::EmulateKeyState(::SDL_Scancode scancode, bool pressed) {
  key_states_[scancode] = pressed;
}

std::string Widget::FetchInputText() {
  std::string output;
  text_lock_.lock();
  output = text_buffer_;
  text_buffer_.clear();
  text_lock_.unlock();
  return output;
}

void Widget::UIEventDispatcher(const SDL_Event& sdl_event) {
  if (sdl_event.type == SDL_EVENT_KEY_DOWN) {
    if (sdl_event.key.windowID == window_id_) {
      if (sdl_event.key.scancode == SDL_SCANCODE_RETURN &&
          (sdl_event.key.mod & SDL_KMOD_ALT)) {
        // Toggle fullscreen
        SetFullscreen(!IsFullscreen());
        return;
      }
    }
  }

  switch (sdl_event.type) {
    case SDL_EVENT_KEY_DOWN:
      if (sdl_event.key.windowID == window_id_)
        key_states_[sdl_event.key.scancode] = true;
      break;
    case SDL_EVENT_KEY_UP:
      if (sdl_event.key.windowID == window_id_)
        key_states_[sdl_event.key.scancode] = false;
      break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (sdl_event.button.windowID == window_id_)
        mouse_state_.states[sdl_event.button.button] = true;
    } break;
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (sdl_event.button.windowID == window_id_) {
        mouse_state_.states[sdl_event.button.button] = false;
        mouse_state_.clicks[sdl_event.button.button] = sdl_event.button.clicks;
      }
    } break;
    case SDL_EVENT_MOUSE_MOTION: {
      if (sdl_event.motion.windowID == window_id_) {
        float scale_x = mouse_state_.resolution.x / mouse_state_.screen.x;
        float scale_y = mouse_state_.resolution.y / mouse_state_.screen.y;
        float origin_x = sdl_event.motion.x - mouse_state_.screen_offset.x;
        float origin_y = sdl_event.motion.y - mouse_state_.screen_offset.y;

        mouse_state_.x = origin_x * scale_x;
        mouse_state_.y = origin_y * scale_y;

        if (mouse_state_.in_window && mouse_state_.focused) {
          if (mouse_state_.visible)
            SDL_ShowCursor();
          else
            SDL_HideCursor();
        }
      }
    } break;
    case SDL_EVENT_MOUSE_WHEEL: {
      if (sdl_event.wheel.windowID == window_id_) {
        int flip =
            (sdl_event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1 : 1);
        mouse_state_.scroll_x += sdl_event.wheel.x * flip;
        mouse_state_.scroll_y += sdl_event.wheel.y * flip;
      }
    } break;
    case SDL_EVENT_FINGER_DOWN: {
      if (sdl_event.tfinger.windowID == window_id_) {
        int i = sdl_event.tfinger.fingerID;
        if (i < MAX_FINGERS)
          finger_states_[i].down = true;
      }
    }  // fallthrough
    case SDL_EVENT_FINGER_MOTION: {
      if (sdl_event.tfinger.windowID == window_id_) {
        int w, h;
        SDL_GetWindowSize(window_, &w, &h);
        int i = sdl_event.tfinger.fingerID;
        if (i < MAX_FINGERS) {
          float scale_x = mouse_state_.resolution.x / mouse_state_.screen.x;
          float scale_y = mouse_state_.resolution.y / mouse_state_.screen.y;
          float origin_x =
              sdl_event.tfinger.x * w - mouse_state_.screen_offset.x;
          float origin_y =
              sdl_event.tfinger.y * h - mouse_state_.screen_offset.y;

          finger_states_[i].x = origin_x * scale_x;
          finger_states_[i].y = origin_y * scale_y;
        }
      }
    } break;
    case SDL_EVENT_FINGER_UP: {
      if (sdl_event.tfinger.windowID == window_id_) {
        int i = sdl_event.tfinger.fingerID;
        if (i < MAX_FINGERS)
          memset(&finger_states_[i], 0, sizeof(finger_states_[0]));
      }
    } break;
    case SDL_EVENT_WINDOW_MOUSE_ENTER: {
      if (sdl_event.window.windowID == window_id_)
        mouse_state_.in_window = true;
    } break;
    case SDL_EVENT_WINDOW_MOUSE_LEAVE: {
      if (sdl_event.window.windowID == window_id_)
        mouse_state_.in_window = false;
    } break;
    case SDL_EVENT_WINDOW_FOCUS_GAINED: {
      if (sdl_event.window.windowID == window_id_)
        mouse_state_.focused = true;
    } break;
    case SDL_EVENT_WINDOW_FOCUS_LOST: {
      if (sdl_event.window.windowID == window_id_)
        mouse_state_.focused = false;
    } break;
    case SDL_EVENT_TEXT_INPUT: {
      if (sdl_event.text.windowID == window_id_) {
        text_lock_.lock();
        text_buffer_ += sdl_event.text.text;
        text_lock_.unlock();
      }
    } break;
    default:
      break;
  }
}

}  // namespace ui
