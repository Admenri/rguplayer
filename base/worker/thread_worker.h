// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_WORKER_THREAD_WORKER_H_
#define BASE_WORKER_THREAD_WORKER_H_

#include "base/thread/platform_thread.h"
#include "base/worker/run_loop.h"

namespace base {

class ThreadWorker : public base::PlatformThread::Delegate {
 public:
  ThreadWorker(const std::string& name);
  virtual ~ThreadWorker();

  ThreadWorker(const ThreadWorker&) = delete;
  ThreadWorker& operator=(const ThreadWorker&) = delete;

  void Start(RunLoop::MessagePumpType type);
  void Stop();

  scoped_refptr<SequencedTaskRunner> task_runner();

  PlatformThreadHandle GetThreadID();
  bool IsRunning();

 private:
  int DoThreadWork() override;
  std::unique_ptr<base::PlatformThread> platform_thread_;
  std::string name_;

  scoped_refptr<SequencedTaskRunner> task_runner_;
  OnceClosure quit_closure_;
  std::unique_ptr<RunLoop> run_loop_;
};

}  // namespace base

#endif  // BASE_WORKER_THREAD_WORKER_H_