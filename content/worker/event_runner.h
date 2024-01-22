// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef CONTENT_WORKER_EVENT_RUNNER_H_
#define CONTENT_WORKER_EVENT_RUNNER_H_

#include "base/memory/ref_counted.h"
#include "base/worker/run_loop.h"
#include "ui/widget/widget.h"

#include "SDL_events.h"

namespace content {

class EventRunner : public base::RefCounted<EventRunner> {
 public:
  EventRunner();
  ~EventRunner();

  EventRunner(const EventRunner&) = delete;
  EventRunner& operator=(const EventRunner&) = delete;

  void InitEventDispatcher(base::WeakPtr<ui::Widget> input_widget);
  void EventMain();

  scoped_refptr<base::SequencedTaskRunner> task_runner() {
    return loop_runner_->task_runner();
  }

 private:
  void OnWidgetDestroying();
  base::CallbackListSubscription quit_observer_;

  std::unique_ptr<base::RunLoop> loop_runner_;

  base::WeakPtrFactory<EventRunner> weak_ptr_factory_{this};
};

}  // namespace content

#endif  // !CONTENT_WORKER_EVENT_RUNNER_H_
