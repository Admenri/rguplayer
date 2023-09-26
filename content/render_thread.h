// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDER_THREAD_H_
#define CONTENT_RENDER_THREAD_H_

#include "base/memory/weak_ptr.h"
#include "base/worker/run_loop.h"
#include "renderer/compositor/renderer_cc.h"
#include "ui/widget/widget.h"

namespace content {

class RenderThreadManager {
 public:
  RenderThreadManager(SDL_Window* sdl_window);
  virtual ~RenderThreadManager();

  RenderThreadManager(const RenderThreadManager&) = delete;
  RenderThreadManager& operator=(const RenderThreadManager&) = delete;

  static void CreateThread(SDL_Window* sdl_window);
  static int RequireStopThread();

  static RenderThreadManager* GetInstance();

  scoped_refptr<base::SequencedTaskRunner> task_runner() {
    if (!render_loop_) return nullptr;
    return render_loop_->task_runner();
  }

  renderer::CCLayer& GetCC() { return *renderer_cc_; }

 private:
  void InitThread();
  void QuitThread();
  void QuitHelper() { render_loop_->QuitWhenIdle(); }

  static int ThreadFunc(void* userdata);

  SDL_Window* sdl_window_;
  std::unique_ptr<renderer::CCLayer> renderer_cc_;
  std::unique_ptr<base::RunLoop> render_loop_;

  base::AtomicFlag sync_start_flag_;

  base::WeakPtrFactory<RenderThreadManager> weak_ptr_factory_{this};
};

}  // namespace content

#endif  // CONTENT_RENDER_THREAD_H_