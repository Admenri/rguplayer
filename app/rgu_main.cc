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

const char* vertexShaderSource =
    "attribute vec3 aPos;\n"
    "attribute vec2 aTex;\n"
    "varying vec2 texcoord;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "   texcoord = aTex;"
    "}\0";
const char* fragmentShaderSource =
    "uniform sampler2D texture;\n"
    "varying vec2 texcoord;\n"
    "void main()\n"
    "{\n"
    "   gl_FragColor = texture2D(texture, texcoord);\n"
    "}\n\0";

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

  render_thread->GetRenderThreadRunner()->PostTask(base::BindOnce([]() {
    auto* cc = content::RendererThread::GetCCForRenderer();
    auto ctx = cc->GetContext();

    ctx->glClearColor(0, 1, 0, 1);
    ctx->glClear(GL_COLOR_BUFFER_BIT);

    auto& shader = cc->DrawableShader();
    renderer::QuadDrawable quad(cc->GetQuadIndicesBuffer(), cc->GetContext());

    scoped_refptr<renderer::GLTexture> tex = new renderer::GLTexture(ctx);

    auto img = IMG_Load("example.png");
    auto size = base::Vec2i(img->w, img->h);

    base::TransformMatrix transform;
    transform.SetPosition(base::Vec2i(800 / 2, 600 / 2));
    transform.SetOrigin(base::Vec2i(img->w / 2, img->h / 2));
    transform.SetRotation(45);

    tex->Bind();
    tex->SetTextureFilter(GL_NEAREST);
    tex->SetSize(size);
    tex->BufferData(img->pixels, GL_RGBA);

    shader.Bind();
    shader.SetViewportMatrix(cc->Viewport().Current().Size());
    shader.SetTransformMatrix(transform.GetMatrixDataUnsafe());
    shader.SetTexture(tex->GetTextureRaw());
    shader.SetTextureSize(size);

    quad.SetPosition(base::RectF(base::Vec2(), size));
    quad.SetTexcoord(base::RectF(base::Vec2(), size));
    quad.Draw();

    SDL_GL_SwapWindow(cc->GetWindow()->AsSDLWindow());
  }));

  base::RunLoop loop;
  base::RunLoop::BindEventDispatcher(
      SDL_QUIT,
      base::BindRepeating(SysEvent, base::Passed(loop.QuitClosure())));

  loop.Run();

  render_thread.reset();
  win.reset();

  IMG_Quit();
  TTF_Quit();
  SDL_Quit();

  return 0;
}
