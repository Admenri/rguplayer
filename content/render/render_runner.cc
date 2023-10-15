// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/render/render_runner.h"

#include "gpu/gles2/gsm/gles_gsm.h"

namespace content {

RenderRunner* g_render_runner = nullptr;

RenderRunner::RenderRunner(bool sync)
    : render_worker_(std::make_unique<base::ThreadWorker>(sync)) {
  g_render_runner = this;
}

RenderRunner::~RenderRunner() {
  task_runner_->PostTask(base::BindOnce(&RenderRunner::ReleaseContextInternal,
                                        weak_ptr_factory_.GetWeakPtr()));
  task_runner_->WaitForSync();
  g_render_runner = nullptr;
}

RenderRunner* RenderRunner::GetInstance() { return g_render_runner; }

void RenderRunner::CreateContextAsync(InitParams renderer_settings) {
  render_worker_->Start(base::RunLoop::MessagePumpType::IO);
  render_worker_->WaitUntilStart();
  task_runner_ = render_worker_->task_runner();

  task_runner_->PostTask(base::BindOnce(
      &RenderRunner::CreateRenderContextInternal,
      weak_ptr_factory_.GetWeakPtr(), std::move(renderer_settings)));
}

scoped_refptr<base::SequencedTaskRunner> RenderRunner::GetRenderThreadRunner() {
  return task_runner_;
}

void RenderRunner::CreateRenderContextInternal(InitParams renderer_settings) {
  glctx_ = SDL_GL_CreateContext(renderer_settings.ogl_window->AsSDLWindow());

  gpu::GL.InitContext();
  gpu::GSM.InitStates();

  gpu::GL.ClearColor(0, 1, 0, 1);
  gpu::GL.Clear(GL_COLOR_BUFFER_BIT);
  SDL_GL_SwapWindow(renderer_settings.ogl_window->AsSDLWindow());
}

void RenderRunner::ReleaseContextInternal() {
  SDL_GL_DeleteContext(glctx_);
  glctx_ = nullptr;
}

}  // namespace content