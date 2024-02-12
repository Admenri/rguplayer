// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/worker/content_compositor.h"

namespace content {

WorkerTreeCompositor::WorkerTreeCompositor() {}

WorkerTreeCompositor::~WorkerTreeCompositor() {
  binding_runner_->RequestQuit();
  binding_runner_.reset();
  event_runner_.reset();
  audio_runner_.reset();
}

void WorkerTreeCompositor::InitCC(ContentInitParams params) {
  config_ = params.config;

  event_runner_ = new EventRunner();
  binding_runner_ = new BindingRunner();
  audio_runner_ = new AudioRunner();

  // Init event runner on main thread
  event_runner_->InitEventDispatcher(config_, params.host_window->AsWeakPtr(),
                                     binding_runner_->AsWeakPtr());

  // Init audio device
  audio_runner_->InitAudioComponents(config_);

  // Init renderer in binding thread for sync mode
  binding_runner_->InitBindingComponents(params);
}

void WorkerTreeCompositor::ContentMain() {
  // Launch script thread
  binding_runner_->BindingMain(event_runner_->user_event_id(), audio_runner_);

  // Launch event loop
  event_runner_->EventMain();
}

}  // namespace content
