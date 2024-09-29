
#include <iostream>

#include "content/public/bitmap.h"
#include "content/public/graphics.h"
#include "content/public/sprite.h"

#include "components/filesystem/filesystem.h"

#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"

#include "SDL3_image/SDL_image.h"
#include "SDL3_ttf/SDL_ttf.h"

int SDL_main(int argc, char** argv) {
  SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO);
  IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
  TTF_Init();

  std::unique_ptr<filesystem::Filesystem> fps(
      new filesystem::Filesystem(argv[0]));
  fps->AddLoadPath(".");

  try {
    std::unique_ptr<ui::Widget> win(new ui::Widget());
    ui::Widget::InitParams win_params;
    win_params.size = base::Vec2i(640, 480);
    win->Init(std::move(win_params));

    scoped_refptr<content::Profile> profile = new content::Profile();

    scoped_refptr<content::Graphics> host = new content::Graphics(
        win->AsWeakPtr(), fps.get(), profile, win_params.size);

    {
      scoped_refptr<content::Bitmap> b = new content::Bitmap(host, "test.png");
      scoped_refptr<content::Bitmap> b2 = new content::Bitmap(host, "bg.png");

      b2->Blt({50, 50}, b, {50, 50, 300, 300}, 125);
      b2->SetPixel({10, 10}, new content::Color(0, 0, 0, 255));
      b2->GradientFillRect({0, 0, 100, 100}, new content::Color(0, 0, 0, 255),
                           new content::Color(255, 255, 255, 255), true);
      // b2->DrawText({0, 0, 200, 200}, "test text draw");

      b2->HueChange(125);

      auto* surf = b2->SurfaceRequired();
      IMG_SavePNG(surf, "out.png");

      scoped_refptr<content::Sprite> spr = new content::Sprite(host);
      spr->SetBitmap(b2);

      while (true) {
        SDL_Event e;
        SDL_PollEvent(&e);
        if (e.type == SDL_EVENT_QUIT)
          break;

        host->Update();
      }
    }
  } catch (base::Exception e) {
    printf(e.GetErrorMessage().c_str());
  }

  TTF_Quit();
  IMG_Quit();
  SDL_Quit();

  return 0;
}
