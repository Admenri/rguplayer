// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/binding/binding_runner.h"

#include <SDL_image.h>

#include "content/script/bitmap.h"
#include "content/script/plane.h"
#include "content/script/sprite.h"
#include "content/script/window.h"

namespace content {

thread_local BindingRunner* t_binding_runner_;

BindingRunner::BindingRunner(WorkerTreeHost* tree_host)
    : tree_host_(tree_host),
      binding_worker_(std::make_unique<base::ThreadWorker>()) {}

BindingRunner::~BindingRunner() {
  binding_worker_->task_runner()->WaitForSync();
}

void BindingRunner::InitThread() {
  binding_worker_->Start(base::RunLoop::MessagePumpType::Worker);
  binding_worker_->WaitUntilStart();
}

void BindingRunner::RunMainAsync(InitParams initial_param) {
  binding_worker_->task_runner()->PostTask(
      base::BindOnce(&BindingRunner::BindingMain,
                     weak_ptr_factory_.GetWeakPtr(), std::move(initial_param)));
}

scoped_refptr<base::SequencedTaskRunner> BindingRunner::GetTaskRunner() {
  return binding_worker_->task_runner();
}

BindingRunner* BindingRunner::Get() { return t_binding_runner_; }

scoped_refptr<base::SequencedTaskRunner> BindingRunner::GetRenderer() {
  return tree_host_->GetRenderTaskRunner();
}

void BindingRunner::InitInternalModules(const InitParams& initial_param) {
  t_binding_runner_ = this;

  modules_.graphics = std::make_unique<Graphics>(initial_param.window,
                                                 initial_param.resolution);

  modules_.input = std::make_unique<Input>(initial_param.window);

  std::unique_ptr<Input::KeySymMap> key_binding(new Input::KeySymMap);
  (*key_binding)[SDL_SCANCODE_RSHIFT] = "A";

  modules_.input->ApplyKeySymBinding(std::move(*key_binding));
}

void BindingRunner::BindingMain(InitParams initial_param) {
  InitInternalModules(initial_param);

  LOG(INFO) << __FUNCTION__;
  {
    scoped_refptr<Bitmap> bmp =
        new Bitmap("D:\\Desktop\\rgu\\app\\test\\example.png");

    scoped_refptr<Bitmap> sampler =
        new Bitmap("D:\\Desktop\\rgu\\app\\test\\test.png");

    bmp->ClearRect(base::Rect(20, 20, 50, 50));
    bmp->Blt(100, 100, sampler, sampler->GetRect()->AsBase(), 255);

    bmp->GradientFillRect(base::Rect(50, 100, 100, 50),
                          new Color(0, 255, 0, 125), new Color(0, 0, 255, 125));

    /* Sync method test */
    SDL_Surface* surf = bmp->SurfaceRequired();
    IMG_SavePNG(surf, "D:\\Desktop\\snap.png");

    scoped_refptr<Sprite> sp = new Sprite();
    sp->SetBitmap(bmp);
    sp->GetTransform().SetOrigin(
        base::Vec2i(sp->GetWidth() / 2, sp->GetHeight() / 2));
    sp->GetTransform().SetPosition(
        base::Vec2i(GetScreen()->GetWidth() / 2, GetScreen()->GetHeight() / 2));

    sp->SetBushDepth(100);
    sp->SetBushOpacity(128);

    sp->SetWaveAmp(20);
    sp->GetSrcRect()->Set(base::Rect(100, 100, 100, 100));

    scoped_refptr<Plane> pl = new Plane();
    pl->SetBitmap(new Bitmap("D:\\Desktop\\rgu\\app\\test\\bg.png"));

    scoped_refptr<Window2> vx_win = new Window2(100, 100, 300, 300);
    vx_win->SetWindowskin(
        new Bitmap("D:\\Desktop\\rgu\\app\\test\\Window.png"));
    vx_win->SetZ(100);
    vx_win->GetTone()->Set(-68, -68, 68, 0);
    vx_win->SetPause(true);

    vx_win->SetContents(bmp);

    vx_win->SetActive(true);
    vx_win->SetCursorRect(new Rect(base::Rect(110, 110, 100, 100)));

    vx_win->SetArrowsVisible(true);

    float xxx = 0;
    for (;;) {
      sp->Update();
      sp->GetTransform().SetRotation(++xxx);

      GetScreen()->Update();

      GetInput()->Update();

      if (!vx_win->IsDisposed()) vx_win->Update();

      if (GetInput()->IsTriggered("A")) LOG(INFO) << "ID Trigger.";

      if (GetInput()->KeyTriggered(SDL_SCANCODE_F)) vx_win->Dispose();

      if (GetInput()->KeyRepeated(SDL_SCANCODE_F)) LOG(INFO) << "F Key Repeat.";

      if (auto i = GetInput()->Dir8()) LOG(INFO) << "Dir8:" << i;
    }
  }
  LOG(INFO) << __FUNCTION__;
}

}  // namespace content
