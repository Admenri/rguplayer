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
#include "content/config/core_config.h"
#include "ui/widget/widget.h"

namespace content {

class CoreConfigure;

class RenderRunner : public base::SequencedTaskRunner {
 public:
  RenderRunner() = default;

  RenderRunner(const RenderRunner&) = delete;
  RenderRunner& operator=(const RenderRunner) = delete;

  void InitRenderer(scoped_refptr<CoreConfigure> config,
                    base::WeakPtr<ui::Widget> host_window);
  void DestroyRenderer();

  void PostTask(base::OnceClosure task) override;
  void WaitForSync() override;

  int max_texture_size() const { return max_texture_size_; }
  int vertex_texture_units() const { return vertex_units_; }
  int fragment_texture_units() const { return fragment_units_; }
  int combine_texture_units() const { return combined_units_; }
  base::WeakPtr<ui::Widget> window() const { return host_window_; }

  static void InitANGLERenderer(CoreConfigure::ANGLERenderer renderer);

 private:
  void InitGLContextInternal();
  void QuitGLContextInternal();

  scoped_refptr<CoreConfigure> config_;
  std::unique_ptr<base::ThreadWorker> worker_;

  base::WeakPtr<ui::Widget> host_window_;
  SDL_GLContext glcontext_;
  int max_texture_size_;
  int vertex_units_, fragment_units_, combined_units_;

  base::WeakPtrFactory<RenderRunner> weak_ptr_factory_{this};
};

}  // namespace content

#endif  //! CONTENT_WORKER_RENDERER_WORKER_H_
