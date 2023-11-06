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
  binding_runner_->InitThread();

  /* Run renderer init on binding thread if sync render worker */
  binding_runner_->GetTaskRunner()->PostTask(base::BindOnce(
      &RenderRunner::InitGLContext, base::Unretained(render_runner_.get()),
      std::move(graph_params)));

  /* Execute binding init after other worker init */
  binding_runner_->RunMainAsync(std::move(script_params));

  /* Launch event loop on main thread for message pump */
  event_runner_->RunMain();
}

}  // namespace content
