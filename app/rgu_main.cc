
// clang-format off

#include "base/debug/logging.h"
#include "base/bind/callback.h"
#include "base/memory/ref_counted.h"

#include "renderer/context/gles2_context.h"

#include "SDL.h"
#include "SDL_opengles2.h"
#include "SDL_video.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

#include "EGL/egl.h"
#include "EGL/eglext.h"

#include "content/worker/content_compositor.h"

#include "physfs.h"

// clang-format on

SDL_EGLAttrib kAttrib[] = {
    EGL_PLATFORM_ANGLE_TYPE_ANGLE,
    EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE,
    // EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE,
    // EGL_PLATFORM_ANGLE_DEVICE_TYPE_SWIFTSHADER_ANGLE,
    EGL_NONE,
};

SDL_EGLAttrib* SDLCALL GetAttribArray() {
  return kAttrib;
}

int main(int argc, char* argv[]) {
  PHYSFS_init(argv[0]);

  SDL_Init(SDL_INIT_EVERYTHING);
  IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
  TTF_Init();

  SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_EGL_PLATFORM, EGL_PLATFORM_ANGLE_ANGLE);
  SDL_EGL_SetEGLAttributeCallbacks(GetAttribArray, nullptr, nullptr);

  SDL_Window* win = SDL_CreateWindow("RGU Window", 800, 600, SDL_WINDOW_OPENGL);

  content::WorkerTreeCompositor cc;
  content::WorkerTreeCompositor::InitParams params;

  params.binding_params.initial_resolution = base::Vec2i(800, 600);
  params.renderer_params.target_window = win;

  cc.InitCC(params);
  cc.ContentMain();

  SDL_DestroyWindow(win);

  TTF_Quit();
  IMG_Quit();
  SDL_Quit();

  return 0;
}
