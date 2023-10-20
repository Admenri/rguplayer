// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SCHEDULER_WORKER_CC_H_
#define CONTENT_SCHEDULER_WORKER_CC_H_

#include <tuple>

#include "content/binding/binding_runner.h"
#include "content/event/event_runner.h"
#include "content/render/render_runner.h"

namespace content {

class WorkerTreeHost final {
 public:
  WorkerTreeHost(bool sync_worker);
  ~WorkerTreeHost();

  WorkerTreeHost(const WorkerTreeHost&) = delete;
  WorkerTreeHost& operator=(const WorkerTreeHost&) = delete;

  /* Run scheduler on event thread */
  void Run(RenderRunner::InitParams graph_params,
           BindingRunner::InitParams script_params);

  scoped_refptr<base::SequencedTaskRunner> GetUITaskRunner() {
    return event_runner_->GetTaskRunner();
  }

  scoped_refptr<base::SequencedTaskRunner> GetRenderTaskRunner() {
    return render_runner_->GetTaskRunner();
  }

  scoped_refptr<base::SequencedTaskRunner> GetBindingRunner() {
    return binding_runner_->GetTaskRunner();
  }

 private:
  std::unique_ptr<EventRunner> event_runner_;
  std::unique_ptr<RenderRunner> render_runner_;
  std::unique_ptr<BindingRunner> binding_runner_;
};

}  // namespace content

#endif  // CONTENT_SCHEDULER_WORKER_CC_H_