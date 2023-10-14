// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_WORKER_RUN_LOOP_H_
#define BASE_WORKER_RUN_LOOP_H_

#include <SDL_events.h>

#include <queue>

#include "base/bind/callback.h"
#include "base/memory/lock.h"
#include "base/memory/ref_counted.h"
#include "base/third_party/concurrentqueue/blockingconcurrentqueue.h"

namespace base {

// Template helpers which use function indirection to erase T from the
// function signature while still remembering it so we can call the
// correct destructor/release function.
//
// We use this trick so we don't need to include bind.h in a header
// file like sequenced_task_runner.h. We also wrap the helpers in a
// templated class to make it easier for users of DeleteSoon to
// declare the helper as a friend.
template <class T>
class DeleteHelper {
 private:
  static void DoDelete(const void* object) {
    delete static_cast<const T*>(object);
  }

  friend class SequencedTaskRunner;
};

template <class T>
class DeleteUniquePtrHelper {
 private:
  static void DoDelete(const void* object) {
    // Carefully unwrap `object`. T could have originally been const-qualified
    // or not, and it is important to ensure that the constness matches in order
    // to use the right specialization of std::default_delete<T>...
    std::unique_ptr<T> destroyer(const_cast<T*>(static_cast<const T*>(object)));
  }

  friend class SequencedTaskRunner;
};

template <class T>
class ReleaseHelper {
 private:
  static void DoRelease(const void* object) {
    static_cast<const T*>(object)->Release();
  }

  friend class SequencedTaskRunner;
};

class SequencedTaskRunner : public base::RefCounted<SequencedTaskRunner> {
 public:
  SequencedTaskRunner() = default;
  virtual ~SequencedTaskRunner() = default;

  SequencedTaskRunner(const SequencedTaskRunner&) = delete;
  SequencedTaskRunner& operator=(const SequencedTaskRunner) = delete;

  virtual void PostTask(base::OnceClosure task) = 0;
  virtual void WaitForSync() = 0;

  template <class T>
  bool DeleteSoon(const T* object) {
    return DeleteOrReleaseSoonInternal(&DeleteHelper<T>::DoDelete, object);
  }

  template <class T>
  bool DeleteSoon(std::unique_ptr<T> object) {
    return DeleteOrReleaseSoonInternal(&DeleteUniquePtrHelper<T>::DoDelete,
                                       object.release());
  }

  template <class T>
  void ReleaseSoon(scoped_refptr<T>&& object) {
    if (!object) return;
    DeleteOrReleaseSoonInternal(&ReleaseHelper<T>::DoRelease, object.release());
  }

 private:
  friend class base::RefCountedThreadSafe<SequencedTaskRunner>;
  bool DeleteOrReleaseSoonInternal(void (*deleter)(const void*),
                                   const void* object) {
    PostTask(base::BindOnce(deleter, object));
    return !!object;
  }
};

class RunLoop {
 public:
  static void BindEventDispatcher(
      Uint32 event_type,
      base::RepeatingCallback<void(const SDL_Event&)> callback);
  static bool IsInUIThread();

  enum class MessagePumpType {
    UI = 0,
    IO,
  };

  RunLoop();
  explicit RunLoop(MessagePumpType type);

  RunLoop(const RunLoop&) = delete;
  RunLoop& operator=(const RunLoop&) = delete;

  scoped_refptr<SequencedTaskRunner> task_runner();

  void Run();
  bool DoLoop();
  base::OnceClosure QuitClosure();
  void QuitWhenIdle();

  bool IsQuitRequired() { return quit_flag_.IsSet(); }

 private:
  friend class RunnerImpl;
  void InitInternal(MessagePumpType type);
  void RequireQuit();
  void LockAddTask(base::OnceClosure task_closure);
  scoped_refptr<SequencedTaskRunner> internal_runner_;

  MessagePumpType type_;
  base::AtomicFlag quit_flag_;
  moodycamel::BlockingConcurrentQueue<base::OnceClosure> closure_task_list_;

  base::WeakPtrFactory<RunLoop> weak_ptr_factory_{this};
};

}  // namespace base

#endif  // BASE_WORKER_RUN_LOOP_H_