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
    const InitParams& params,
    scoped_refptr<RenderRunner> renderer,
    const RenderRunner::InitParams& renderer_params) {
  renderer_ = renderer;
  params_ = params;
  renderer_params_ = renderer_params;
}

void BindingRunner::BindingMain() {
  runner_thread_ = std::make_unique<std::jthread>(
      BindingFuncMain, weak_ptr_factory_.GetWeakPtr());
}

void BindingRunner::BindingFuncMain(std::stop_token token,
                                    base::WeakPtr<BindingRunner> self) {
  // Init binding thread runner
  self->quit_req_ = &token;
  self->renderer_->InitRenderer(self->renderer_params_);
  self->graphics_ =
      new Graphics(self->renderer_, self->params_.initial_resolution);

  // Boot binding external components
  self->params_.binding_boot.Run(self.get());
}

}  // namespace content
