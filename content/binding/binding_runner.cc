#include "content/binding/binding_runner.h"

#include <SDL_image.h>

#include "content/script/bitmap.h"

namespace content {

BindingRunner::BindingRunner()
    : binding_worker_(std::make_unique<base::ThreadWorker>()) {}

BindingRunner::~BindingRunner() {
  binding_worker_->task_runner()->WaitForSync();
}

scoped_refptr<base::SequencedTaskRunner> BindingRunner::GetBindingRunnerTask() {
  return binding_runner_;
}

void BindingRunner::InitializeBindingInterpreter() {
  binding_worker_->Start(base::RunLoop::MessagePumpType::Worker);
  binding_worker_->WaitUntilStart();
  binding_runner_ = binding_worker_->task_runner();
}

void BindingRunner::PostBindingBoot(BindingParams initial_param) {
  binding_worker_->task_runner()->PostTask(
      base::BindOnce(&BindingRunner::BindingMain,
                     weak_ptr_factory_.GetWeakPtr(), std::move(initial_param)));
}

void BindingRunner::BindingMain(BindingParams initial_param) {
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
  }

  LOG(INFO) << __FUNCTION__;
}

}  // namespace content
