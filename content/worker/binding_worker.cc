// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/worker/binding_worker.h"

#include "content/config/core_config.h"
#include "content/engine/binding_engine.h"
#include "content/public/font.h"

namespace content {

BindingRunner::BindingRunner() {}

BindingRunner::~BindingRunner() {
  QuitRequired();
}

void BindingRunner::InitBindingComponents(
    ContentInitParams& params,
    scoped_refptr<RenderRunner> renderer) {
  config_ = params.config;
  renderer_ = renderer;
  window_ = params.host_window;
  initial_resolution_ = params.initial_resolution;
  binding_engine_ = std::move(params.binding_engine);
}

void BindingRunner::BindingMain() {
  runner_thread_ = std::make_unique<std::jthread>(
      BindingFuncMain, weak_ptr_factory_.GetWeakPtr());
}

void BindingRunner::QuitRequired() {
  runner_thread_.reset();
}

void BindingRunner::BindingFuncMain(std::stop_token token,
                                    base::WeakPtr<BindingRunner> self) {
  // Init binding thread runner
  self->quit_req_ = &token;
  self->renderer_->InitRenderer(self->config_, self->window_);

  // Init font attributes
  Font::InitStaticFont();

  // Init Modules
  self->graphics_ = new Graphics(self->renderer_, self->initial_resolution_);
  self->input_ = new Input(self->window_);

  // Before run main initialize
  self->binding_engine_->InitializeBinding(self.get());

  // Boot binding components
  self->binding_engine_->RunBindingMain();

  // Destroy and release resource for current worker cc
  self->binding_engine_->FinalizeBinding();
}

}  // namespace content
