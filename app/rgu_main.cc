
#include "base/buildflags/compiler_specific.h"
#include "binding/mri/mri_main.h"
#include "content/config/core_config.h"
#include "content/worker/content_compositor.h"

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_main.h"
#include "SDL_system.h"
#include "SDL_ttf.h"

#if defined(OS_LINUX)
#include "app/icon.xxd"
#endif  //! defined(OS_LINUX)

#if defined(OS_WIN)
#include <windows.h>
extern "C" {
__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

#if defined(OS_ANDROID)
#include <jni.h>
#include <sys/system_properties.h>
#include <unistd.h>
#include <filesystem>
#endif

namespace {

void ReplaceStringWidth(std::string& str, char before, char after) {
  for (size_t i = 0; i < str.size(); ++i)
    if (str[i] == before)
      str[i] = after;
}

}  // namespace

int main(int argc, char* argv[]) {
#if defined(OS_WIN)
  ::SetConsoleOutputCP(CP_UTF8);
#endif  //! defined(OS_WIN)

#if defined(OS_ANDROID)
  // Get GAME_PATH string field from JNI (MainActivity.java)
  JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
  jobject activity = (jobject)SDL_AndroidGetActivity();
  jclass cls = env->GetObjectClass(activity);
  jfieldID fIDGamePath =
      env->GetStaticFieldID(cls, "GAME_PATH", "Ljava/lang/String;");
  jstring strJGamePath = (jstring)env->GetStaticObjectField(cls, fIDGamePath);
  const char* dataDir = env->GetStringUTFChars(strJGamePath, 0);

  // Set and ensure current directory
  std::filesystem::path stdPath(dataDir);
  if (!std::filesystem::exists(stdPath) ||
      !std::filesystem::is_directory(stdPath)) {
    std::filesystem::create_directories(stdPath);
  }

  std::filesystem::current_path(stdPath);
  if (std::filesystem::equivalent(std::filesystem::current_path(), stdPath))
    LOG(INFO) << "[Android] Base directory: " << dataDir;

  env->ReleaseStringUTFChars(strJGamePath, dataDir);
  env->DeleteLocalRef(strJGamePath);
  env->DeleteLocalRef(cls);

  // Fixed configure file
  std::string app = "Game";
  std::string ini = app + ".ini";
#else
  std::string app(argv[0]);
  ReplaceStringWidth(app, '\\', '/');
  auto last_sep = app.find_last_of('/');
  if (last_sep != std::string::npos)
    app = app.substr(last_sep + 1);

  LOG(INFO) << "[App] Path: " << app;

  last_sep = app.find_last_of('.');
  if (last_sep != std::string::npos)
    app = app.substr(0, last_sep);
  std::string ini = app + ".ini";
#endif  //! defined(OS_ANDROID)

  LOG(INFO) << "[App] Configure: " << ini;

  scoped_refptr<content::CoreConfigure> config = new content::CoreConfigure();
  config->LoadCommandLine(argc, argv);

  std::unique_ptr<filesystem::Filesystem> iosystem =
      std::make_unique<filesystem::Filesystem>(argv[0]);
  iosystem->AddLoadPath(".");

  SDL_IOStream* inifile = nullptr;
  try {
    inifile = iosystem->OpenReadRaw(ini);
  } catch (const base::Exception& e) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "RGU Kernel",
                             e.GetErrorMessage().c_str(), nullptr);
    return 1;
  }

  if (!config->LoadConfigure(inifile, app))
    return 1;

  config->executable_file() = app;
  for (auto& it : config->load_paths())
    iosystem->AddLoadPath(it);

  if (config->content_version() == content::RGSSVersion::Null) {
    LOG(INFO) << "[Entry] Failed to load RGSS version.";
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "RGU Kernel",
                             "Failed to identify RGSS version from configure.",
                             nullptr);
    return 1;
  }

  // Load i18n translation
  config->Loadi18nXML(iosystem.get());

  SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
  SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

  SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "1");
  SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "1");

#if defined(OS_ANDROID)
  SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
#endif

  SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO);
  IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
  TTF_Init();

  // Init ANGLE vendor settings
  content::RenderRunner::InitANGLERenderer(config->angle_renderer());

  std::unique_ptr<ui::Widget> win = std::make_unique<ui::Widget>();
  ui::Widget::InitParams win_params;

  win_params.fullscreen = config->fullscreen();
  win_params.size = config->window_size();
  win_params.title = config->game_title();
  win_params.resizable = true;
  win_params.hpixeldensity = true;
  win->Init(std::move(win_params));

#if defined(OS_LINUX)
  auto* icon_ops =
      SDL_IOFromConstMem(rgu_favicon_64_png, rgu_favicon_64_png_len);

  if (icon_ops) {
    auto* icon = IMG_Load_IO(icon_ops, SDL_TRUE);

    if (icon) {
      SDL_SetWindowIcon(win->AsSDLWindow(), icon);
      SDL_DestroySurface(icon);
    }
  }
#endif  //! defined(OS_LINUX)

  std::unique_ptr<content::WorkerTreeCompositor> cc(
      new content::WorkerTreeCompositor);
  content::ContentInitParams params;

  params.filesystem = std::move(iosystem);
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
