// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/engine/binding_engine.h"

#include "content/worker/binding_worker.h"

namespace content {

BindingEngine::BindingEngine() {}

BindingEngine::~BindingEngine() {}

void BindingEngine::InitializeBinding(
    scoped_refptr<BindingRunner> binding_host) {
  LOG(INFO) << "Empty InitializeBinding implement for BindingEngine.";
}

void BindingEngine::RunBindingMain() {
  LOG(INFO) << "Empty RunBindingMain implement for BindingEngine.";
}

void BindingEngine::QuitRequired() {
  LOG(INFO) << "Empty QuitRequired implement for BindingEngine.";
}

void BindingEngine::FinalizeBinding() {
  LOG(INFO) << "Empty FinalizeBinding implement for BindingEngine.";
}

}  // namespace content
