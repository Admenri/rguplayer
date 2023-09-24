// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/worker/thread_worker.h"

#include <SDL_timer.h>

namespace base {

ThreadWorker::ThreadWorker(const std::string& name) : name_(name) {}

ThreadWorker::~ThreadWorker() {
  Stop();

  // Detach thread
  platform_thread_.reset();
}

void ThreadWorker::Start(RunLoop::MessagePumpType type) {
  platform_thread_ = PlatformThread::Create(this, name_);
}

void ThreadWorker::Stop() {
  run_loop_->task_runner()->PostTask(base::BindOnce(
      &ThreadWorker::QuitHelper, weak_ptr_factory_.GetWeakPtr()));
}

void ThreadWorker::WaitUntilStart() {
  if (booted_thread_.IsSet()) return;

  while (!booted_thread_.IsSet()) {
    SDL_Delay(1);
  }
}

scoped_refptr<SequencedTaskRunner> ThreadWorker::task_runner() {
  if (!run_loop_) return nullptr;
  return run_loop_->task_runner();
}

PlatformThreadHandle ThreadWorker::GetThreadID() {
  return platform_thread_->GetHandle();
}

bool ThreadWorker::IsRunning() { return !!task_runner().get(); }

int ThreadWorker::DoThreadWork() {
  run_loop_ = std::make_unique<RunLoop>();

  booted_thread_.Set();
  run_loop_->Run();

  run_loop_.reset();

  return 0;
}

void ThreadWorker::QuitHelper() {}

}  // namespace base
