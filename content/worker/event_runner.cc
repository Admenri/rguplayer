// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/worker/event_runner.h"

namespace content {

EventRunner::EventRunner() {}

EventRunner::~EventRunner() {}

void EventRunner::InitEventDispatcher(base::WeakPtr<ui::Widget> input_widget) {
  quit_observer_ = input_widget->AddDestroyObserver(
      base::BindOnce(&EventRunner::OnWidgetDestroying, base::Unretained(this)));
  loop_runner_ =
      std::make_unique<base::RunLoop>(base::RunLoop::MessagePumpType::UI);
}

void EventRunner::EventMain() {
  loop_runner_->Run();
}

void EventRunner::OnWidgetDestroying() {
  // Quit event required
  loop_runner_->QuitWhenIdle();
}

}  // namespace content
