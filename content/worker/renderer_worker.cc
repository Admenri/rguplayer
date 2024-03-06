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

std::vector<SDL_EGLAttrib> g_angle_attrib;
SDL_EGLAttrib* SDLCALL GetANGLEAttribArray() {
  return g_angle_attrib.data();
}

}  // namespace

void RenderRunner::InitRenderer(scoped_refptr<CoreConfigure> config,
                                base::WeakPtr<ui::Widget> host_window) {
  config_ = config;
  host_window_ = host_window;

  InitGLContextInternal();
}

void RenderRunner::DestroyRenderer() {
  QuitGLContextInternal();
}

void RenderRunner::InitANGLERenderer(CoreConfigure::ANGLERenderer renderer) {
  if (renderer == content::CoreConfigure::ANGLERenderer::Default) {
    LOG(INFO) << "[Renderer] Use default OpenGL driver for renderer.";
    return;
  }

  SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");
  if (renderer == content::CoreConfigure::ANGLERenderer::GLES) {
    LOG(INFO) << "[Renderer] Use OpenGL ES driver for renderer.";
    return;
  }

  g_angle_attrib.push_back(EGL_PLATFORM_ANGLE_TYPE_ANGLE);
  switch (renderer) {
    default:
    case content::CoreConfigure::ANGLERenderer::D3D9:
      LOG(INFO) << "[Renderer] Use ANGLE D3D9 Renderer.";
      g_angle_attrib.push_back(EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE);
      break;
    case content::CoreConfigure::ANGLERenderer::D3D11:
      LOG(INFO) << "[Renderer] Use ANGLE D3D11 Renderer.";
      g_angle_attrib.push_back(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE);
      break;
    case content::CoreConfigure::ANGLERenderer::Vulkan:
      LOG(INFO) << "[Renderer] Use ANGLE Vulkan Renderer.";
      g_angle_attrib.push_back(EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE);
      break;
    case content::CoreConfigure::ANGLERenderer::Metal:
      LOG(INFO) << "[Renderer] Use ANGLE Metal Renderer.";
      g_angle_attrib.push_back(EGL_PLATFORM_ANGLE_TYPE_METAL_ANGLE);
      break;
    case content::CoreConfigure::ANGLERenderer::Software:
      LOG(INFO) << "[Renderer] Use ANGLE Software Renderer.";
      g_angle_attrib.push_back(EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE);
      g_angle_attrib.push_back(EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE);
      g_angle_attrib.push_back(
          EGL_PLATFORM_ANGLE_DEVICE_TYPE_SWIFTSHADER_ANGLE);
      break;
  }
  g_angle_attrib.push_back(EGL_NONE);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_EGL_PLATFORM, EGL_PLATFORM_ANGLE_ANGLE);
  SDL_EGL_SetEGLAttributeCallbacks(GetANGLEAttribArray, nullptr, nullptr);
}

void RenderRunner::InitGLContextInternal() {
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);

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

  renderer::GSM.enable_es_shaders() =
      (config_->angle_renderer() !=
       content::CoreConfigure::ANGLERenderer::Default);
  renderer::GSM.InitStates();
  max_texture_size_ = renderer::GSM.max_texture_size();

  renderer::GL.GetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &vertex_units_);
  renderer::GL.GetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &fragment_units_);
  renderer::GL.GetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,
                           &combined_units_);
  LOG(INFO) << "[Content] MaxVertexTextureUnits: " << vertex_units_;
  LOG(INFO) << "[Content] MaxFragmentTextureUnits: " << fragment_units_;
  LOG(INFO) << "[Content] MaxCombinedTextureUnits: " << combined_units_;

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
