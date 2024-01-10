// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "renderer/context/gles2_context.h"

#include "SDL_video.h"

namespace renderer {

// Thread based GLESContext
thread_local GLES2Context GL;

void GLES2Context::CreateForCurrentThread() {
  GL.InitGLESContext();
}

void GLES2Context::InitGLESContext() {
#include "renderer/context/gles2_command_buffer_header_autogen.cc"
}

void* GLES2Context::GetGLProc(const std::string& fname) {
  std::string glfname;

  glfname += "gl";
  glfname += fname;

  void* fptr = SDL_GL_GetProcAddress(glfname.c_str());

  return fptr;
}

}  // namespace renderer
