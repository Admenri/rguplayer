// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SCHEDULER_WORKER_CC_H_
#define CONTENT_SCHEDULER_WORKER_CC_H_

#include "content/binding/binding_runner.h"
#include "content/event/event_runner.h"
#include "content/render/render_runner.h"

namespace content {

class WorkerTreeHost {
 public:
  WorkerTreeHost(bool sync_worker);
  ~WorkerTreeHost();

  WorkerTreeHost(const WorkerTreeHost&) = delete;
  WorkerTreeHost& operator=(const WorkerTreeHost&) = delete;

  static WorkerTreeHost* GetInstance();

  RenderRunner* AsRenderPort() { return render_runner_.get(); }
  BindingRunner* AsBindingPort() { return binding_runner_.get(); }

  /* Run scheduler on event thread */
  void Run(RenderRunner::InitParams graph_params,
           BindingRunner::BindingParams script_params);

  scoped_refptr<base::SequencedTaskRunner> GetUITaskRunner();
  scoped_refptr<base::SequencedTaskRunner> GetRenderTaskRunner();
  scoped_refptr<base::SequencedTaskRunner> GetBindingTaskRunner();

 private:
  std::unique_ptr<EventRunner> event_runner_;
  std::unique_ptr<RenderRunner> render_runner_;
  std::unique_ptr<BindingRunner> binding_runner_;
};

}  // namespace content

#endif  // CONTENT_SCHEDULER_WORKER_CC_H_