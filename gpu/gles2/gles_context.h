#ifndef GPU_GLES2_GLES_CONTEXT_H_
#define GPU_GLES2_GLES_CONTEXT_H_

#include <SDL_opengl.h>
#include <SDL_opengles2.h>

#include <string>

#include "gles2.h"

namespace gpu {

class GLES2CommandContext final {
 public:
  GLES2CommandContext();
  ~GLES2CommandContext();

  GLES2CommandContext(const GLES2CommandContext&) = delete;
  GLES2CommandContext& operator=(const GLES2CommandContext&) = delete;

  void InitContext();
  bool IsGLES() { return is_gles_; }

 public:
  // Import from autogen header part
#include "gpu/gles2/gles2_command_buffer_header_autogen.h"

 private:
  void* GetProc(const std::string& proc_name);

  bool is_gles_;
};

}  // namespace gpu

#endif  // GPU_GLES2_GLES_CONTEXT_H_