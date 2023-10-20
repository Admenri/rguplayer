// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_GLES2_CONTEXT_GLES_CONTEXT_H_
#define GPU_GLES2_CONTEXT_GLES_CONTEXT_H_

#include <SDL_opengl.h>
#include <SDL_opengles2.h>

#include <string>

#include "base/memory/ref_counted.h"
#include "gpu/gles2/context/gles2_apis.h"

namespace gpu {

class GLES2CommandContext;

thread_local extern GLES2CommandContext GL;

class GLES2CommandContext final {
 public:
  GLES2CommandContext();
  ~GLES2CommandContext();

  GLES2CommandContext(const GLES2CommandContext&) = delete;
  GLES2CommandContext& operator=(const GLES2CommandContext&) = delete;

  /* (-except: OGLError-) */
  void InitContext();

 public:
  // Import from autogen header part
#include "gpu/gles2/context/gles2_command_buffer_header_autogen.h"

 private:
  friend class base::RefCountedThreadSafe<GLES2CommandContext>;
  void* GetProc(const std::string& proc_name);

  std::string gl_prefix_;
  std::string gl_suffix_;
};

}  // namespace gpu

#endif  // GPU_GLES2_CONTEXT_GLES_CONTEXT_H_