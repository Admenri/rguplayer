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

base::RepeatingCallback<void(const SDL_Event&)>
    g_event_dispatcher[SDL_LASTEVENT];

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

 private:
  base::WeakPtr<RunLoop> runner_;
};

bool RunLoop::IsInUIThread() {
  return std::this_thread::get_id() == g_ui_thread_id;
}

void RunLoop::BindEventDispatcher(
    Uint32 event_type,
    base::RepeatingCallback<void(const SDL_Event&)> callback) {
  SDL_EventState(event_type, SDL_ENABLE);
  g_event_dispatcher[event_type] = callback;
}

RunLoop::RunLoop() {
  InitInternal(IsInUIThread() ? MessagePumpType::IO : MessagePumpType::UI);
}

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
  if (!closure_task_list_.empty()) {
    std::move(closure_task_list_.front()).Run();

    base::AutoLock queue_lock_mutex(queue_lock_);
    closure_task_list_.pop_front();
    return true;
  }

  SDL_Event sdl_event;
  if (SDL_PollEvent(&sdl_event)) {
    if (!g_event_dispatcher[sdl_event.type].is_null()) {
      g_event_dispatcher[sdl_event.type].Run(sdl_event);
      return true;
    }
  }

  return false;
}

void RunLoop::QuitWhenIdle() { RequireQuit(); }

void RunLoop::InitInternal(MessagePumpType type) {
  internal_runner_ =
      base::MakeRefCounted<RunnerImpl>(weak_ptr_factory_.GetWeakPtr());
}

void RunLoop::RequireQuit() { quit_flag_.Set(); }

void RunLoop::LockAddTask(base::OnceClosure task_closure) {
  if (task_closure) {
    base::AutoLock mutex_lock(queue_lock_);
    closure_task_list_.push_back(std::move(task_closure));
  }
}

}  // namespace base
