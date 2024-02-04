// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/worker/event_runner.h"

namespace content {

EventRunner::EventRunner() {
  user_event_id_ = SDL_RegisterEvents(EVENT_NUMS);
}

void EventRunner::InitEventDispatcher(base::WeakPtr<ui::Widget> window) {
  window_ = window;
  loop_runner_ =
      std::make_unique<base::RunLoop>(base::RunLoop::MessagePumpType::UI);

  quit_observer_ = base::RunLoop::BindEventDispatcher(
      base::BindRepeating(&EventRunner::EventFilter, base::Unretained(this)));
}

void EventRunner::EventMain() {
  loop_runner_->Run();
}

void EventRunner::EventFilter(const SDL_Event& event) {
  if (event.type - user_event_id_ == QUIT_SYSTEM_EVENT ||
      event.type == SDL_EVENT_QUIT) {
    loop_runner_->QuitWhenIdle();
  }
}

}  // namespace content
