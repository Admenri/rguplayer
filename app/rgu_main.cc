
// clang-format off

#include "ruby.h"
#include "ruby/intern.h"
#include "ruby/encoding.h"

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

  ruby_sysinit(&argc, &argv);

  RUBY_INIT_STACK;
  ruby_init();

  std::vector<const char*> rubyArgsC{argv[0], "-e "};
  void* node =
      ruby_options(rubyArgsC.size(), const_cast<char**>(rubyArgsC.data()));

  int state = 0;
  bool valid = ruby_executable_node(node, &state);

  if (valid)
    state = ruby_exec_node(node);

  if (state || !valid) {
    return -1;
  }

  rb_enc_set_default_internal(rb_enc_from_encoding(rb_utf8_encoding()));
  rb_enc_set_default_external(rb_enc_from_encoding(rb_utf8_encoding()));

  SDL_Init(SDL_INIT_EVERYTHING);
  IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
  TTF_Init();

  SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_EGL_PLATFORM, EGL_PLATFORM_ANGLE_ANGLE);

  SDL_EGL_SetEGLAttributeCallbacks(GetAttribArray, nullptr, nullptr);

  SDL_Window* win =
      SDL_CreateWindow("ANGLE Window", 800, 600, SDL_WINDOW_OPENGL);
  SDL_GLContext ctx = SDL_GL_CreateContext(win);

  SDL_GL_MakeCurrent(win, ctx);
  SDL_GL_SetSwapInterval(0);

  renderer::GLES2Context::CreateForCurrentThread();

  printf("GL_VERSION: %s\n", renderer::GL.GetString(GL_VERSION));
  printf("GL_VENDOR: %s\n", renderer::GL.GetString(GL_VENDOR));
  printf("GL_RENDERER: %s\n", renderer::GL.GetString(GL_RENDERER));

  SDL_Event e;
  int t = 0;
  while (true) {
    SDL_PollEvent(&e);
    if (e.type == SDL_EVENT_QUIT)
      break;

    renderer::GL.ClearColor((t < 255) ? t / 255.0f : 0,
                            (t > 255 && t < 255 * 2) ? ((t - 255) / 255.0f) : 0,
                            (t > 255 * 2) ? ((t - 255 * 2) / 255.0f) : 0, 1.0f);
    renderer::GL.Clear(GL_COLOR_BUFFER_BIT);
    SDL_GL_SwapWindow(win);

    t += 5;
    if (t >= 255 * 3)
      t = 0;

    SDL_Delay(10);
  }

  SDL_DestroyWindow(win);

  TTF_Quit();
  IMG_Quit();
  SDL_Quit();

  ruby_cleanup(0);

  return 0;
}
