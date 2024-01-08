// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef CONTENT_WORKER_BINDING_WORKER_H_
#define CONTENT_WORKER_BINDING_WORKER_H_

#include "base/memory/ref_counted.h"
#include "content/public/graphics.h"
#include "content/worker/renderer_worker.h"

#include <thread>

namespace content {

class BindingRunner : public base::RefCounted<BindingRunner> {
 public:
  struct InitParams {
    base::Vec2i initial_resolution;

    base::RepeatingCallback<void(scoped_refptr<BindingRunner>)> binding_boot;

    InitParams() = default;
  };

  BindingRunner();
  ~BindingRunner();

  BindingRunner(const BindingRunner&) = delete;
  BindingRunner& operator=(const BindingRunner&) = delete;

  void InitBindingComponents(const InitParams& params,
                             scoped_refptr<RenderRunner> renderer,
                             const RenderRunner::InitParams& renderer_params);
  void BindingMain();

  bool quit_required() { return quit_req_ && quit_req_->stop_requested(); }
  scoped_refptr<Graphics> graphics() { return graphics_; }

 private:
  static void BindingFuncMain(std::stop_token token,
                              base::WeakPtr<BindingRunner> self);

  scoped_refptr<RenderRunner> renderer_;
  scoped_refptr<Graphics> graphics_;
  std::unique_ptr<std::jthread> runner_thread_;
  InitParams params_;
  RenderRunner::InitParams renderer_params_;
  std::stop_token* quit_req_ = nullptr;

  base::WeakPtrFactory<BindingRunner> weak_ptr_factory_{this};
};

}  // namespace content

#endif  //! CONTENT_WORKER_BINDING_WORKER_H_
