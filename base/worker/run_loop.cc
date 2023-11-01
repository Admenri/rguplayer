// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/worker/run_loop.h"

#include <chrono>
#include <mutex>
#include <queue>
#include <thread>

namespace base {

namespace {

base::RepeatingCallbackList<void(const SDL_Event&)> g_event_dispatcher;

std::thread::id g_ui_thread_id;

}  // namespace

class RunnerImpl : public SequencedTaskRunner {
 public:
  RunnerImpl(base::WeakPtr<RunLoop> runner) : runner_(runner) {}
  virtual ~RunnerImpl() {}

  RunnerImpl(const RunnerImpl&) = delete;
  RunnerImpl& operator=(const RunnerImpl&) = delete;

  void PostTask(base::OnceClosure task) override {
    if (runner_) runner_->LockAddTask(std::move(task));
  }

  void WaitForSync() override {
    if (runner_) {
      PostTask(base::BindOnce(&moodycamel::LightweightSemaphore::signal,
                              base::Unretained(&semaphore_), 1));
      semaphore_.wait();
    }
  }

 private:
  base::WeakPtr<RunLoop> runner_;
  moodycamel::LightweightSemaphore semaphore_;
};

bool RunLoop::IsInUIThread() {
  return std::this_thread::get_id() == g_ui_thread_id;
}

base::CallbackListSubscription RunLoop::BindEventDispatcher(
    base::RepeatingCallback<void(const SDL_Event&)> callback) {
  return g_event_dispatcher.Add(std::move(callback));
}

RunLoop::RunLoop() { InitInternal(MessagePumpType::Worker); }

RunLoop::RunLoop(MessagePumpType type) {
  if (type == MessagePumpType::UI) g_ui_thread_id = std::this_thread::get_id();

  InitInternal(type);
}

base::OnceClosure RunLoop::QuitClosure() {
  return base::BindOnce(&RunLoop::RequireQuit, weak_ptr_factory_.GetWeakPtr());
}

scoped_refptr<SequencedTaskRunner> RunLoop::task_runner() {
  return internal_runner_;
}

void RunLoop::Run() {
  for (;;) {
    if (quit_flag_.IsSet()) return;

    // Run once
    if (!DoLoop()) {
      // No work for sleep thread
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }
}

bool RunLoop::DoLoop() {
  base::OnceClosure task;
  if (closure_task_list_.try_dequeue(task)) {
    std::move(task).Run();
    return true;
  }

  if (type_ == MessagePumpType::UI) {
    SDL_Event sdl_event;
    if (SDL_PollEvent(&sdl_event)) {
      if (!g_event_dispatcher.empty()) {
        g_event_dispatcher.Notify(sdl_event);
        return true;
      }
    }
  }

  return false;
}

void RunLoop::QuitWhenIdle() { RequireQuit(); }

void RunLoop::InitInternal(MessagePumpType type) {
  type_ = type;
  internal_runner_ =
      base::MakeRefCounted<RunnerImpl>(weak_ptr_factory_.GetWeakPtr());
}

void RunLoop::RequireQuit() { quit_flag_.Set(); }

void RunLoop::LockAddTask(base::OnceClosure task_closure) {
  if (!task_closure.is_null()) {
    closure_task_list_.enqueue(std::move(task_closure));
  }
}

}  // namespace base
