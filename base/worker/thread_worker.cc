// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/worker/thread_worker.h"

namespace base {

ThreadWorker::ThreadWorker(const std::string& name) : name_(name) {}

ThreadWorker::~ThreadWorker() { Stop(); }

void ThreadWorker::Start(RunLoop::MessagePumpType type) {
  run_loop_ = std::make_unique<base::RunLoop>(type);
  quit_closure_ = run_loop_->QuitClosure();
  task_runner_ = run_loop_->task_runner();

  platform_thread_ = PlatformThread::Create(this, name_);
}

void ThreadWorker::Stop() {
  if (quit_closure_) {
    std::move(*platform_thread_).Detach();
  }
}

scoped_refptr<SequencedTaskRunner> ThreadWorker::task_runner() {
  return task_runner_;
}

PlatformThreadHandle ThreadWorker::GetThreadID() {
  return platform_thread_->GetHandle();
}

bool ThreadWorker::IsRunning() { return !!task_runner_.get(); }

int ThreadWorker::DoThreadWork() {
  std::unique_ptr<RunLoop> scoped_run_loop = std::move(run_loop_);
  scoped_run_loop->Run();

  return 0;
}

}  // namespace base
