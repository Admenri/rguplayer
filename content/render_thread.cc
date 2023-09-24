// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/render_thread.h"

namespace content {

RenderThreadManager* g_render_thread = nullptr;

RenderThreadManager::RenderThreadManager(SDL_Window* sdl_window) {
  render_worker_ = std::make_unique<base::ThreadWorker>("RGU.Graphics.Thread");
  render_worker_->Start(base::RunLoop::MessagePumpType::IO);

  render_worker_->WaitUntilStart();

  render_worker_->task_runner()->PostTask(
      base::BindOnce(&RenderThreadManager::InitThread,
                     weak_ptr_factory_.GetWeakPtr(), sdl_window));

  g_render_thread = this;
}

RenderThreadManager::~RenderThreadManager() {
  render_worker_->task_runner()->PostTask(
      base::BindOnce(&RenderThreadManager::QuitThread,
                     weak_ptr_factory_.GetWeakPtr(), std::move(renderer_cc_)));

  render_worker_.reset();
  g_render_thread = nullptr;
}

RenderThreadManager* RenderThreadManager::GetInstance() {
  return g_render_thread;
}

void RenderThreadManager::InitThread(SDL_Window* sdl_window) {
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  SDL_GLContext glctx = SDL_GL_CreateContext(sdl_window);
  renderer_cc_ = std::make_unique<renderer::CCLayer>(glctx);
}

void RenderThreadManager::QuitThread(std::unique_ptr<renderer::CCLayer> cc) {
  SDL_GLContext glctx = cc->GetSDLGLCtx();
  SDL_GL_DeleteContext(glctx);
}

}  // namespace content
