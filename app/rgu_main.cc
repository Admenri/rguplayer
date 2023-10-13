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
#include "gpu/gles2/draw/quad_drawable.h"
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
  params.title = "Graph Widget";
  win->Init(std::move(params));

  SDL_GL_CreateContext(win->AsSDLWindow());

  gpu::GL.InitContext();
  gpu::GSM.InitStates();

  gpu::QuadDrawable quad;

  gpu::GLID<gpu::Texture> tex = gpu::Texture::Gen();

  gpu::Texture::Bind(tex);
  gpu::Texture::SetFilter();
  gpu::Texture::SetWrap();

  SDL_Surface* img =
      IMG_Load("D:\\Desktop\\rgu\\app\\resources\\rgu_favicon_64.png");
  auto size = base::Vec2i(img->w, img->h);
  gpu::Texture::TexImage2D(img->w, img->h, GL_RGBA, img->pixels);

  base::TransformMatrix* trans = new base::TransformMatrix[50000];

  for (int i = 0; i < 50000; i++) {
    trans[i].SetPosition(base::Vec2i(rand() % 800, rand() % 600));
  }

  quad.SetPositionRect(base::Rect(size));
  quad.SetTexCoordRect(base::Rect(size));

  int xxx = 0;

  gpu::GSM.states.viewport.Set(base::Rect(win->GetSize()));

  SDL_GL_SetSwapInterval(0);

  SDL_Event e;
  for (;;) {
    xxx += 1;

    gpu::GL.ClearColor(0, 1, 0, 1);
    gpu::GL.Clear(GL_COLOR_BUFFER_BIT);

    for (int i = 0; i < 10000; i++) {
      auto& shader = gpu::GSM.shaders->transform;

      shader.Bind();
      shader.SetProjectionMatrix(base::Vec2i(800, 600));
      shader.SetTexture(tex);
      shader.SetTextureSize(size);

      trans[i].SetRotation(xxx);

      shader.SetTransformMatrix(trans[i].GetMatrixDataUnsafe());

      gpu::GSM.states.blend.Push(true);
      gpu::GSM.states.blend_func.Push(gpu::GLBlendType::Normal);

      quad.Draw();

      gpu::GSM.states.blend.Pop();
      gpu::GSM.states.blend_func.Pop();
    }

    SDL_GL_SwapWindow(win->AsSDLWindow());
    SDL_PollEvent(&e);
  }

  base::RunLoop loop(base::RunLoop::MessagePumpType::UI);
  base::RunLoop::BindEventDispatcher(
      SDL_QUIT,
      base::BindRepeating(SysEvent, base::Passed(loop.QuitClosure())));

  while (auto e = gpu::GL.GetError()) {
    LOG(INFO) << "GLError:" << e;
  }

  loop.Run();

  win.reset();

  IMG_Quit();
  TTF_Quit();
  SDL_Quit();

  return 0;
}
