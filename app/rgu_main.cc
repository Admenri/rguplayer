
#include "binding/mri/mri_main.h"
#include "content/config/core_config.h"
#include "content/worker/content_compositor.h"

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

int main(int argc, char* argv[]) {
#if defined(OS_WIN)
  ::SetConsoleOutputCP(CP_UTF8);
#endif  //! defined(OS_WIN)

  std::string app(argv[0]);
  auto last_sep = app.find_last_of('\\');
  if (last_sep != std::string::npos)
    app = app.substr(last_sep + 1);

  LOG(INFO) << "[App] Path: " << app;

  scoped_refptr<content::CoreConfigure> config = new content::CoreConfigure();
  config->LoadCommandLine(argc, argv);

  last_sep = app.find_last_of('.');
  if (last_sep != std::string::npos)
    app = app.substr(0, last_sep);
  std::string ini = app + ".ini";

  LOG(INFO) << "[App] Configure: " << ini;

  if (!config->LoadConfigure(ini))
    return 1;

  if (config->content_version() == content::RGSSVersion::Null) {
    LOG(INFO) << "[Entry] Failed to load RGSS version.";
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "RGU Core",
                             "Failed to identify RGSS version from configure.",
                             nullptr);
    return 1;
  }

  SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
  SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0");
  SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

  SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO);
  IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
  TTF_Init();

  content::RenderRunner::InitANGLERenderer(config->angle_renderer());

  std::unique_ptr<ui::Widget> win = std::make_unique<ui::Widget>();
  ui::Widget::InitParams win_params;

  win_params.size = config->window_size();
  win_params.title = config->game_title();
  win_params.resizable = true;
  win_params.hpixeldensity = false;

  win->Init(std::move(win_params));

  std::unique_ptr<content::WorkerTreeCompositor> cc(
      new content::WorkerTreeCompositor);
  content::ContentInitParams params;

  params.argv0 = *argv;
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
