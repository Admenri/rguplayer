// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BINDING_BINDING_RUNNER_H_
#define CONTENT_BINDING_BINDING_RUNNER_H_

#include "base/worker/thread_worker.h"
#include "content/script/graphics.h"
#include "content/script/input.h"
#include "ui/widget/widget.h"

namespace content {

class WorkerTreeHost;

class BindingRunner final {
 public:
  struct BindingModules {
    std::unique_ptr<Graphics> graphics;
    std::unique_ptr<Input> input;
  };

  struct InitParams {
    base::WeakPtr<ui::Widget> window;
    base::Vec2i resolution;

    InitParams() = default;
    InitParams(const InitParams&) = delete;
    InitParams(InitParams&&) = default;
  };

  BindingRunner(WorkerTreeHost* tree_host);
  ~BindingRunner();

  BindingRunner(const BindingRunner&) = delete;
  BindingRunner& operator=(const BindingRunner&) = delete;

  void InitAndBootBinding(InitParams initial_param);
  scoped_refptr<base::SequencedTaskRunner> GetTaskRunner();

  static BindingRunner* Get();

  Graphics* GetScreen() { return modules_.graphics.get(); }
  Input* GetInput() { return modules_.input.get(); }

  scoped_refptr<base::SequencedTaskRunner> GetRenderer();

 private:
  void InitInternalModules(const InitParams& initial_param);
  void BindingMain(InitParams initial_param);
  BindingModules modules_;

  WorkerTreeHost* tree_host_;
  std::unique_ptr<base::ThreadWorker> binding_worker_;

  base::WeakPtrFactory<BindingRunner> weak_ptr_factory_{this};
};

}  // namespace content

#endif  // CONTENT_BINDING_BINDING_RUNNER_H_