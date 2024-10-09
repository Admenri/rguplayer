
#include <iostream>

#include "content/public/bitmap.h"
#include "content/public/geometry.h"
#include "content/public/graphics.h"
#include "content/public/plane.h"
#include "content/public/sprite.h"
#include "content/public/window2.h"

#include "components/filesystem/filesystem.h"

#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"

#include "SDL3_image/SDL_image.h"
#include "SDL3_ttf/SDL_ttf.h"

void MainLoop(fiber_t* fiber) {
  content::CoroutineContext* cc =
      static_cast<content::CoroutineContext*>(fiber->userdata);

  std::unique_ptr<filesystem::Filesystem> fps(new filesystem::Filesystem(""));
  fps->AddLoadPath(".");

  try {
    std::unique_ptr<ui::Widget> win(new ui::Widget());
    ui::Widget::InitParams win_params;
    win_params.size = base::Vec2i(800, 600);
    win_params.resizable = true;
    win->Init(std::move(win_params));

    scoped_refptr<content::Graphics> host =
        new content::Graphics(cc, win->AsWeakPtr(),
                              std::make_unique<content::ScopedFontData>(
                                  fps.get(), "Fonts/Default.ttf"),
                              win_params.size, content::APIVersion::RGSS3);

    fiber->userdata = host.get();

    {
      scoped_refptr<content::Bitmap> b =
          new content::Bitmap(host, fps.get(), "test.png");
      scoped_refptr<content::Bitmap> b2 =
          new content::Bitmap(host, fps.get(), "bg.png");

      b2->Blt({50, 50}, b, {50, 50, 300, 300}, 125);
      b2->SetPixel({10, 10}, new content::Color(0, 0, 0, 255));
      b2->GradientFillRect({0, 0, 100, 100}, new content::Color(0, 0, 0, 255),
                           new content::Color(255, 255, 255, 255), true);
      // b2->DrawText({0, 0, 200, 200}, "test text draw");

      b2->HueChange(125);

      auto* surf = b2->SurfaceRequired();
      IMG_SavePNG(surf, "out.png");
    }

    host->Freeze();

    {
      scoped_refptr<content::Viewport> vp0 =
          new content::Viewport(host, base::Rect(50, 50, 600, 600));

      scoped_refptr<content::Sprite> bg_spr = new content::Sprite(host, vp0);
      bg_spr->SetBitmap(new content::Bitmap(host, fps.get(), "bg.png"));
      bg_spr->SetWaveAmp(10);

      scoped_refptr<content::Plane> bg_ple = new content::Plane(host, vp0);
      bg_ple->SetBitmap(new content::Bitmap(host, fps.get(), "tile.png"));

      scoped_refptr<content::Sprite> bg_spr1 = new content::Sprite(host, vp0);
      bg_spr1->SetBitmap(new content::Bitmap(host, fps.get(), "bg1.png"));
      bg_spr1->SetX(100);
      bg_spr1->SetY(100);

      scoped_refptr<content::Viewport> vp = new content::Viewport(host, vp0);
      vp->SetRect(new content::Rect(base::Rect(50, 50, 200, 200)));
      vp->SetTone(new content::Tone(-68, -68, 0, 68));

      std::vector<scoped_refptr<content::Sprite>> sprs;
      scoped_refptr<content::Bitmap> item =
          new content::Bitmap(host, fps.get(), "item.png");

      // host->SetBrightness(155);

      scoped_refptr<content::Window2> win =
          new content::Window2(host, base::Rect(200, 200, 300, 300));
      win->SetWindowskin(new content::Bitmap(host, fps.get(), "Window.png"));
      win->SetTone(new content::Tone(-34, 0, 68, 0));
      win->SetContents(bg_spr->GetBitmap());
      win->SetOX(200);
      win->SetOY(200);

      srand(time(nullptr));
      for (int i = 0; i < 1000; ++i) {
        scoped_refptr<content::Sprite> spr = new content::Sprite(host, vp);
        spr->SetBitmap(item);

        spr->SetX(rand() % 800);
        spr->SetY(rand() % 600);

        sprs.push_back(spr);
      }

      scoped_refptr<content::Bitmap> snap =
          new content::Bitmap(host, base::Vec2i(800, 600));
      vp0->SnapToBitmap(snap);

      auto* surf = snap->SurfaceRequired();
      IMG_SavePNG(surf, "out111.png");

      host->Transition(120);

      scoped_refptr<content::Geometry> geo = new content::Geometry(host);
      geo->SetBitmap(new content::Bitmap(host, fps.get(), "geo.png"));

      geo->SetPosition(0, base::Vec4(10, 10, 0, 1));
      geo->SetPosition(1, base::Vec4(500, 10, 0, 1));
      geo->SetPosition(2, base::Vec4(10, 500, 0, 1));

      geo->SetTexcoord(0, base::Vec2(10, 10));
      geo->SetTexcoord(1, base::Vec2(500, 10));
      geo->SetTexcoord(2, base::Vec2(10, 500));

      int c = 0;
      while (true) {
        for (auto& it : sprs) {
          it->SetAngle(c);
        }

        bg_spr->Update();

        bg_ple->SetOX(bg_ple->GetOX() + 5);
        bg_ple->SetOY(bg_ple->GetOY() + 5);

        c += 3;
        host->Update();
      }
    }

  } catch (base::Exception e) {
    printf(e.GetErrorMessage().c_str());
  }
}

int SDL_main(int argc, char** argv) {
  SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO);
  IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
  TTF_Init();

  content::CoroutineContext cc;
  cc.primary_fiber = fiber_create(nullptr, 0, nullptr, nullptr);
  cc.main_loop_fiber = fiber_create(cc.primary_fiber, 0, &MainLoop, &cc);

  fpslimiter::FPSLimiter lmt(165);
  fiber_switch(cc.main_loop_fiber);

  while (true) {
    lmt.Delay();

    if (!static_cast<content::Graphics*>(cc.main_loop_fiber->userdata)
             ->ExecuteEventMainLoop())
      break;
  }

  TTF_Quit();
  IMG_Quit();
  SDL_Quit();

  return 0;
}
