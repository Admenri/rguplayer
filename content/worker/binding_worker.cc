// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/worker/binding_worker.h"

namespace content {

BindingRunner::BindingRunner() {}

BindingRunner::~BindingRunner() {
  if (runner_thread_)
    runner_thread_->join();
}

void BindingRunner::InitBindingComponents(
    const ContentInitParams& params,
    scoped_refptr<RenderRunner> renderer) {
  renderer_ = renderer;
  window_ = params.host_window;
  initial_resolution_ = params.initial_resolution;
  binding_main_ = std::move(params.binding_boot);
}

void BindingRunner::BindingMain() {
  runner_thread_ = std::make_unique<std::jthread>(
      BindingFuncMain, weak_ptr_factory_.GetWeakPtr());
}

void BindingRunner::BindingFuncMain(std::stop_token token,
                                    base::WeakPtr<BindingRunner> self) {
  // Init binding thread runner
  self->quit_req_ = &token;
  self->renderer_->InitRenderer(self->window_);

  self->graphics_ = new Graphics(self->renderer_, self->initial_resolution_);
  self->input_ = new Input(self->window_);

  // Boot binding external components
  std::move(self->binding_main_).Run(self.get());
}

}  // namespace content
