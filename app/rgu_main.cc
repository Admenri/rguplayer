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
#include "content/scheduler/worker_cc.h"
#include "content/script/drawable.h"
#include "content/script/plane.h"
#include "gpu/gles2/draw/quad_drawable.h"
#include "gpu/gles2/gsm/gles_gsm.h"
#include "gpu/gles2/vertex/vertex_array.h"
#include "ui/widget/widget.h"

int main() {
  SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS);
  TTF_Init();
  IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);

  std::unique_ptr<ui::Widget> win(new ui::Widget());
  ui::Widget::InitParams params;
  params.size = base::Vec2i(800, 600);
  params.title = "Graph Widget";
  win->Init(std::move(params));

  std::unique_ptr<content::WorkerTreeHost> engine =
      std::make_unique<content::WorkerTreeHost>(false /* Render Sync Worker */);

  content::RenderRunner::InitParams render_params;
  render_params.ogl_window = win->AsWeakPtr();
  render_params.initial_resolution = win->GetSize();

  content::BindingRunner::InitParams binding_params;
  binding_params.window = win->AsWeakPtr();
  binding_params.resolution = win->GetSize();
  engine->Run(std::move(render_params), std::move(binding_params));

  engine.reset();

  win.reset();

  IMG_Quit();
  TTF_Quit();
  SDL_Quit();

  return 0;
}
