// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "renderer/context/gl_device.h"

#include "base/debug/logging.h"
#include "renderer/context/gles2_context.h"

#include "SDL_loadso.h"

#include <vector>

namespace {

#define EGL_IMPORT                                                          \
  EGL_IMPORT_FUNC(PFNEGLBINDAPIPROC, eglBindAPI);                           \
  EGL_IMPORT_FUNC(PFNEGLCHOOSECONFIGPROC, eglChooseConfig);                 \
  EGL_IMPORT_FUNC(PFNEGLCREATECONTEXTPROC, eglCreateContext);               \
  EGL_IMPORT_FUNC(PFNEGLCREATEPBUFFERSURFACEPROC, eglCreatePbufferSurface); \
  EGL_IMPORT_FUNC(PFNEGLCREATEWINDOWSURFACEPROC, eglCreateWindowSurface);   \
  EGL_IMPORT_FUNC(PFNEGLDESTROYCONTEXTPROC, eglDestroyContext);             \
  EGL_IMPORT_FUNC(PFNEGLDESTROYSURFACEPROC, eglDestroySurface);             \
  EGL_IMPORT_FUNC(PFNEGLGETCURRENTCONTEXTPROC, eglGetCurrentContext);       \
  EGL_IMPORT_FUNC(PFNEGLGETCURRENTSURFACEPROC, eglGetCurrentSurface);       \
  EGL_IMPORT_FUNC(PFNEGLGETDISPLAYPROC, eglGetDisplay);                     \
  EGL_IMPORT_FUNC(PFNEGLGETERRORPROC, eglGetError);                         \
  EGL_IMPORT_FUNC(PFNEGLGETPROCADDRESSPROC, eglGetProcAddress);             \
  EGL_IMPORT_FUNC(PFNEGLINITIALIZEPROC, eglInitialize);                     \
  EGL_IMPORT_FUNC(PFNEGLMAKECURRENTPROC, eglMakeCurrent);                   \
  EGL_IMPORT_FUNC(PFNEGLSWAPBUFFERSPROC, eglSwapBuffers);                   \
  EGL_IMPORT_FUNC(PFNEGLSWAPINTERVALPROC, eglSwapInterval);                 \
  EGL_IMPORT_FUNC(PFNEGLTERMINATEPROC, eglTerminate);                       \
  EGL_IMPORT_FUNC(PFNEGLQUERYSTRINGPROC, eglQueryString);

#define EGL_IMPORT_FUNC(_proto, _func) _proto fn_##_func
EGL_IMPORT
#undef EGL_IMPORT_FUNC

void* EGL_LoadGlobal(void* handle) {
#define EGL_IMPORT_FUNC(_proto, _func)                   \
  fn_##_func = (_proto)SDL_LoadFunction(handle, #_func); \
  if (!fn_##_func)                                       \
    LOG(INFO) << "Failed get " #_func ".";
  EGL_IMPORT
#undef EGL_IMPORT_FUNC

  return handle;
}

void* g_egl_library = nullptr;
void* g_glesv2_library = nullptr;

}  // namespace

namespace renderer {

std::unique_ptr<ANGLEDevice> ANGLEDevice::CreateANGLEDevice(
    SDL_Window* host_window,
    ANGLEBackend type) {
  if (!g_egl_library)
    g_egl_library = SDL_LoadObject(
#if defined(OS_WIN)
        "libEGL.dll"
#elif defined(OS_LINUX)
        "./libEGL.so"
#endif
    );

  if (!g_egl_library) {
    LOG(INFO) << "[Renderer] Failed to load EGL dynamic library (libEGL): "
              << SDL_GetError();
    return nullptr;
  }

  if (!g_glesv2_library)
    g_glesv2_library = SDL_LoadObject(
#if defined(OS_WIN)
        "libGLESv2.dll"
#elif defined(OS_LINUX)
        "./libGLESv2.so"
#else
#error unsupport platform
#endif
    );

  if (!g_glesv2_library) {
    LOG(INFO)
        << "[Renderer] Failed to load ANGLE renderer library (libGLESv2): "
        << SDL_GetError();
    return nullptr;
  }

  EGL_LoadGlobal(g_egl_library);

  std::unique_ptr<ANGLEDevice> self(new ANGLEDevice);
  self->host_ = host_window;
  self->gl_lib_ = g_glesv2_library;

  SDL_PropertiesID win_prop = SDL_GetWindowProperties(host_window);
  EGLNativeDisplayType native_display = 0;
  EGLNativeWindowType native_window = 0;

#if defined(OS_WIN)
  native_window = (HWND)SDL_GetPointerProperty(
      win_prop, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
  native_display = (HDC)SDL_GetPointerProperty(
      win_prop, SDL_PROP_WINDOW_WIN32_HDC_POINTER, nullptr);
#elif defined(OS_LINUX)
  native_window =
      SDL_GetNumberProperty(win_prop, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
  native_display = SDL_GetPointerProperty(
      win_prop, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr);
#else
#error unsupport platform
#endif

  self->GetEGLDisplay(native_display, type);
  if (!self->display_) {
    LOG(INFO) << "[EGL] Failed to get ANGLE display, use default EGLDisplay.";

    self->display_ = fn_eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (!self->display_) {
      LOG(INFO) << "[EGL] Failed to get EGLDisplay.";
      return nullptr;
    }
  }

  EGLint majorVersion;
  EGLint minorVersion;
  if (!fn_eglInitialize(self->display_, &majorVersion, &minorVersion)) {
    LOG(INFO) << "[GLContext] Failed to initialize.";
    return nullptr;
  }

  std::vector<EGLint> configAttribs = {
      EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
      EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
      EGL_RED_SIZE,        8,
      EGL_GREEN_SIZE,      8,
      EGL_BLUE_SIZE,       8,
      EGL_ALPHA_SIZE,      8,
  };

  configAttribs.push_back(EGL_NONE);

  EGLint numConfigs;
  EGLConfig surfaceConfig;
  if (!fn_eglChooseConfig(self->display_, configAttribs.data(), &surfaceConfig,
                          1, &numConfigs) ||
      numConfigs < 1) {
    LOG(INFO) << "[GLContext] Could not create choose config.";
    return nullptr;
  }

  self->surface_ = fn_eglCreateWindowSurface(self->display_, surfaceConfig,
                                             native_window, nullptr);
  if (!self->surface_) {
    LOG(INFO) << "[EGL] Failed to create window surface.";
    return nullptr;
  }

  const EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
  self->context_ = fn_eglCreateContext(self->display_, surfaceConfig,
                                       EGL_NO_CONTEXT, contextAttribs);
  if (self->context_ == EGL_NO_CONTEXT) {
    LOG(INFO) << "[GLContext] Could not create context!";
    return nullptr;
  }

  return self;
}

ANGLEDevice::~ANGLEDevice() {
  if (surface_)
    fn_eglDestroySurface(display_, surface_);
  if (context_)
    fn_eglDestroyContext(display_, context_);
  if (display_)
    fn_eglTerminate(display_);
}

bool ANGLEDevice::SwapBuffers() {
  return fn_eglSwapBuffers(display_, surface_);
}

bool ANGLEDevice::SetInterval(int value) {
  return fn_eglSwapInterval(display_, value);
}

bool ANGLEDevice::MakeCurrent(bool null_context) {
  if (null_context)
    return fn_eglMakeCurrent(display_, nullptr, nullptr, nullptr);

  return fn_eglMakeCurrent(display_, surface_, surface_, context_);
}

void* ANGLEDevice::GetGLLibrary() {
  return gl_lib_;
}

void ANGLEDevice::GetEGLDisplay(void* nativeDisplay, ANGLEBackend type) {
  PFNEGLGETPLATFORMDISPLAYPROC fn_GetPlatformDisplay =
      (PFNEGLGETPLATFORMDISPLAYPROC)fn_eglGetProcAddress(
          "eglGetPlatformDisplay");

  LOG(INFO) << "[EGL] Extensions:\n"
            << fn_eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);

  std::vector<EGLAttrib> displayAttributes;
  switch (type) {
    default:
    case renderer::OGLDevice::ANGLEBackend::kOpenGL:
      displayAttributes.push_back(EGL_PLATFORM_ANGLE_TYPE_ANGLE);
      displayAttributes.push_back(EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE);
      break;
    case renderer::OGLDevice::ANGLEBackend::kOpenGLES:
      displayAttributes.push_back(EGL_PLATFORM_ANGLE_TYPE_ANGLE);
      displayAttributes.push_back(EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE);
      break;
    case renderer::OGLDevice::ANGLEBackend::kD3D11on12:
      displayAttributes.push_back(EGL_PLATFORM_ANGLE_D3D11ON12_ANGLE);
      displayAttributes.push_back(EGL_TRUE);
    case renderer::OGLDevice::ANGLEBackend::kD3D11:
      displayAttributes.push_back(EGL_PLATFORM_ANGLE_TYPE_ANGLE);
      displayAttributes.push_back(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE);
      break;
    case renderer::OGLDevice::ANGLEBackend::kSoftware:
      displayAttributes.push_back(EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE);
      displayAttributes.push_back(
          EGL_PLATFORM_ANGLE_DEVICE_TYPE_SWIFTSHADER_ANGLE);
    case renderer::OGLDevice::ANGLEBackend::kVulkan:
      displayAttributes.push_back(EGL_PLATFORM_ANGLE_TYPE_ANGLE);
      displayAttributes.push_back(EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE);
      break;
  }

  displayAttributes.push_back(EGL_NONE);

  display_ = fn_GetPlatformDisplay(EGL_PLATFORM_ANGLE_ANGLE, nativeDisplay,
                                   displayAttributes.data());
}

std::unique_ptr<OGLDevice> OGLDevice::Create(SDL_Window* host_window,
                                             ANGLEBackend type) {
  // Default SDL device context
  if (type == ANGLEBackend::kDisable)
    return SDLDevice::CreateSDLDevice(host_window);

  // ANGLE based device context
  return ANGLEDevice::CreateANGLEDevice(host_window, type);
}

SDLDevice::~SDLDevice() {
  if (context_)
    SDL_GL_DestroyContext(context_);
}

std::unique_ptr<SDLDevice> SDLDevice::CreateSDLDevice(SDL_Window* host_window) {
  std::unique_ptr<SDLDevice> device(new SDLDevice);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, SDL_TRUE);

  device->context_ = SDL_GL_CreateContext(host_window);
  device->host_ = host_window;

  if (!device->context_)
    return nullptr;

  return device;
}

bool SDLDevice::SwapBuffers() {
  return SDL_GL_SwapWindow(host_);
}

bool SDLDevice::SetInterval(int value) {
  return SDL_GL_SetSwapInterval(value);
}

bool SDLDevice::MakeCurrent(bool null_context) {
  return SDL_GL_MakeCurrent(host_, null_context ? nullptr : context_);
}

void* SDLDevice::GetGLLibrary() {
  return nullptr;
}

}  // namespace renderer
