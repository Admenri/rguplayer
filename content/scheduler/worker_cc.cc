// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/scheduler/worker_cc.h"

namespace content {

WorkerTreeHost::WorkerTreeHost(bool sync_worker) {
  event_runner_ = std::make_unique<EventRunner>();
  render_runner_ = std::make_unique<RenderRunner>(sync_worker);
}

WorkerTreeHost::~WorkerTreeHost() { render_runner_.reset(); }

WorkerTreeHost* WorkerTreeHost::GetInstance() { return nullptr; }

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
