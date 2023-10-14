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
  scoped_refptr<base::SequencedTaskRunner> GetUIThreadTaskRunner();

 private:
  friend class WorkerTreeHost;

  static void EventFilter(base::OnceClosure quit_closure,
                          const SDL_Event& sdl_event);

  std::unique_ptr<base::RunLoop> event_loop_;
  scoped_refptr<base::SequencedTaskRunner> ui_runner_;
};

}  // namespace content

#endif  // CONTENT_EVENT_EVENT_RUNNER_H_