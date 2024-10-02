// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/worker/binding_worker.h"

#include "content/config/core_config.h"
#include "content/engine/binding_engine.h"
#include "content/public/font.h"
#include "content/worker/event_runner.h"

namespace content {

BindingRunner::BindingRunner(WorkerShareData* share_data)
    : share_data_(share_data), quit_atomic_(false), reset_atomic_(false) {}

void BindingRunner::InitBindingComponents(ContentInitParams& params) {
  binding_engine_ = std::move(params.binding_engine);
}

void BindingRunner::BindingMain() {
  runner_thread_ =
      std::make_unique<std::thread>(&BindingRunner::BindingFuncMain, this);
}

void BindingRunner::RequestQuit() {
  quit_atomic_.store(true);
  runner_thread_->join();
  runner_thread_.reset();
}

void BindingRunner::RequestReset() {
  reset_atomic_.store(true);
}

void BindingRunner::ClearResetFlag() {
  reset_atomic_.store(false);
}

bool BindingRunner::CheckRunnerFlags() {
  bool quit_required = false;
  quit_required |= quit_atomic_.load();
  quit_required |= reset_atomic_.load();

  return quit_required;
}

void BindingRunner::RaiseRunnerFlags() {
  if (!binding_engine_)
    return;

  if (quit_atomic_.load())
    binding_engine_->QuitRequired();
  if (reset_atomic_.load())
    binding_engine_->ResetRequired();
}

void BindingRunner::BindingFuncMain() {
  // Attach renderer to binding thread
  renderer_ = new RenderRunner();

  if (renderer_->InitRenderer(share_data_->config, share_data_->window)) {
    // Init Modules
    graphics_ =
        new Graphics(share_data_, weak_ptr_factory_.GetWeakPtr(), renderer_,
                     share_data_->config->initial_resolution());
    input_ = new Input(share_data_);
    audio_ = new Audio(share_data_);
    mouse_ = new Mouse(share_data_);
    touch_ = new Touch(share_data_);

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
    mouse_.reset();
    touch_.reset();

    // Destroy renderer on binding thread
    renderer_->DestroyRenderer();
  } else {
    // Error when loading renderer
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "RGU Kernel",
                             "Failed to load renderer device.",
                             share_data_->window->AsSDLWindow());
  }

  // Quit app required
  SDL_Event quit_event;
  quit_event.type = share_data_->user_event_id + EventRunner::QUIT_SYSTEM_EVENT;
  SDL_PushEvent(&quit_event);
}

}  // namespace content
