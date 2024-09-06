// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "renderer/context/gles2_context.h"

#include "SDL_loadso.h"
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

GLES2Context GL;

void GLES2Context::CreateForCurrentThread(void* gl_library_handle) {
  GL.InitGLESContext(gl_library_handle);
}

void GLES2Context::EnableDebugOutputForCurrentThread() {
  GL.EnableDebugOutput();
}

void GLES2Context::InitGLESContext(void* gl_library_handle) {
  ogl_library_ = gl_library_handle;
  suffix_.clear();
#include "renderer/context/gles2_command_buffer_header_autogen.cc"
}

void GLES2Context::EnableDebugOutput() {
  if (PFNGLDEBUGMESSAGECALLBACKKHRPROC debug_callback =
          (PFNGLDEBUGMESSAGECALLBACKKHRPROC)GetGLProc("DebugMessageCallback")) {
    Enable(GL_DEBUG_OUTPUT_KHR);

    debug_callback(DebugOutput, nullptr);
    LOG(INFO) << "[Renderer] Enable GL Debug Info.";
  }
}

void* GLES2Context::GetGLProc(const std::string& fname) {
  std::string glfname("gl");

  glfname += fname;
  glfname += suffix_;

  void* fptr = nullptr;
  if (ogl_library_) {
    fptr = (void*)SDL_LoadFunction(ogl_library_, glfname.c_str());
  } else {
    fptr = (void*)SDL_GL_GetProcAddress(glfname.c_str());
  }

  if (!fptr)
    LOG(INFO) << "[Renderer] Error: " << SDL_GetError();

  return fptr;
}

}  // namespace renderer
