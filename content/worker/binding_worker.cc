// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/worker/binding_worker.h"

#include "content/config/core_config.h"
#include "content/engine/binding_engine.h"
#include "content/public/font.h"
#include "content/worker/event_runner.h"

namespace content {

void BindingRunner::InitBindingComponents(ContentInitParams& params) {
  argv0_ = params.argv0;
  config_ = params.config;
  window_ = params.host_window->AsWeakPtr();
  initial_resolution_ = params.initial_resolution;
  binding_engine_ = std::move(params.binding_engine);
}

void BindingRunner::BindingMain(uint32_t event_id) {
  user_event_id_ = event_id;
  runner_thread_ = std::make_unique<std::thread>(
      BindingFuncMain, weak_ptr_factory_.GetWeakPtr());
}

void BindingRunner::RequestQuit() {
  quit_atomic_.Set();
  runner_thread_->join();

  runner_thread_.reset();
}

void BindingRunner::RequestReset() {
  reset_atomic_.Set();
}

void BindingRunner::ClearResetFlag() {
  reset_atomic_.UnsafeResetForTesting();
}

bool BindingRunner::CheckFlags() {
  bool quit_required = false;
  quit_required |= quit_atomic_.IsSet();
  quit_required |= reset_atomic_.IsSet();

  return quit_required;
}

void BindingRunner::RaiseFlags() {
  if (!binding_engine_)
    return;

  if (quit_atomic_.IsSet())
    binding_engine_->QuitRequired();
  if (reset_atomic_.IsSet())
    binding_engine_->ResetRequired();
}

void BindingRunner::BindingFuncMain(base::WeakPtr<BindingRunner> self) {
  // Attach renderer to binding thread
  self->renderer_ = new RenderRunner();
  self->renderer_->InitRenderer(self->config_, self->window_);

  // Init I/O filesystem
  self->file_manager_ = std::make_unique<filesystem::Filesystem>(self->argv0_);
  for (auto& it : self->config_->load_paths())
    self->file_manager_->AddLoadPath(it);

  // Init Modules
  self->graphics_ =
      new Graphics(self.get(), self->renderer_, self->initial_resolution_);
  self->input_ = new Input(self->window_);

  // Before run main initialize
  self->binding_engine_->InitializeBinding(self.get());

  // Boot binding components
  self->binding_engine_->RunBindingMain();

  // Destroy and release resource for current worker cc
  self->binding_engine_->FinalizeBinding();

  // Release content module
  self->graphics_.reset();
  self->input_.reset();

  // Destroy renderer on binding thread
  self->renderer_->DestroyRenderer();

  // Release I/O filesystem
  self->file_manager_.reset();

  // Quit app required
  SDL_Event quit_event;
  quit_event.type = self->user_event_id_ + EventRunner::QUIT_SYSTEM_EVENT;
  SDL_PushEvent(&quit_event);
}

}  // namespace content
