
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

#include "binding/mri/mri_main.h"
#include "content/worker/content_compositor.h"

#include "EGL/egl.h"
#include "EGL/eglext.h"

SDL_EGLAttrib kAttrib[] = {
    EGL_PLATFORM_ANGLE_TYPE_ANGLE,
    EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
    // EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE,
    // EGL_PLATFORM_ANGLE_DEVICE_TYPE_SWIFTSHADER_ANGLE,
    EGL_NONE,
};

SDL_EGLAttrib* SDLCALL GetAttribArray() {
  return kAttrib;
}

int main(int argc, char* argv[]) {
  SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS);
  IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
  TTF_Init();

  SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_EGL_PLATFORM, EGL_PLATFORM_ANGLE_ANGLE);
  SDL_EGL_SetEGLAttributeCallbacks(GetAttribArray, nullptr, nullptr);

  scoped_refptr<content::CoreConfigure> config = new content::CoreConfigure();

  std::unique_ptr<ui::Widget> win = std::make_unique<ui::Widget>();
  ui::Widget::InitParams win_params;

  win_params.size = config->initial_resolution();
  win_params.title = config->game_title();
  win_params.resizable = true;

  win->Init(std::move(win_params));

  std::unique_ptr<content::WorkerTreeCompositor> cc(
      new content::WorkerTreeCompositor);
  content::ContentInitParams params;

  config->version() = content::CoreConfigure::RGSS3;
  config->game_scripts() = "D:/Desktop/Project1/Data/Scripts.rvdata2";

  params.config = config;
  params.binding_engine = std::make_unique<binding::BindingEngineMri>();
  params.initial_resolution = config->initial_resolution();
  params.host_window = win->AsWeakPtr();
  params.sync_renderer = true;

  cc->InitCC(std::move(params));
  cc->ContentMain();

  cc.reset();

  win.reset();

  TTF_Quit();
  IMG_Quit();
  SDL_Quit();

  return 0;
}
