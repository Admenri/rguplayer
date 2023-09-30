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

static inline const char* glGetStringInt(
    scoped_refptr<gpu::GLES2CommandContext> glcontext, GLenum name) {
  return (const char*)glcontext->glGetString(name);
}

static void printGLInfo(base::OnceClosure complete) {
  auto this_context = content::RendererThread::GetCCForRenderer();
  scoped_refptr<gpu::GLES2CommandContext> glcontext =
      this_context->GetContext();
  SDL_Window* win = this_context->GetWindow()->AsSDLWindow();

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

  glcontext->glClearColor(1, 1, 1, 1);
  glcontext->glClear(GL_COLOR_BUFFER_BIT);

  SDL_GL_SwapWindow(win);

  std::move(complete).Run();
}

int main() {
  SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS);
  TTF_Init();
  IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);

  std::unique_ptr<ui::Widget> win(new ui::Widget());
  ui::Widget::InitParams params;
  params.size = base::Vec2i(800, 600);
  win->Init(std::move(params));

  scoped_refptr<content::RendererThread> worker =
      base::MakeRefCounted<content::RendererThread>();
  worker->InitContextAsync(win.get());

  std::unique_ptr<modules::Bitmap> bmp(
      new modules::Bitmap(worker, "example.png"));

  base::RunLoop wait_loop;
  worker->GetRenderThreadRunner()->PostTask(
      base::BindOnce(printGLInfo, wait_loop.QuitClosure()));
  wait_loop.Run();

  worker->GetRenderThreadRunner()->PostTask(base::BindOnce(
      [](modules::Bitmap* bmp) {
        auto this_context = content::RendererThread::GetCCForRenderer();
        SDL_Window* win = this_context->GetWindow()->AsSDLWindow();
        scoped_refptr<gpu::GLES2CommandContext> glcontext =
            this_context->GetContext();

        glcontext->glClearColor(1, 0, 1, 1);
        glcontext->glClear(GL_COLOR_BUFFER_BIT);

        renderer::QuadDrawable quad(this_context->GetQuadIndicesBuffer(),
                                    this_context->GetContext());

        auto& shader = this_context->SimpleShader();

        base::Debug() << "BitmapW: " << bmp->GetSize().x << bmp->GetSize().y;

        shader.Bind();
        shader.SetTexture(bmp->GetTexture());
        shader.SetTextureSize(bmp->GetSize());

        shader.SetTransOffset(base::Vec2i(100, 10));
        shader.SetViewportMatrix(base::Vec2i(800, 600));

        quad.SetPosition(base::RectF(base::Vec2i(), bmp->GetSize()));
        quad.SetTexcoord(base::RectF(base::Vec2i(), bmp->GetSize()));
        quad.Draw();

        SDL_GL_SwapWindow(win);
      },
      bmp.get()));

  base::RunLoop loop;
  base::RunLoop::BindEventDispatcher(
      SDL_QUIT,
      base::BindRepeating(SysEvent, base::Passed(loop.QuitClosure())));

  loop.Run();

  bmp.reset();

  worker.reset();
  win.reset();

  IMG_Quit();
  TTF_Quit();
  SDL_Quit();

  return 0;
}
