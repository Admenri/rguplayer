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

using WorkerTree =
    std::tuple<std::unique_ptr<EventRunner>, std::unique_ptr<RenderRunner>,
               std::unique_ptr<BindingRunner>>;

class WorkerTreeHost {
 public:
  WorkerTreeHost(bool sync_worker);
  ~WorkerTreeHost();

  WorkerTreeHost(const WorkerTreeHost&) = delete;
  WorkerTreeHost& operator=(const WorkerTreeHost&) = delete;

  static WorkerTreeHost* GetInstance();

  /* Run scheduler on event thread */
  void Run(RenderRunner::InitParams graph_params,
           BindingRunner::BindingParams script_params);

  static scoped_refptr<base::SequencedTaskRunner> GetUITaskRunner();
  static scoped_refptr<base::SequencedTaskRunner> GetRenderTaskRunner();
  static scoped_refptr<base::SequencedTaskRunner> GetBindingTaskRunner();

  WorkerTree& AccessToTree() { return workers_; }

  /* Binding interface */
  template <int index>
  decltype(auto) AsWorker() {
    return std::get<index>(workers_).get();
  }

 private:
  WorkerTree workers_;
};

}  // namespace content

#endif  // CONTENT_SCHEDULER_WORKER_CC_H_