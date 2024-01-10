
// clang-format off

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

#include "content/worker/content_compositor.h"
#include "content/public/bitmap.h"
#include "content/public/sprite.h"
#include "content/public/plane.h"
#include "content/public/window.h"

#include "physfs.h"

// clang-format on

SDL_EGLAttrib kAttrib[] = {
    EGL_PLATFORM_ANGLE_TYPE_ANGLE,
    EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
    // EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE,
    // EGL_PLATFORM_ANGLE_DEVICE_TYPE_SWIFTSHADER_ANGLE,
    EGL_NONE,
};

SDL_EGLAttrib* SDLCALL GetAttribArray() {
  return kAttrib;
}

int main(int argc, char* argv[]) {
  PHYSFS_init(argv[0]);

  SDL_Init(SDL_INIT_EVERYTHING);
  IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
  TTF_Init();

  SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_EGL_PLATFORM, EGL_PLATFORM_ANGLE_ANGLE);
  SDL_EGL_SetEGLAttributeCallbacks(GetAttribArray, nullptr, nullptr);

  SDL_Window* win = SDL_CreateWindow("RGU Window", 800, 600,
                                     SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

  content::WorkerTreeCompositor cc;
  content::WorkerTreeCompositor::InitParams params;

  params.binding_params.binding_boot =
      base::BindRepeating([](scoped_refptr<content::BindingRunner> binding) {
        scoped_refptr<content::Graphics> screen = binding->graphics();

        screen->ResizeScreen(base::Vec2i(1024, 768));

        scoped_refptr<content::Bitmap> bmp = new content::Bitmap(
            screen, "D:\\Desktop\\rgu\\app\\test\\example.png");

        scoped_refptr<content::Bitmap> sampler = new content::Bitmap(
            screen, "D:\\Desktop\\rgu\\app\\test\\test.png");

        bmp->ClearRect(base::Rect(20, 20, 50, 50));
        bmp->Blt(100, 100, sampler, sampler->GetRect()->AsBase(), 255);

        bmp->GradientFillRect(base::Rect(50, 100, 100, 50),
                              new content::Color(0, 255, 0, 125),
                              new content::Color(0, 0, 255, 125));

        /* Sync method test */
        // SDL_Surface* surf = bmp->SurfaceRequired();
        // IMG_SavePNG(surf, "D:\\Desktop\\snap.png");

        scoped_refptr<content::Sprite> sp = new content::Sprite(screen);
        sp->SetBitmap(bmp);
        sp->GetTransform().SetOrigin(
            base::Vec2i(sp->GetWidth() / 2, sp->GetHeight() / 2));
        sp->GetTransform().SetPosition(
            base::Vec2i(screen->GetWidth() / 2, screen->GetHeight() / 2));

        sp->SetBushDepth(100);
        sp->SetBushOpacity(128);

        sp->SetWaveAmp(20);
        sp->GetSrcRect()->Set(base::Rect(100, 100, 100, 100));

        scoped_refptr<content::Plane> pl = new content::Plane(screen);
        pl->SetBitmap(
            new content::Bitmap(screen, "D:\\Desktop\\rgu\\app\\test\\bg.png"));

        for (int i = 0; i < 120; ++i) {
          screen->Update();
        }

        screen->Freeze();

        scoped_refptr<content::Viewport> viewp =
            new content::Viewport(screen, base::Rect(0, 0, 300, 300));
        viewp->SetZ(100);

        scoped_refptr<content::Window2> vx_win =
            new content::Window2(screen, 100, 100, 300, 300);

        vx_win->SetViewport(viewp);

        vx_win->SetWindowskin(new content::Bitmap(
            screen, "D:\\Desktop\\rgu\\app\\test\\Window.png"));
        vx_win->SetZ(100);
        vx_win->GetTone()->Set(-68, -68, 68, 0);
        vx_win->SetPause(true);

        vx_win->SetContents(bmp);

        vx_win->SetActive(true);
        vx_win->SetCursorRect(
            new content::Rect(base::Rect(110, 110, 100, 100)));

        vx_win->SetArrowsVisible(true);

        viewp->SetTone(new content::Tone(68, 68, 0, 0));

        screen->SetBrightness(125);

        scoped_refptr<content::Bitmap> snapshot = screen->SnapToBitmap();
        auto* surf = snapshot->SurfaceRequired();
        IMG_SavePNG(surf, "D:\\Desktop\\snap.png");

        screen->Transition(
            120, new content::Bitmap(
                     screen, "D:\\Desktop\\rgu\\app\\test\\BattleStart.png"));

        scoped_refptr<content::Bitmap> snapshot2 =
            new content::Bitmap(screen, 800, 600);
        viewp->SnapToBitmap(snapshot2);
        auto* surf2 = snapshot2->SurfaceRequired();
        IMG_SavePNG(surf2, "D:\\Desktop\\snap2.png");

        float xxx = 0;
        while (!binding->quit_required()) {
          sp->Update();
          sp->GetTransform().SetRotation(++xxx);

          screen->Update();

          if (!vx_win->IsDisposed())
            vx_win->Update();
        }
      });

  params.binding_params.initial_resolution = base::Vec2i(800, 600);
  params.renderer_params.target_window = win;

  cc.InitCC(params);
  cc.ContentMain();

  SDL_DestroyWindow(win);

  TTF_Quit();
  IMG_Quit();
  SDL_Quit();

  return 0;
}
