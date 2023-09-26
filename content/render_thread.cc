// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/render_thread.h"

#include <SDL_thread.h>
#include <SDL_timer.h>

namespace content {

namespace {

const char kRendererThreadName[] = "RGU.Renderer.Thread";

RenderThreadManager* g_render_thread = nullptr;
SDL_Thread* g_render_thread_sdl = nullptr;

}  // namespace

RenderThreadManager::RenderThreadManager(SDL_Window* sdl_window)
    : sdl_window_(sdl_window) {
  g_render_thread = this;
}

RenderThreadManager::~RenderThreadManager() {
  QuitThread();

  g_render_thread = nullptr;
}

void RenderThreadManager::CreateThread(SDL_Window* sdl_window) {
  if (!g_render_thread_sdl) {
    auto render_obj = new RenderThreadManager(sdl_window);
    g_render_thread_sdl = SDL_CreateThread(RenderThreadManager::ThreadFunc,
                                           kRendererThreadName, render_obj);

    while (!render_obj->sync_start_flag_.IsSet()) {
      SDL_Delay(1);
    }
  }
}

int RenderThreadManager::RequireStopThread() {
  int status = 0;
  if (g_render_thread) {
    g_render_thread->task_runner()->PostTask(base::BindOnce(
        &RenderThreadManager::QuitHelper, base::Unretained(g_render_thread)));

    SDL_WaitThread(g_render_thread_sdl, &status);
  }
  return status;
}

RenderThreadManager* RenderThreadManager::GetInstance() {
  return g_render_thread;
}

void RenderThreadManager::InitThread() {
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  SDL_GLContext glctx = SDL_GL_CreateContext(sdl_window_);
  renderer_cc_ = std::make_unique<renderer::CCLayer>(glctx);
}

void RenderThreadManager::QuitThread() {
  SDL_GLContext glctx = renderer_cc_->GetSDLGLCtx();
  SDL_GL_DeleteContext(glctx);
}

int RenderThreadManager::ThreadFunc(void* userdata) {
  std::unique_ptr<RenderThreadManager> renderer_manager(
      static_cast<RenderThreadManager*>(userdata));
  renderer_manager->InitThread();

  renderer_manager->render_loop_ = std::make_unique<base::RunLoop>();
  renderer_manager->sync_start_flag_.Set();

  renderer_manager->render_loop_->Run();

  // Release
  renderer_manager.reset();

  return 0;
}

}  // namespace content
