// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_WORKER_THREAD_WORKER_H_
#define BASE_WORKER_THREAD_WORKER_H_

#include <atomic>

#include "base/memory/atomic_flag.h"
#include "base/memory/weak_ptr.h"
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

  void WaitUntilStart();

  scoped_refptr<SequencedTaskRunner> task_runner();

  PlatformThreadHandle GetThreadID();
  bool IsRunning();

 private:
  int DoThreadWork() override;
  void QuitHelper();

  std::unique_ptr<base::PlatformThread> platform_thread_;

  std::string name_;
  base::AtomicFlag booted_thread_;

  std::unique_ptr<RunLoop> run_loop_;

  base::WeakPtrFactory<ThreadWorker> weak_ptr_factory_{this};
};

}  // namespace base

#endif  // BASE_WORKER_THREAD_WORKER_H_