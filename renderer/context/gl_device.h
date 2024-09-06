// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef RENDERER_CONTEXT_GL_DEVICE_H_
#define RENDERER_CONTEXT_GL_DEVICE_H_

#include "SDL_video.h"

#include <memory>

#define EGL_EGL_PROTOTYPES 1
#include "EGL/egl.h"
#include "EGL/eglext.h"

namespace renderer {

class OGLDevice {
 public:
  enum class ANGLEBackend {
    kDisable = 0,
    kOpenGL,
    kOpenGLES,
    kD3D11,
    kD3D11on12,
    kVulkan,
    kSoftware,
  };

  virtual ~OGLDevice() {}

  virtual bool SwapBuffers() = 0;
  virtual bool SetInterval(int value) = 0;
  virtual bool MakeCurrent(bool null_context) = 0;
  virtual void* GetGLLibrary() = 0;

  static std::unique_ptr<OGLDevice> Create(SDL_Window* host_window,
                                           ANGLEBackend type);
};

class SDLDevice : public OGLDevice {
 public:
  ~SDLDevice() override;

  SDLDevice(const SDLDevice&) = delete;
  SDLDevice& operator=(const SDLDevice&) = delete;

  static std::unique_ptr<SDLDevice> CreateSDLDevice(SDL_Window* host_window);

  bool SwapBuffers() override;
  bool SetInterval(int value) override;
  bool MakeCurrent(bool null_context) override;
  void* GetGLLibrary() override;

 private:
  SDLDevice() = default;

  SDL_GLContext context_ = nullptr;
  SDL_Window* host_ = nullptr;
};

class ANGLEDevice : public OGLDevice {
 public:
  ~ANGLEDevice() override;

  ANGLEDevice(const ANGLEDevice&) = delete;
  ANGLEDevice& operator=(const ANGLEDevice&) = delete;

  static std::unique_ptr<ANGLEDevice> CreateANGLEDevice(SDL_Window* host_window,
                                                        ANGLEBackend type);

  bool SwapBuffers() override;
  bool SetInterval(int value) override;
  bool MakeCurrent(bool null_context) override;
  void* GetGLLibrary() override;

 private:
  ANGLEDevice() = default;
  void GetEGLDisplay(void* nativeDisplay, ANGLEBackend type);

  EGLDisplay display_ = nullptr;
  EGLSurface surface_ = nullptr;
  EGLContext context_ = nullptr;
  SDL_Window* host_ = nullptr;
  void* gl_lib_ = nullptr;
};

}  // namespace renderer

#endif  //! RENDERER_CONTEXT_GL_DEVICE_H_
