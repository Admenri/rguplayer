// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/render/render_runner.h"

#include "gpu/gles2/gsm/gles_gsm.h"

namespace content {

RenderRunner::RenderRunner(bool sync)
    : render_worker_(std::make_unique<base::ThreadWorker>(sync)) {}

RenderRunner::~RenderRunner() {
  GetTaskRunner()->PostTask(base::BindOnce(
      &RenderRunner::ReleaseContextInternal, weak_ptr_factory_.GetWeakPtr()));
  GetTaskRunner()->WaitForSync();
}

void RenderRunner::InitGLContext(InitParams inital_params) {
  render_worker_->Start(base::RunLoop::MessagePumpType::Worker);
  render_worker_->WaitUntilStart();

  GetTaskRunner()->PostTask(
      base::BindOnce(&RenderRunner::CreateRenderContextInternal,
                     weak_ptr_factory_.GetWeakPtr(), std::move(inital_params)));

  GetTaskRunner()->WaitForSync();
}

scoped_refptr<base::SequencedTaskRunner> RenderRunner::GetTaskRunner() {
  return render_worker_->task_runner();
}

void RenderRunner::CreateRenderContextInternal(InitParams renderer_settings) {
  glctx_ = SDL_GL_CreateContext(renderer_settings.ogl_window->AsSDLWindow());

  gpu::GL.InitContext();
  gpu::GSM.InitStates();

  SDL_GL_SetSwapInterval(0);
  gpu::GL.Clear(GL_COLOR_BUFFER_BIT);
  SDL_GL_SwapWindow(renderer_settings.ogl_window->AsSDLWindow());
}

void RenderRunner::ReleaseContextInternal() {
  SDL_GL_DeleteContext(glctx_);
  glctx_ = nullptr;
}

}  // namespace content