// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/worker/thread_worker.h"

#include <chrono>

namespace base {

namespace {

class SyncSequencedTaskRunner : public SequencedTaskRunner {
 public:
  void PostTask(base::OnceClosure task) override { std::move(task).Run(); }

  void WaitForSync() override {}
};

}  // namespace

ThreadWorker::ThreadWorker(bool sync_worker)
    : sync_(sync_worker),
      task_runner_(sync_worker ? new SyncSequencedTaskRunner() : nullptr) {}

ThreadWorker::~ThreadWorker() {
  Stop();
}

void ThreadWorker::Start(RunLoop::MessagePumpType message_type) {
  if (sync_)
    return;

  thread_ = std::make_unique<std::jthread>(ThreadWorker::ThreadFunc,
                                           message_type, std::ref(start_flag_),
                                           std::ref(task_runner_));
}

void ThreadWorker::Stop() {
  thread_.reset();
  task_runner_.reset();
}

void ThreadWorker::WaitUntilStart() {
  if (sync_)
    return;

  while (!start_flag_.IsSet())
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

scoped_refptr<base::SequencedTaskRunner> ThreadWorker::task_runner() {
  return task_runner_;
}

void ThreadWorker::ThreadFunc(
    std::stop_token token,
    RunLoop::MessagePumpType message_type,
    base::AtomicFlag& start_flag,
    scoped_refptr<base::SequencedTaskRunner>& runner) {
  base::RunLoop run_loop(message_type);
  runner = run_loop.task_runner();
  start_flag.Set();

  for (;;) {
    if (token.stop_requested())
      break;

    if (!run_loop.DoLoop()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }
}

}  // namespace base
