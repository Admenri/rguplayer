// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/worker/content_compositor.h"

namespace content {

WorkerTreeCompositor::WorkerTreeCompositor()
    : share_data_(std::make_unique<WorkerShareData>()) {}

WorkerTreeCompositor::~WorkerTreeCompositor() {
  binding_runner_->RequestQuit();
  binding_runner_.reset();
  event_runner_.reset();
  share_data_.reset();
}

void WorkerTreeCompositor::InitCC(ContentInitParams params) {
  share_data_->config = params.config;
  share_data_->window = params.host_window;

  event_runner_ = new EventRunner(share_data_.get());
  binding_runner_ = new BindingRunner(share_data_.get());

  // Init event runner on main thread
  event_runner_->InitDispatcher(binding_runner_->AsWeakPtr());

  // Init renderer in binding thread for sync mode
  binding_runner_->InitBindingComponents(params);
}

void WorkerTreeCompositor::ContentMain() {
  // Launch script thread
  binding_runner_->BindingMain();

  // Launch event loop
  event_runner_->EventMain();
}

}  // namespace content
