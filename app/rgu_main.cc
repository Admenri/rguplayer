
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

#include "binding/mri/mri_main.h"
#include "content/worker/content_compositor.h"

int main(int argc, char* argv[]) {
  SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS);
  IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
  TTF_Init();

  SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");

  scoped_refptr<content::CoreConfigure> config = new content::CoreConfigure();

  std::unique_ptr<ui::Widget> win = std::make_unique<ui::Widget>();
  ui::Widget::InitParams win_params;

  win_params.size = config->initial_resolution();
  win_params.title = config->game_title();
  win_params.resizable = true;
  win_params.hpixeldensity = false;

  win->Init(std::move(win_params));

  std::unique_ptr<content::WorkerTreeCompositor> cc(
      new content::WorkerTreeCompositor);
  content::ContentInitParams params;

  config->renderer_debug_output() = false;
  config->version() = content::CoreConfigure::RGSS3;
  config->base_path() = "D:/Desktop/Project1/";

  params.config = config;
  params.binding_engine = std::make_unique<binding::BindingEngineMri>();
  params.initial_resolution = config->initial_resolution();
  params.host_window = win->AsWeakPtr();

  cc->InitCC(std::move(params));
  cc->ContentMain();

  cc.reset();

  win.reset();

  TTF_Quit();
  IMG_Quit();
  SDL_Quit();

  return 0;
}
