#include "gpu/gles2/gles_context.h"

#include "SDL_video.h"
#include "base/debug/debugwriter.h"
#include "base/exceptions/exception.h"

namespace gpu {

GLES2CommandContext::GLES2CommandContext() {}

GLES2CommandContext::~GLES2CommandContext() {}

void GLES2CommandContext::InitContext() {
  // Import defination from autogen file
#include "gles2_command_buffer_autogen.cc"

  // Check for GL version
  std::string gl_version(
      reinterpret_cast<const char*>(this->glGetString(GL_VERSION)));
  is_gles_ = gl_version.find("OpenGL ES") != std::string::npos;
}

void* GLES2CommandContext::GetProc(const std::string& proc_name) {
  void* proc_ptr = SDL_GL_GetProcAddress(proc_name.c_str());

  if (!proc_ptr)
    throw base::Exception::Exception(base::Exception::OpenGLError,
                                     "Cannot find GLES2 proc: %s",
                                     proc_name.c_str());

  return proc_ptr;
}

}  // namespace gpu
