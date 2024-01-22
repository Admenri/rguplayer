// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef CONTENT_WORKER_CONTENT_PARAMS_H_
#define CONTENT_WORKER_CONTENT_PARAMS_H_

#include "base/bind/callback.h"
#include "base/math/math.h"
#include "ui/widget/widget.h"

namespace content {

class BindingRunner;

struct ContentInitParams {
  bool sync_renderer = false;

  base::WeakPtr<ui::Widget> host_window = nullptr;
  base::Vec2i initial_resolution;

  base::RepeatingCallback<void(scoped_refptr<BindingRunner>)> binding_boot;

  ContentInitParams() = default;
  ContentInitParams(ContentInitParams&&) = default;
};

}  // namespace content

#endif  // !CONTENT_WORKER_CONTENT_PARAMS_H_
