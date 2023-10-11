#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#undef main

#include <iostream>

#include "base/bind/callback.h"
#include "base/buildflags/compiler_specific.h"
#include "base/debug/logging.h"
#include "base/exceptions/exception.h"
#include "base/math/transform.h"
#include "base/memory/weak_ptr.h"
#include "base/worker/run_loop.h"
#include "base/worker/thread_worker.h"
#include "gpu/gles2/gsm/gles_gsm.h"
#include "gpu/gles2/vertex/vertex_array.h"
#include "ui/widget/widget.h"

void SysEvent(base::OnceClosure quit_closure, const SDL_Event& sdl_event) {
  if (sdl_event.type == SDL_QUIT) {
    std::move(quit_closure).Run();
  }
}

int main() {
  SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS);
  TTF_Init();
  IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);

  std::unique_ptr<ui::Widget> win(new ui::Widget());
  ui::Widget::InitParams params;
  params.size = base::Vec2i(800, 600);
  params.title = "RGU Widget";
  win->Init(std::move(params));

  SDL_GL_CreateContext(win->AsSDLWindow());

  gpu::GL.InitContext();
  gpu::GSM.InitStates();

  gpu::VertexArray<gpu::CommonVertex> vertex;

  gpu::VertexArray<gpu::CommonVertex>::Init(vertex);

  base::RunLoop loop(base::RunLoop::MessagePumpType::UI);
  base::RunLoop::BindEventDispatcher(
      SDL_QUIT,
      base::BindRepeating(SysEvent, base::Passed(loop.QuitClosure())));

  loop.Run();

  win.reset();

  IMG_Quit();
  TTF_Quit();
  SDL_Quit();

  return 0;
}
