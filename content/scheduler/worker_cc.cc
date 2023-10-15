// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/scheduler/worker_cc.h"

namespace content {

WorkerTreeHost* g_worker_scheduler = nullptr;

WorkerTreeHost::WorkerTreeHost(bool sync_worker) {
  event_runner_ = std::make_unique<EventRunner>();
  render_runner_ = std::make_unique<RenderRunner>(sync_worker);
  g_worker_scheduler = this;
}

WorkerTreeHost::~WorkerTreeHost() {
  render_runner_.reset();
  g_worker_scheduler = nullptr;
}

WorkerTreeHost* WorkerTreeHost::GetInstance() { return g_worker_scheduler; }

void WorkerTreeHost::Run(RenderRunner::InitParams graph_params) {
  render_runner_->CreateContextAsync(std::move(graph_params));

  event_runner_->RunMain();
}

scoped_refptr<base::SequencedTaskRunner> WorkerTreeHost::GetUITaskRunner() {
  return event_runner_->GetUIThreadTaskRunner();
}

scoped_refptr<base::SequencedTaskRunner> WorkerTreeHost::GetRenderTaskRunner() {
  return render_runner_->GetRenderThreadRunner();
}

}  // namespace content
