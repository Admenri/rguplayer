// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "renderer/context/gles2_context.h"

#include "SDL_video.h"

namespace renderer {

namespace {

void GL_APIENTRY DebugOutput(GLenum source,
                             GLenum type,
                             GLuint id,
                             GLenum severity,
                             GLsizei length,
                             const GLchar* message,
                             const void* userParam) {
  LOG(INFO) << "[Renderer] Debug Info: " << std::string(message);
}

}  // namespace

// Thread based GLESContext
thread_local GLES2Context GL;

void GLES2Context::CreateForCurrentThread() {
  GL.InitGLESContext();
}

void GLES2Context::EnableDebugOutputForCurrentThread() {
  GL.EnableDebugOutput();
}

void GLES2Context::InitGLESContext() {
  suffix_.clear();
#include "renderer/context/gles2_command_buffer_header_autogen.cc"
#define BIND_GLES_FUN(x) x = reinterpret_cast<decltype(x)>(GetGLProc(#x));

  // VertexArray extension
  if (SDL_GL_ExtensionSupported("GL_ARB_vertex_array_object"))
    suffix_.clear();
  else if (SDL_GL_ExtensionSupported("GL_OES_vertex_array_object"))
    suffix_ = "OES";
  BIND_GLES_FUN(BindVertexArray);
  BIND_GLES_FUN(DeleteVertexArrays);
  BIND_GLES_FUN(GenVertexArrays);
  BIND_GLES_FUN(IsVertexArray);

  if (!GenVertexArrays)
    LOG(INFO) << "[Renderer] Unsupport Vertex Array extension.";

  // FrameBuffer blit
  if (SDL_GL_ExtensionSupported("GL_ARB_framebuffer_object"))
    suffix_.clear();
  else if (SDL_GL_ExtensionSupported("GL_EXT_framebuffer_blit"))
    suffix_ = "EXT";
  else if (SDL_GL_ExtensionSupported("GL_ANGLE_framebuffer_blit"))
    suffix_ = "ANGLE";
  else if (SDL_GL_ExtensionSupported("GL_NV_framebuffer_blit"))
    suffix_ = "NV";
  else
    suffix_.clear();
  BIND_GLES_FUN(BlitFramebuffer);

  if (!BlitFramebuffer)
    LOG(INFO) << "[Renderer] Unsupport FrameBuffer blit extension.";

#undef BIND_GLES_FUN
}

void GLES2Context::EnableDebugOutput() {
  if (SDL_GL_ExtensionSupported("GL_KHR_debug")) {
    Enable(GL_DEBUG_OUTPUT_KHR);

    PFNGLDEBUGMESSAGECALLBACKKHRPROC callback =
        (PFNGLDEBUGMESSAGECALLBACKKHRPROC)GetGLProc("DebugMessageCallbackKHR");

    if (callback) {
      callback(DebugOutput, nullptr);

      LOG(INFO) << "[Renderer] Enable GL Debug Info.";
    }
  }
}

void* GLES2Context::GetGLProc(const std::string& fname) {
  std::string glfname("gl");

  glfname += fname;
  glfname += suffix_;

  void* fptr = reinterpret_cast<void*>(SDL_GL_GetProcAddress(glfname.c_str()));
  return fptr;
}

}  // namespace renderer
