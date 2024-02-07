// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef CONTENT_WORKER_EVENT_RUNNER_H_
#define CONTENT_WORKER_EVENT_RUNNER_H_

#include "base/memory/ref_counted.h"
#include "base/worker/run_loop.h"
#include "content/config/core_config.h"
#include "ui/widget/widget.h"

#include "SDL_events.h"

namespace content {

class EventRunner : public base::RefCounted<EventRunner> {
 public:
  using EventID = enum {
    QUIT_SYSTEM_EVENT = 0,
    UPDATE_FPS_DISPLAY,

    EVENT_NUMS,
  };

  EventRunner();

  EventRunner(const EventRunner&) = delete;
  EventRunner& operator=(const EventRunner&) = delete;

  void InitEventDispatcher(scoped_refptr<CoreConfigure> config,
                           base::WeakPtr<ui::Widget> window);
  void EventMain();

  uint32_t user_event_id() { return user_event_id_; }
  scoped_refptr<base::SequencedTaskRunner> task_runner() {
    return loop_runner_->task_runner();
  }

 private:
  void EventFilter(const SDL_Event& event);
  uint32_t user_event_id_;
  scoped_refptr<CoreConfigure> config_;
  base::CallbackListSubscription quit_observer_;
  std::unique_ptr<base::RunLoop> loop_runner_;
  base::WeakPtr<ui::Widget> window_;

  struct {
    uint64_t frame_count;
    uint64_t last_counter;
    uint64_t counter_freq;
  } fps_counter_;

  base::WeakPtrFactory<EventRunner> weak_ptr_factory_{this};
};

}  // namespace content

#endif  // !CONTENT_WORKER_EVENT_RUNNER_H_
