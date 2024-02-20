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
  runner_thread_ =
      std::make_unique<std::thread>(&BindingRunner::BindingFuncMain, this);
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

void BindingRunner::BindingFuncMain() {
  // Attach renderer to binding thread
  renderer_ = new RenderRunner();
  renderer_->InitRenderer(config_, window_);

  // Init I/O filesystem
  file_manager_ = std::make_unique<filesystem::Filesystem>(argv0_);
  file_manager_->AddLoadPath(".");
  for (auto& it : config_->load_paths())
    file_manager_->AddLoadPath(it);

  // Init Modules
  graphics_ = new Graphics(weak_ptr_factory_.GetWeakPtr(), renderer_,
                           initial_resolution_);
  input_ = new Input(window_);
  audio_ = new Audio(file_manager_->AsWeakPtr(), config_);

  // Before run main initialize
  binding_engine_->InitializeBinding(this);

  // Boot binding components
  binding_engine_->RunBindingMain();

  // Destroy and release resource for current worker cc
  binding_engine_->FinalizeBinding();
  binding_engine_.reset();

  // Release content module
  graphics_.reset();
  input_.reset();
  audio_.reset();

  // Destroy renderer on binding thread
  renderer_->DestroyRenderer();

  // Release I/O filesystem
  file_manager_.reset();

  // Quit app required
  SDL_Event quit_event;
  quit_event.type = user_event_id_ + EventRunner::QUIT_SYSTEM_EVENT;
  SDL_PushEvent(&quit_event);
}

}  // namespace content
