// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/scheduler/worker_cc.h"

namespace content {

WorkerTreeHost::WorkerTreeHost(bool sync_worker)
    : event_runner_(new EventRunner()),
      render_runner_(new RenderRunner(sync_worker)),
      binding_runner_(new BindingRunner(this)) {}

WorkerTreeHost::~WorkerTreeHost() {
  binding_runner_.reset();
  render_runner_.reset();
}

void WorkerTreeHost::Run(RenderRunner::InitParams graph_params,
                         BindingRunner::InitParams script_params) {
  render_runner_->InitGLContext(std::move(graph_params));
  binding_runner_->InitAndBootBinding(std::move(script_params));

  event_runner_->RunMain();
}

}  // namespace content
