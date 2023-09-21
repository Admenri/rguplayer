#include <iostream>

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "base/debug/debugwriter.h"
#include "base/exceptions/exception.h"
#include "gpu/gles2/gles_context.h"
#include "renderer/quad_draw.h"

#undef main

static inline const char* glGetStringInt(
    std::shared_ptr<gpu::GLES2CommandContext> glcontext, GLenum name) {
  return (const char*)glcontext->glGetString(name);
}

static void printGLInfo(std::shared_ptr<gpu::GLES2CommandContext> glcontext) {
  base::Debug() << "* GLES:" << std::boolalpha << glcontext->IsGLES();
  base::Debug() << "* OpenGL Info: Renderer   :"
                << glGetStringInt(glcontext, GL_RENDERER);
  base::Debug() << "               Version    :"
                << glGetStringInt(glcontext, GL_VERSION);
  base::Debug() << "               SL Version :"
                << glGetStringInt(glcontext, GL_SHADING_LANGUAGE_VERSION);
  base::Debug() << "* SDL Info: Main Version :" << SDL_COMPILEDVERSION;
  base::Debug() << "            TTF Version  :" << SDL_TTF_COMPILEDVERSION;
  base::Debug() << "            IMG Version  :" << SDL_IMAGE_COMPILEDVERSION;
}

int main() {
  SDL_Init(SDL_INIT_EVERYTHING);
  TTF_Init();
  IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);

  SDL_Window* win = SDL_CreateWindow(
      "Pic display", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 544, 416,
      SDL_WINDOW_OPENGL | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_RESIZABLE);
  SDL_GLContext glCtx;

  /* Setup GL context */
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  glCtx = SDL_GL_CreateContext(win);

  std::shared_ptr<gpu::GLES2CommandContext> glcontext =
      std::make_shared<gpu::GLES2CommandContext>();
  try {
    glcontext->InitContext();
  } catch (const base::Exception& e) {
    base::Debug() << e.GetErrorMessage();
  }

  printGLInfo(glcontext);

  renderer::QuadDrawable quad(glcontext);
  quad.Draw();

  SDL_Event e;
  while (true) {
    SDL_PollEvent(&e);
    if (e.type == SDL_QUIT) break;

    SDL_Delay(10);
  }

  SDL_GL_DeleteContext(glCtx);
  SDL_DestroyWindow(win);

  IMG_Quit();
  TTF_Quit();
  SDL_Quit();
  return 0;
}
