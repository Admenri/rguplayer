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
#include "content/render_thread.h"
#include "modules/bitmap.h"
#include "modules/graphics.h"
#include "modules/sprite.h"
#include "renderer/compositor/renderer_cc.h"
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

  scoped_refptr<content::RendererThread> render_thread =
      new content::RendererThread();
  render_thread->InitContextAsync(win.get());

  std::unique_ptr<modules::Graphics> graphics(new modules::Graphics(
      render_thread, base::Rect(base::Vec2i(), win->GetSize())));

  scoped_refptr<modules::Bitmap> bmp(
      new modules::Bitmap(render_thread, "D:\\Desktop\\rgu\\app\\example.png"));

  scoped_refptr<modules::Bitmap> bmp2(
      new modules::Bitmap(render_thread, "D:\\Desktop\\rgu\\app\\aaa.jpg"));

  auto i = SDL_GetTicks();
  {
    bmp->ClearRect(base::Rect(10, 10, 100, 100));

    bmp->SetPixel(5, 5, new modules::Color(0, 0, 255, 255));

    auto* surf = bmp->GetSurface();
    IMG_SavePNG(surf, "example.png");

    bmp->StretchBlt(base::Rect(100, 100, 256, 256), bmp2.get(), bmp2->GetRect(),
                    125);

    bmp->GradientFillRect(base::Rect(200, 200, 200, 50),
                          new modules::Color(0, 0, 255, 0),
                          new modules::Color(0, 0, 255, 255));
  }
  base::Debug() << "Time Ticks:" << SDL_GetTicks() - i;

  scoped_refptr<modules::Viewport> viewport(
      new modules::Viewport(graphics.get()));

  scoped_refptr<modules::Sprite> sprite(
      new modules::Sprite(graphics.get(), viewport));
  sprite->InitRefCountedAttributes();
  sprite->SetBitmap(bmp);

  viewport->SetOX(100);

  graphics->Update();

  base::RunLoop loop;
  base::RunLoop::BindEventDispatcher(
      SDL_QUIT,
      base::BindRepeating(SysEvent, base::Passed(loop.QuitClosure())));

  loop.Run();

  bmp.reset();
  render_thread.reset();
  win.reset();

  IMG_Quit();
  TTF_Quit();
  SDL_Quit();

  return 0;
}
