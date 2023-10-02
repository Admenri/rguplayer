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

class GLES2CommandContext
    : public base::RefCountedThreadSafe<GLES2CommandContext> {
 public:
  GLES2CommandContext();
  virtual ~GLES2CommandContext();

  GLES2CommandContext(const GLES2CommandContext&) = delete;
  GLES2CommandContext& operator=(const GLES2CommandContext&) = delete;

  void InitContext();
  bool IsGLES() { return is_gles_; }

 public:
  // Import from autogen header part
#include "gpu/gles2/context/gles2_command_buffer_header_autogen.h"

// FrameBufferObject EXT
#include "gpu/gles2/context/gles2_command_buffer_header_autogen_ext_fbo.h"

 private:
  friend class base::RefCountedThreadSafe<GLES2CommandContext>;
  void* GetProc(const std::string& proc_name);

  void ParseExtensionsCore(std::vector<std::string>& out);
  void ParseExtensionsCompat(std::vector<std::string>& out);

  bool is_gles_ = false;
  std::string suffix_;
};

}  // namespace gpu

#endif  // GPU_GLES2_CONTEXT_GLES_CONTEXT_H_