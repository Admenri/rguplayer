// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDER_THREAD_H_
#define CONTENT_RENDER_THREAD_H_

#include "base/memory/weak_ptr.h"
#include "base/worker/thread_worker.h"
#include "renderer/compositor/renderer_cc.h"
#include "ui/widget/widget.h"

namespace content {

class RenderThreadManager {
 public:
  RenderThreadManager(SDL_Window* sdl_window);
  virtual ~RenderThreadManager();

  RenderThreadManager(const RenderThreadManager&) = delete;
  RenderThreadManager& operator=(const RenderThreadManager&) = delete;

  static RenderThreadManager* GetInstance();

  scoped_refptr<base::SequencedTaskRunner> task_runner() {
    return render_worker_->task_runner();
  }

  renderer::CCLayer& GetCC() { return *renderer_cc_; }

 private:
  void InitThread(SDL_Window* sdl_window);
  void QuitThread(std::unique_ptr<renderer::CCLayer> cc);

  std::unique_ptr<renderer::CCLayer> renderer_cc_;
  std::unique_ptr<base::ThreadWorker> render_worker_;

  base::WeakPtrFactory<RenderThreadManager> weak_ptr_factory_{this};
};

}  // namespace content

#endif  // CONTENT_RENDER_THREAD_H_