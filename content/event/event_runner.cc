// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/event/event_runner.h"

namespace content {

EventRunner::EventRunner()
    : event_loop_(
          std::make_unique<base::RunLoop>(base::RunLoop::MessagePumpType::UI)),
      ui_runner_(event_loop_->task_runner()) {
  dispatcher_binding_ = base::RunLoop::BindEventDispatcher(base::BindRepeating(
      &EventRunner::EventFilter, weak_ptr_factory_.GetWeakPtr()));
}

EventRunner::~EventRunner() { event_loop_.reset(); }

void EventRunner::RunMain() { event_loop_->Run(); }

scoped_refptr<base::SequencedTaskRunner> EventRunner::GetTaskRunner() {
  return event_loop_->task_runner();
}

void EventRunner::EventFilter(const SDL_Event& sdl_event) {
  switch (sdl_event.type) {
    case SDL_QUIT:
      return event_loop_->QuitWhenIdle();
    default:
      break;
  }
}

}  // namespace content
