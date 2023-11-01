// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_EVENT_EVENT_RUNNER_H_
#define CONTENT_EVENT_EVENT_RUNNER_H_

#include "base/worker/run_loop.h"

namespace content {

class EventRunner final {
 public:
  EventRunner();
  ~EventRunner();

  EventRunner(const EventRunner&) = delete;
  EventRunner& operator=(const EventRunner&) = delete;

  void RunMain();
  scoped_refptr<base::SequencedTaskRunner> GetTaskRunner();

 private:
  void EventFilter(const SDL_Event& sdl_event);

  std::unique_ptr<base::RunLoop> event_loop_;
  scoped_refptr<base::SequencedTaskRunner> ui_runner_;
  base::CallbackListSubscription dispatcher_binding_;

  base::WeakPtrFactory<EventRunner> weak_ptr_factory_{this};
};

}  // namespace content

#endif  // CONTENT_EVENT_EVENT_RUNNER_H_