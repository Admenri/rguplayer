// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/event/event_runner.h"

namespace content {

EventRunner::EventRunner()
    : event_loop_(
          std::make_unique<base::RunLoop>(base::RunLoop::MessagePumpType::UI)),
      ui_runner_(event_loop_->task_runner()) {
  base::RunLoop::BindEventDispatcher(
      SDL_QUIT, base::BindRepeating(&EventRunner::EventFilter,
                                    base::Passed(event_loop_->QuitClosure())));
}

EventRunner::~EventRunner() { event_loop_.reset(); }

void EventRunner::RunMain() { event_loop_->Run(); }

scoped_refptr<base::SequencedTaskRunner> EventRunner::GetTaskRunner() {
  return event_loop_->task_runner();
}

void EventRunner::EventFilter(base::OnceClosure quit_closure,
                              const SDL_Event& sdl_event) {
  if (sdl_event.type == SDL_QUIT) {
    std::move(quit_closure).Run();
  }
}

}  // namespace content
