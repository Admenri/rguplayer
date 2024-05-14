// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef CONTENT_WORKER_EVENT_RUNNER_H_
#define CONTENT_WORKER_EVENT_RUNNER_H_

#include "base/memory/ref_counted.h"
#include "base/worker/run_loop.h"
#include "content/config/core_config.h"
#include "content/worker/worker_share.h"
#include "ui/widget/widget.h"

#include "SDL_events.h"

#include <optional>

namespace content {

class BindingRunner;

class EventRunner : public base::RefCounted<EventRunner> {
 public:
  using EventID = enum {
    QUIT_SYSTEM_EVENT = 0,
    UPDATE_FPS_DISPLAY,

    EVENT_NUMS,
  };

  EventRunner(WorkerShareData* share_data);

  EventRunner(const EventRunner&) = delete;
  EventRunner& operator=(const EventRunner&) = delete;

  void InitDispatcher(base::WeakPtr<BindingRunner> binding_runner);
  void EventMain();

  scoped_refptr<base::SequencedTaskRunner> task_runner() {
    return loop_runner_->task_runner();
  }

 private:
  static int EventFilter(void* userdata, SDL_Event* event);
  void EventDispatch(const SDL_Event& event);

  WorkerShareData* share_data_;
  base::WeakPtr<BindingRunner> binding_runner_;
  base::CallbackListSubscription quit_observer_;
  std::unique_ptr<base::RunLoop> loop_runner_;

  struct {
    uint64_t frame_count;
    uint64_t last_counter;
    uint64_t counter_freq;
    int32_t average_fps;
  } fps_counter_;

  base::WeakPtrFactory<EventRunner> weak_ptr_factory_{this};
};

}  // namespace content

#endif  // !CONTENT_WORKER_EVENT_RUNNER_H_
