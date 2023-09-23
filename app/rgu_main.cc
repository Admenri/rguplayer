#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#undef main

#include <iostream>

#include "base/bind/callback.h"
#include "base/buildflags/compiler_specific.h"
#include "base/debug/logging.h"
#include "base/exceptions/exception.h"
#include "base/memory/weak_ptr.h"
#include "renderer/compositor/renderer_cc.h"
#include "ui/widget/widget.h"

static inline const char* glGetStringInt(
    scoped_refptr<gpu::GLES2CommandContext> glcontext, GLenum name) {
  return (const char*)glcontext->glGetString(name);
}

static void printGLInfo(scoped_refptr<gpu::GLES2CommandContext> glcontext) {
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

  std::unique_ptr<ui::Widget> win = std::make_unique<ui::Widget>();
  ui::Widget::InitParams params;

  params.size = base::Vec2i(800, 600);

  win->Init(std::move(params));

  SDL_GLContext glCtx;

  /* Setup GL context */
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  glCtx = SDL_GL_CreateContext(win->AsSDLWindow());

  std::unique_ptr<renderer::CCLayer> glcontext;

  try {
    glcontext = std::make_unique<renderer::CCLayer>(glCtx);
  } catch (const base::Exception& e) {
    base::Debug() << e.GetErrorMessage();
  }

  auto cb = base::BindOnce(printGLInfo);
  std::move(cb).Run(glcontext->GetContext());

  SDL_Event e;
  while (true) {
    SDL_PollEvent(&e);
    if (e.type == SDL_QUIT) break;

    SDL_Delay(10);
  }

  SDL_GL_DeleteContext(glCtx);
  win.reset();

  IMG_Quit();
  TTF_Quit();
  SDL_Quit();
  return 0;
}
