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

class RunnerImpl : public SequencedTaskRunner {
 public:
  RunnerImpl(RunLoop::MessagePumpType type) : type_(type) {}
  virtual ~RunnerImpl() {}

  RunnerImpl(const RunnerImpl&) = delete;
  RunnerImpl& operator=(const RunnerImpl&) = delete;

  void PostTask(base::OnceClosure task) override {
    if (task.is_null()) return;

    queue_mutex_.lock();
    task_sequenced_list_.emplace(std::move(task));
    queue_mutex_.unlock();
  }

  base::WeakPtr<RunnerImpl> AsWeakPtr() {
    return weak_ptr_factory_.GetWeakPtr();
  }

  void Run() {
    SDL_Event sdl_event;

    for (;;) {
      if (require_quit_) {
        require_quit_ = false;
        return;
      }

      if (!task_sequenced_list_.empty()) {
        std::move(task_sequenced_list_.front()).Run();
        task_sequenced_list_.pop();
        continue;
      }

      if (SDL_PollEvent(&sdl_event)) {
        if (!g_event_dispatcher[sdl_event.type].is_null()) {
          g_event_dispatcher[sdl_event.type].Run(sdl_event);
        }
        continue;
      }

      // Thread::YieldCurrent
      SDL_Delay(1);
    }
  }

 private:
  friend class RunLoop;
  void RequireQuit() { require_quit_ = true; }

  std::mutex queue_mutex_;
  std::queue<base::OnceClosure> task_sequenced_list_;
  bool require_quit_ = false;

  RunLoop::MessagePumpType type_;
  base::WeakPtrFactory<RunnerImpl> weak_ptr_factory_{this};
};

}  // namespace

void RunLoop::RegisterUnhandledEventFilter(
    Uint32 event_type,
    base::RepeatingCallback<void(const SDL_Event&)> callback) {
  SDL_EventState(event_type, SDL_ENABLE);
  g_event_dispatcher[event_type] = callback;
}

RunLoop::RunLoop() {
  internal_runner_ = base::MakeRefCounted<RunnerImpl>(MessagePumpType::Default);
}

RunLoop::RunLoop(MessagePumpType type) {
  internal_runner_ = base::MakeRefCounted<RunnerImpl>(type);
}

base::OnceClosure RunLoop::QuitClosure() {
  return base::BindOnce(
      &RunnerImpl::RequireQuit,
      static_cast<RunnerImpl*>(internal_runner_.get())->AsWeakPtr());
}

scoped_refptr<SequencedTaskRunner> RunLoop::task_runner() {
  return internal_runner_;
}

void RunLoop::Run() { static_cast<RunnerImpl*>(internal_runner_.get())->Run(); }

void RunLoop::QuitWhenIdle() {
  static_cast<RunnerImpl*>(internal_runner_.get())->require_quit_ = true;
}

}  // namespace base
