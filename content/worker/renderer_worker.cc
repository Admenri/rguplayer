// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/worker/renderer_worker.h"

#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "SDL_hints.h"
#include "content/config/core_config.h"
#include "renderer/context/gles2_context.h"
#include "renderer/states/draw_states.h"
#include "renderer/thread/thread_manager.h"

namespace content {

namespace {

std::vector<SDL_EGLAttrib> g_angle_platform;
SDL_EGLAttrib* SDLCALL GetANGLEPlatformCallback() {
  return g_angle_platform.data();
}

}  // namespace

bool RenderRunner::InitRenderer(scoped_refptr<CoreConfigure> config,
                                base::WeakPtr<ui::Widget> host_window) {
  config_ = config;
  host_window_ = host_window;

  worker_ = std::make_unique<base::ThreadWorker>(!config->async_renderer());
  worker_->Start(base::RunLoop::MessagePumpType::Worker);
  worker_->WaitUntilStart();

  PostTask(base::BindOnce(&RenderRunner::InitGLContextInternal,
                          base::Unretained(this)));
  WaitForSync();

  return !!glcontext_.get();
}

void RenderRunner::DestroyRenderer() {
  if (!glcontext_)
    return;

  PostTask(base::BindOnce(&RenderRunner::QuitGLContextInternal,
                          base::Unretained(this)));
  WaitForSync();
}

void RenderRunner::PostTask(base::OnceClosure task) {
  worker_->task_runner()->PostTask(std::move(task));
}

void RenderRunner::WaitForSync() {
  worker_->task_runner()->WaitForSync();
}

void RenderRunner::InitGLContextInternal() {
  glcontext_ = renderer::OGLDevice::Create(
      host_window_->AsSDLWindow(),
      (renderer::OGLDevice::ANGLEBackend)config_->angle_renderer());
  if (!glcontext_)
    return;

  glcontext_->MakeCurrent(false);
  glcontext_->SetInterval(0);

  renderer::GLES2Context::CreateForCurrentThread(glcontext_->GetGLLibrary());
  if (config_->renderer_debug_output())
    renderer::GLES2Context::EnableDebugOutputForCurrentThread();

  if (config_->async_renderer())
    LOG(INFO) << "[Content] Running renderer thread.";
  LOG(INFO) << "[Content] GLRenderer: " << renderer::GL.GetString(GL_RENDERER);
  LOG(INFO) << "[Content] GLVendor: " << renderer::GL.GetString(GL_VENDOR);
  LOG(INFO) << "[Content] GLVersion: " << renderer::GL.GetString(GL_VERSION);
  LOG(INFO) << "[Content] GLSL: "
            << renderer::GL.GetString(GL_SHADING_LANGUAGE_VERSION);

  renderer::GSM.InitStates(config_->angle_renderer() !=
                           CoreConfigure::ANGLEBackend::kDisable);
  max_texture_size_ = renderer::GSM.max_texture_size();
  LOG(INFO) << "[Content] MaxTextureSize: " << max_texture_size_ << "x"
            << max_texture_size_;

  renderer::GL.Clear(GL_COLOR_BUFFER_BIT);
  glcontext_->SwapBuffers();
}

void RenderRunner::QuitGLContextInternal() {
  renderer::GSM.QuitStates();
  glcontext_.reset();

  LOG(INFO) << "[Content] Destroy renderer.";
}

}  // namespace content
