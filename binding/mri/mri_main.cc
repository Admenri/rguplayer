// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "binding/mri/mri_main.h"

#include "content/worker/binding_worker.h"

#include "binding/mri/mri_isolate.h"

namespace binding {

BindingEngineMri::BindingEngineMri() {}

BindingEngineMri::~BindingEngineMri() {}

void BindingEngineMri::InitializeBinding(
    scoped_refptr<content::BindingRunner> binding_host) {
  binding_ = binding_host;

  init_mri_binding(binding_->config()->version());

  LOG(INFO) << "Mri binding initialized.";
}

void BindingEngineMri::RunBindingMain() {
  LOG(INFO) << "Execute Mri binding main loop.";
  while (!binding_->quit_required()) {
    binding_->graphics()->Update();
  }
}

void BindingEngineMri::QuitRequired() {}

void BindingEngineMri::FinalizeBinding() {
  uninit_mri_binding();
  LOG(INFO) << "Quit Mri binding.";
}

}  // namespace binding
