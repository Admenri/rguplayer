// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDER_THREAD_H_
#define CONTENT_RENDER_THREAD_H_

#include <thread>

#include "base/exceptions/exception.h"
#include "base/memory/weak_ptr.h"
#include "base/worker/run_loop.h"
#include "base/worker/thread_worker.h"
#include "renderer/compositor/renderer_cc.h"

namespace content {

class RendererThread : public base::RefCountedThreadSafe<RendererThread> {
 public:
  RendererThread();
  virtual ~RendererThread();

  RendererThread(const RendererThread&) = delete;
  RendererThread& operator=(const RendererThread&) = delete;

  // Init gl context and graphics device
  void InitContextAsync(SDL_Window* render_canvas);

  // Only valid after initialize
  // Maybe nullptr
  renderer::CCLayer* GetCC() { return renderer_cc_.get(); }

  // Get render thread task runner
  scoped_refptr<base::SequencedTaskRunner> GetRenderThreadRunner();

  // Get cc for current render thread
  static renderer::CCLayer* GetCCForRenderer();

 private:
  friend class base::RefCountedThreadSafe<RendererThread>;
  void InitThread();
  void QuitThread();

  SDL_Window* render_widget_;
  std::unique_ptr<renderer::CCLayer> renderer_cc_;

  std::unique_ptr<base::ThreadWorker> thread_;
  SDL_GLContext gl_context_ = nullptr;

  base::WeakPtrFactory<RendererThread> weak_ptr_factory_{this};
};

}  // namespace content

#endif  // CONTENT_RENDER_THREAD_H_