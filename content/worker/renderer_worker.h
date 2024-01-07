// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef CONTENT_WORKER_RENDERER_WORKER_H_
#define CONTENT_WORKER_RENDERER_WORKER_H_

#include "base/bind/callback.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/worker/run_loop.h"
#include "base/worker/thread_worker.h"

#include "SDL_video.h"

namespace content {

class RenderRunner : public base::SequencedTaskRunner {
 public:
  struct InitParams {
    SDL_Window* target_window = nullptr;

    InitParams() = default;
  };

  RenderRunner(bool sync_worker = false);
  ~RenderRunner();

  RenderRunner(const RenderRunner&) = delete;
  RenderRunner& operator=(const RenderRunner) = delete;

  void InitRenderer(const InitParams& params);

  // base::SequencedTaskRunner methods:
  void PostTask(base::OnceClosure task) override;
  void WaitForSync() override;

  base::WeakPtr<RenderRunner> AsWeakptr() {
    return weak_ptr_factory_.GetWeakPtr();
  }

  SDL_Window* window() { return host_window_; }

 private:
  void InitGLContextInternal(InitParams params);
  void QuitGLContextInternal();

  std::unique_ptr<base::ThreadWorker> worker_;

  SDL_Window* host_window_;
  SDL_GLContext glcontext_;

  base::WeakPtrFactory<RenderRunner> weak_ptr_factory_{this};
};

}  // namespace content

#endif  //! CONTENT_WORKER_RENDERER_WORKER_H_
