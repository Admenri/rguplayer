// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef RENDERER_CONTEXT_GLES2_CONTEXT_H_
#define RENDERER_CONTEXT_GLES2_CONTEXT_H_

#include "base/memory/ref_counted.h"

#include "GLES2/gl2.h"
#include "GLES2/gl2ext.h"

#include <string>

namespace renderer {

class GLES2Context;

extern thread_local GLES2Context GL;

class GLES2Context {
 public:
  GLES2Context() = default;
  ~GLES2Context() = default;

  GLES2Context(const GLES2Context&) = delete;
  GLES2Context& operator=(const GLES2Context&) = delete;

  // Create the GLESContext on current thread
  static void CreateForCurrentThread();

 public:
  // Import from autogen-commands
#include "renderer/context/gles2_command_buffer_header_autogen.h"

  // OES Extensions
  bool OES_TextureNpot = false;

 private:
  void InitGLESContext();
  void* GetGLProc(const std::string& fname);
};

}  // namespace renderer

#endif  //! RENDERER_CONTEXT_GLES2_CONTEXT_H_
