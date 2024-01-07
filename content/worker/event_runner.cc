// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/worker/event_runner.h"

namespace content {

EventRunner::EventRunner() {}

EventRunner::~EventRunner() {}

void EventRunner::InitEventDispatcher() {
  loop_runner_ =
      std::make_unique<base::RunLoop>(base::RunLoop::MessagePumpType::UI);

  dispatcher_binding_ = base::RunLoop::BindEventDispatcher(base::BindRepeating(
      &EventRunner::EventFilter, weak_ptr_factory_.GetWeakPtr()));
}

void EventRunner::EventMain() {
  loop_runner_->Run();
}

void EventRunner::EventFilter(const SDL_Event& sdl_event) {
  switch (sdl_event.type) {
    case SDL_EVENT_QUIT:
      return loop_runner_->QuitWhenIdle();
    default:
      break;
  }
}

}  // namespace content
