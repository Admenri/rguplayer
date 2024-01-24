// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/worker/renderer_worker.h"

#include "content/config/core_config.h"
#include "renderer/context/gles2_context.h"
#include "renderer/states/draw_states.h"
#include "renderer/thread/thread_manager.h"

namespace content {

RenderRunner::RenderRunner(bool sync_worker) {
  worker_ = std::make_unique<base::ThreadWorker>(sync_worker);
}

RenderRunner::~RenderRunner() {
  QuitRequired();
}

void RenderRunner::InitRenderer(scoped_refptr<CoreConfigure> config,
                                base::WeakPtr<ui::Widget> host_window) {
  config_ = config;
  worker_->Start(base::RunLoop::MessagePumpType::Worker);
  worker_->WaitUntilStart();
  host_window_ = host_window;

  PostTask(base::BindOnce(&RenderRunner::InitGLContextInternal, AsWeakptr()));
  WaitForSync();
}

void RenderRunner::QuitRequired() {
  WaitForSync();
  PostTask(base::BindOnce(&RenderRunner::QuitGLContextInternal, AsWeakptr()));
  WaitForSync();

  worker_.reset();
}

void RenderRunner::PostTask(base::OnceClosure task) {
  worker_->task_runner()->PostTask(std::move(task));
}

void RenderRunner::WaitForSync() {
  if (worker_->IsSyncMode())
    return;
  worker_->task_runner()->WaitForSync();
}

void RenderRunner::InitGLContextInternal() {
  glcontext_ = SDL_GL_CreateContext(host_window_->AsSDLWindow());
  SDL_GL_MakeCurrent(host_window_->AsSDLWindow(), glcontext_);
  SDL_GL_SetSwapInterval(0);

  renderer::GLES2Context::CreateForCurrentThread();
  renderer::GSM.InitStates();

  renderer::GL.Clear(GL_COLOR_BUFFER_BIT);
  SDL_GL_SwapWindow(host_window_->AsSDLWindow());
}

void RenderRunner::QuitGLContextInternal() {
  renderer::GSM.QuitStates();

  SDL_GL_DeleteContext(glcontext_);
  glcontext_ = nullptr;
}

}  // namespace content
