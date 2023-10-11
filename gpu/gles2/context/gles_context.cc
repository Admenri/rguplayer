// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/gles2/context/gles_context.h"

#include "SDL_video.h"
#include "base/debug/debugwriter.h"
#include "base/exceptions/exception.h"

namespace gpu {

GLES2CommandContext GL;

GLES2CommandContext::GLES2CommandContext() {}

GLES2CommandContext::~GLES2CommandContext() {}

void GLES2CommandContext::InitContext() {
  gl_prefix_ = "gl";

  // Import defination from autogen file
#include "gpu/gles2/context/gles2_command_buffer_autogen.cc"
}

void* GLES2CommandContext::GetProc(const std::string& proc_name) {
  std::string proc(gl_prefix_);
  proc += proc_name;
  proc += gl_suffix_;

  void* proc_ptr = SDL_GL_GetProcAddress(proc.c_str());

  if (!proc_ptr) {
    base::Debug() << "Cannot find OGL proc:" << proc_name;
    throw base::Exception::Exception(base::Exception::OpenGLError,
                                     "Cannot find OGL proc: %s",
                                     proc_name.c_str());
  }

  return proc_ptr;
}

}  // namespace gpu
