// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/worker/content_compositor.h"
#include "content_compositor.h"

namespace content {

WorkerTreeCompositor::WorkerTreeCompositor() {}

WorkerTreeCompositor::~WorkerTreeCompositor() {
  binding_runner_.reset();
  render_runner_.reset();
}

void WorkerTreeCompositor::InitCC(const InitParams& params) {
  event_runner_ = new EventRunner();
  render_runner_ = new RenderRunner(params.sync_renderer);
  binding_runner_ = new BindingRunner();

  event_runner_->InitEventDispatcher();

  // Init renderer in binding thread for sync mode
  binding_runner_->InitBindingComponents(params.binding_params, render_runner_,
                                         params.renderer_params);
}

void WorkerTreeCompositor::ContentMain() {
  // Launch script thread
  binding_runner_->BindingMain();

  // Launch event loop
  event_runner_->EventMain();
}

}  // namespace content
