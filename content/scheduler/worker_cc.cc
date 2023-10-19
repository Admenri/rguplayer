// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/scheduler/worker_cc.h"

namespace content {

WorkerTreeHost* g_worker_scheduler = nullptr;

WorkerTreeHost::WorkerTreeHost(bool sync_worker) {
  workers_ = std::make_tuple(std::make_unique<EventRunner>(),
                             std::make_unique<RenderRunner>(sync_worker),
                             std::make_unique<BindingRunner>());

  g_worker_scheduler = this;
}

WorkerTreeHost::~WorkerTreeHost() {
  std::get<2>(workers_).reset();
  std::get<1>(workers_).reset();

  g_worker_scheduler = nullptr;
}

WorkerTreeHost* WorkerTreeHost::GetInstance() { return g_worker_scheduler; }

void WorkerTreeHost::Run(RenderRunner::InitParams graph_params,
                         BindingRunner::BindingParams script_params) {
  std::get<1>(workers_)->CreateContextSync(std::move(graph_params));
  std::get<2>(workers_)->InitializeBindingInterpreter();

  std::get<2>(workers_)->PostBindingBoot(std::move(script_params));

  std::get<0>(workers_)->RunMain();
}

scoped_refptr<base::SequencedTaskRunner> WorkerTreeHost::GetUITaskRunner() {
  return std::get<0>(g_worker_scheduler->workers_)->GetUIThreadTaskRunner();
}

scoped_refptr<base::SequencedTaskRunner> WorkerTreeHost::GetRenderTaskRunner() {
  return std::get<1>(g_worker_scheduler->workers_)->GetRenderThreadRunner();
}

scoped_refptr<base::SequencedTaskRunner>
WorkerTreeHost::GetBindingTaskRunner() {
  return std::get<2>(g_worker_scheduler->workers_)->GetBindingRunnerTask();
}

}  // namespace content
