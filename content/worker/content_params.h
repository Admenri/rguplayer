// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef CONTENT_WORKER_CONTENT_PARAMS_H_
#define CONTENT_WORKER_CONTENT_PARAMS_H_

#include <memory>

#include "base/bind/callback.h"
#include "base/math/math.h"
#include "content/config/core_config.h"
#include "content/engine/binding_engine.h"
#include "ui/widget/widget.h"

namespace content {

struct ContentInitParams {
  bool sync_renderer = false;

  base::WeakPtr<ui::Widget> host_window = nullptr;
  base::Vec2i initial_resolution;

  std::unique_ptr<BindingEngine> binding_engine;
  scoped_refptr<CoreConfigure> config;

  ContentInitParams() = default;
  ContentInitParams(ContentInitParams&&) = default;
  ContentInitParams(const ContentInitParams&) = delete;
  ContentInitParams& operator=(const ContentInitParams&) = delete;
};

}  // namespace content

#endif  // !CONTENT_WORKER_CONTENT_PARAMS_H_
