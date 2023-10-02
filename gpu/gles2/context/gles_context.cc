// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/gles2/context/gles_context.h"

#include "SDL_video.h"
#include "base/debug/debugwriter.h"
#include "base/exceptions/exception.h"

namespace gpu {

namespace {

const char kGLESPrefix[] = "OpenGL ES ";

const int kGLESPrefixN = sizeof(kGLESPrefix) - 1;

}  // namespace

GLES2CommandContext::GLES2CommandContext() {}

GLES2CommandContext::~GLES2CommandContext() {}

void GLES2CommandContext::InitContext() {
  is_gles_ = false;
  suffix_.clear();

  // Import defination from autogen file
#include "gpu/gles2/context/gles2_command_buffer_autogen.cc"

  const char* gl_version = (const char*)this->glGetString(GL_VERSION);

  const char glesPrefix[] = "OpenGL ES ";
  const size_t glesPrefixN = sizeof(glesPrefix) - 1;

  if (!strncmp(gl_version, glesPrefix, glesPrefixN)) {
    is_gles_ = true;

    gl_version += glesPrefixN;
  }

  /* Assume single digit */
  int glMajor = *gl_version - '0';

  if (glMajor < 2) {
    base::Debug() << "[OGL] No version match:" << gl_version;
    throw base::Exception(base::Exception::OpenGLError,
                          "At least OpenGL ES 2.0 is required.");
  }

  std::vector<std::string> ext;

  if (glMajor >= 3)
    ParseExtensionsCore(ext);
  else
    ParseExtensionsCompat(ext);

  // EXT: FrameBufferObject
  /* FBO entrypoints */
  {
    if ((glMajor >= 3 || std::find(ext.begin(), ext.end(),
                                   "GL_ARB_framebuffer_object") != ext.end()) ||
        (is_gles_ && glMajor == 2)) {
      suffix_.clear();
    } else if (std::find(ext.begin(), ext.end(), "GL_EXT_framebuffer_object") !=
               ext.end()) {
      suffix_ = "EXT";
    } else {
      throw base::Exception(base::Exception::OpenGLError,
                            "No FBO support available");
    }

#include "gpu/gles2/context/gles2_command_buffer_autogen_ext_fbo.cc"
    suffix_.clear();
  }
}

void* GLES2CommandContext::GetProc(const std::string& proc_name) {
  std::string proc(proc_name);
  if (!suffix_.empty()) proc += suffix_;

  void* proc_ptr = SDL_GL_GetProcAddress(proc.c_str());

  if (!proc_ptr) {
    base::Debug() << "Cannot find OGL proc:" << proc;
    throw base::Exception::Exception(base::Exception::OpenGLError,
                                     "Cannot find OGL proc: %s", proc.c_str());
  }

  return proc_ptr;
}

using _PFNGLGETSTRINGIPROC = const GLubyte*(APIENTRYP)(GLenum, GLuint);
void GLES2CommandContext::ParseExtensionsCore(std::vector<std::string>& out) {
  _PFNGLGETSTRINGIPROC GetStringi =
      (_PFNGLGETSTRINGIPROC)GetProc("glGetStringi");

  GLint extCount = 0;
  this->glGetIntegerv(GL_NUM_EXTENSIONS, &extCount);

  for (GLint i = 0; i < extCount; ++i) {
    out.push_back((const char*)GetStringi(GL_EXTENSIONS, i));
  }
}

void GLES2CommandContext::ParseExtensionsCompat(std::vector<std::string>& out) {
  const char* ext = (const char*)this->glGetString(GL_EXTENSIONS);

  if (!ext) return;

  char buffer[0x100];
  size_t bufferI;

  while (*ext) {
    bufferI = 0;
    while (*ext && *ext != ' ') buffer[bufferI++] = *ext++;

    buffer[bufferI] = '\0';

    out.push_back(buffer);

    if (*ext == ' ') ++ext;
  }
}

}  // namespace gpu
