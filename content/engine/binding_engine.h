// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef CONTENT_ENGINE_BINDING_ENGINE_H_
#define CONTENT_ENGINE_BINDING_ENGINE_H_

#include "base/memory/ref_counted.h"

namespace content {

class BindingRunner;

class BindingEngine {
 public:
  BindingEngine();
  virtual ~BindingEngine();

  BindingEngine(const BindingEngine&) = delete;
  BindingEngine& operator=(const BindingEngine&) = delete;

  virtual void InitializeBinding(scoped_refptr<BindingRunner> binding_host);
  virtual void RunBindingMain();
  virtual void QuitRequired();
  virtual void FinalizeBinding();
};

}  // namespace content

#endif  //! CONTENT_ENGINE_BINDING_ENGINE_H_
