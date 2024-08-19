// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "renderer/context/gles2_context.h"

#include "SDL_video.h"

namespace renderer {

namespace {

void GL_APIENTRY DebugOutput(GLenum,
                             GLenum,
                             GLuint,
                             GLenum,
                             GLsizei,
                             const GLchar* message,
                             const void*) {
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
#include "renderer/context/gles_command_buffer_define_autogen.inc"
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
  if (!fptr)
    LOG(INFO) << "[Renderer] Error: " << SDL_GetError();

  return fptr;
}

}  // namespace renderer
