// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/worker/thread_worker.h"

namespace base {

ThreadWorker::ThreadWorker(const std::string& name) : name_(name) {}

ThreadWorker::~ThreadWorker() { Stop(); }

void ThreadWorker::Start(RunLoop::MessagePumpType type) {
  run_loop_ = std::make_unique<base::RunLoop>(type);
  platform_thread_ = PlatformThread::Create(this, name_);
}

void ThreadWorker::Stop() {
  if (run_loop_) {
    std::move(run_loop_->QuitClosure()).Run();
  }
}

void ThreadWorker::StopSoon() { Stop(); }

scoped_refptr<SequencedTaskRunner> ThreadWorker::task_runner() {
  if (!run_loop_) return nullptr;
  return run_loop_->task_runner();
}

PlatformThreadHandle ThreadWorker::GetThreadID() {
  return platform_thread_->GetHandle();
}

bool ThreadWorker::IsRunning() { return !!run_loop_.get(); }

int ThreadWorker::DoThreadWork() {
  run_loop_->Run();

  return 0;
}

}  // namespace base
