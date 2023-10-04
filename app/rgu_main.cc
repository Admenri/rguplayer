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

  std::unique_ptr<modules::Bitmap> bmp(
      new modules::Bitmap(render_thread, "D:\\Desktop\\rgu\\app\\example.png"));

  std::unique_ptr<modules::Bitmap> bmp2(
      new modules::Bitmap(render_thread, "D:\\Desktop\\rgu\\app\\aaa.jpg"));

  auto i = SDL_GetTicks();
  {
    bmp->ClearRect(base::Rect(10, 10, 100, 100));

    bmp->SetPixel(5, 5, base::Vec4i(0, 0, 255, 255));

    auto* surf = bmp->GetSurface();
    IMG_SavePNG(surf, "example.png");

    bmp->StretchBlt(base::Rect(100, 100, 256, 256), bmp2.get(), bmp2->GetRect(),
                    125);
  }
  base::Debug() << "Time Ticks:" << SDL_GetTicks() - i;

  render_thread->GetRenderThreadRunner()->PostTask(base::BindOnce(
      [](modules::Bitmap* bmp) {
        auto* cc = content::RendererThread::GetCCForRenderer();
        auto ctx = cc->GetContext();

        base::Debug() << "[APP] OpenGL Info: Renderer   :"
                      << ctx->glGetString(GL_RENDERER);
        base::Debug() << "                   Version    :"
                      << ctx->glGetString(GL_VERSION);
        base::Debug() << "                   SL Version :"
                      << ctx->glGetString(GL_SHADING_LANGUAGE_VERSION);
        base::Debug() << "                   Max Texture Size:"
                      << cc->GetTextureMaxSize() << "x"
                      << cc->GetTextureMaxSize();
        base::Debug() << "[APP] SDL Info: Main Version :"
                      << SDL_COMPILEDVERSION;
        base::Debug() << "                TTF Version  :"
                      << SDL_TTF_COMPILEDVERSION;
        base::Debug() << "                IMG Version  :"
                      << SDL_IMAGE_COMPILEDVERSION;

        ctx->glClearColor(0, 1, 0, 1);
        ctx->glClear(GL_COLOR_BUFFER_BIT);

        auto* shader = cc->Shaders()->drawable_shader.get();
        renderer::QuadDrawable quad(cc->GetQuadIndicesBuffer(),
                                    cc->GetContext());

        base::TransformMatrix transform;
        /*transform.SetPosition(
            base::Vec2i(cc->States()->viewport->Current().width / 2,
                        cc->States()->viewport->Current().height / 2));
        transform.SetOrigin(
            base::Vec2i(bmp->GetSize().x / 2, bmp->GetSize().y / 2));
        transform.SetRotation(45);*/

        shader->Bind();
        shader->SetViewportMatrix(cc->States()->viewport->Current().Size());
        shader->SetTransformMatrix(transform.GetMatrixDataUnsafe());
        shader->SetTexture(bmp->GetGLTexture()->GetTextureRaw());
        shader->SetTextureSize(bmp->GetSize());

        quad.SetPosition(base::RectF(base::Vec2(), bmp->GetSize()));
        quad.SetTexcoord(base::RectF(base::Vec2(), bmp->GetSize()));
        quad.Draw();

        SDL_GL_SwapWindow(cc->GetWindow()->AsSDLWindow());
      },
      bmp.get()));

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
