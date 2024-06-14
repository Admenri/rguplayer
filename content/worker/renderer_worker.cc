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

void RenderRunner::InitRenderer(scoped_refptr<CoreConfigure> config,
                                base::WeakPtr<ui::Widget> host_window) {
  config_ = config;
  host_window_ = host_window;

  worker_ = std::make_unique<base::ThreadWorker>(false);
  worker_->Start(base::RunLoop::MessagePumpType::Worker);
  worker_->WaitUntilStart();

  PostTask(base::BindOnce(&RenderRunner::InitGLContextInternal,
                          base::Unretained(this)));
  WaitForSync();
}

void RenderRunner::DestroyRenderer() {
  PostTask(base::BindOnce(&RenderRunner::QuitGLContextInternal,
                          base::Unretained(this)));
  WaitForSync();
}

void RenderRunner::InitANGLERenderer(CoreConfigure::ANGLERenderer renderer) {
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, SDL_TRUE);

  if (renderer == content::CoreConfigure::ANGLERenderer::DefaultES)
    return;

  SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");
  SDL_GL_SetAttribute(SDL_GL_EGL_PLATFORM, EGL_PLATFORM_ANGLE_ANGLE);
  g_angle_platform.push_back(EGL_PLATFORM_ANGLE_TYPE_ANGLE);
  switch (renderer) {
    case content::CoreConfigure::ANGLERenderer::D3D9:
      g_angle_platform.push_back(EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE);
      break;
    case content::CoreConfigure::ANGLERenderer::D3D11:
      g_angle_platform.push_back(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE);
      break;
    case content::CoreConfigure::ANGLERenderer::Vulkan:
      g_angle_platform.push_back(EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE);
      break;
    case content::CoreConfigure::ANGLERenderer::Metal:
      g_angle_platform.push_back(EGL_PLATFORM_ANGLE_TYPE_METAL_ANGLE);
      break;
    case content::CoreConfigure::ANGLERenderer::Software:
      g_angle_platform.push_back(EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE);
      g_angle_platform.push_back(EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE);
      g_angle_platform.push_back(
          EGL_PLATFORM_ANGLE_DEVICE_TYPE_SWIFTSHADER_ANGLE);
      break;
    default:
      g_angle_platform.push_back(EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE);
      break;
  }
  g_angle_platform.push_back(EGL_NONE);
  SDL_EGL_SetEGLAttributeCallbacks(GetANGLEPlatformCallback, nullptr, nullptr);
}

void RenderRunner::PostTask(base::OnceClosure task) {
  worker_->task_runner()->PostTask(std::move(task));
}

void RenderRunner::WaitForSync() {
  worker_->task_runner()->WaitForSync();
}

void RenderRunner::InitGLContextInternal() {
  glcontext_ = SDL_GL_CreateContext(host_window_->AsSDLWindow());
  SDL_GL_MakeCurrent(host_window_->AsSDLWindow(), glcontext_);
  SDL_GL_SetSwapInterval(0);

  renderer::GLES2Context::ContextParams context_params;
  context_params.enable_vertex_array = true;
  context_params.enable_framebuffer_blit =
      config_->angle_renderer() != content::CoreConfigure::ANGLERenderer::D3D9;
  renderer::GLES2Context::CreateForCurrentThread(context_params);

  if (config_->renderer_debug_output())
    renderer::GLES2Context::EnableDebugOutputForCurrentThread();

  // Always enable opengl es mode for ANGLE backend
  renderer::GSM.enable_es_shaders() = true;
  renderer::GSM.InitStates();
  max_texture_size_ = renderer::GSM.max_texture_size();

  LOG(INFO) << "[Content] GLRenderer: " << renderer::GL.GetString(GL_RENDERER);
  LOG(INFO) << "[Content] GLVendor: " << renderer::GL.GetString(GL_VENDOR);
  LOG(INFO) << "[Content] GLVersion: " << renderer::GL.GetString(GL_VERSION);
  LOG(INFO) << "[Content] GLSL: "
            << renderer::GL.GetString(GL_SHADING_LANGUAGE_VERSION);
  LOG(INFO) << "[Content] MaxTextureSize: " << max_texture_size_ << "x"
            << max_texture_size_;

  renderer::GL.Clear(GL_COLOR_BUFFER_BIT);
  SDL_GL_SwapWindow(host_window_->AsSDLWindow());
}

void RenderRunner::QuitGLContextInternal() {
  renderer::GSM.QuitStates();

  SDL_GL_DeleteContext(glcontext_);
  glcontext_ = nullptr;

  LOG(INFO) << "[Content] Destroy renderer.";
}

}  // namespace content
