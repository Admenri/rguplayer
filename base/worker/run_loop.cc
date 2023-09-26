// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/worker/run_loop.h"

#include <SDL_timer.h>

#include <mutex>
#include <queue>

namespace base {

namespace {

base::RepeatingCallback<void(const SDL_Event&)>
    g_event_dispatcher[SDL_LASTEVENT];

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

void RunLoop::BindEventDispatcher(
    Uint32 event_type,
    base::RepeatingCallback<void(const SDL_Event&)> callback) {
  SDL_EventState(event_type, SDL_ENABLE);
  g_event_dispatcher[event_type] = callback;
}

RunLoop::RunLoop() { InitInternal(MessagePumpType::UI); }

RunLoop::RunLoop(MessagePumpType type) { InitInternal(type); }

base::OnceClosure RunLoop::QuitClosure() {
  return base::BindOnce(&RunLoop::RequireQuit, weak_ptr_factory_.GetWeakPtr());
}

scoped_refptr<SequencedTaskRunner> RunLoop::task_runner() {
  return internal_runner_;
}

void RunLoop::Run() {
  for (;;) {
    if (quit_flag_.IsSet()) return;

    {
      base::AutoLock queue_lock_mutex(queue_lock_);
      if (!closure_task_list_.empty() &&
          !closure_task_list_.front().is_null()) {
        std::move(closure_task_list_.front()).Run();
        closure_task_list_.pop();
        continue;
      }
    }

    SDL_Event sdl_event;
    if (SDL_PollEvent(&sdl_event)) {
      if (!g_event_dispatcher[sdl_event.type].is_null()) {
        g_event_dispatcher[sdl_event.type].Run(sdl_event);
        continue;
      }
    }

    // No work for sleep thread
    SDL_Delay(1);
  }
}

void RunLoop::QuitWhenIdle() { RequireQuit(); }

void RunLoop::InitInternal(MessagePumpType type) {
  internal_runner_ =
      base::MakeRefCounted<RunnerImpl>(weak_ptr_factory_.GetWeakPtr());
}

void RunLoop::RequireQuit() { quit_flag_.Set(); }

void RunLoop::LockAddTask(base::OnceClosure task_closure) {
  base::AutoLock mutex_lock(queue_lock_);
  closure_task_list_.push(std::move(task_closure));
}

}  // namespace base
