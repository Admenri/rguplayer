// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/binding/binding_runner.h"

#include <SDL_image.h>

#include "content/script/bitmap.h"
#include "content/script/sprite.h"

namespace content {

thread_local BindingRunner* t_binding_runner_;

BindingRunner::BindingRunner(WorkerTreeHost* tree_host)
    : tree_host_(tree_host),
      binding_worker_(std::make_unique<base::ThreadWorker>()) {}

BindingRunner::~BindingRunner() {
  binding_worker_->task_runner()->WaitForSync();
}

void BindingRunner::InitAndBootBinding(InitParams initial_param) {
  binding_worker_->Start(base::RunLoop::MessagePumpType::Worker);
  binding_worker_->WaitUntilStart();

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

    float xxx = 0;
    for (;;) {
      sp->GetTransform().SetRotation(++xxx);
      GetScreen()->Update();
    }
  }
  LOG(INFO) << __FUNCTION__;
}

}  // namespace content
