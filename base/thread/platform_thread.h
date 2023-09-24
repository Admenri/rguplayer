// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_THREAD_PLATFORM_THREAD_H_
#define BASE_THREAD_PLATFORM_THREAD_H_

#include <string>

#include "base/memory/ref_counted.h"

struct SDL_Thread;

namespace base {

using PlatformThreadHandle = unsigned long;

class PlatformThread {
 public:
  class Delegate {
   public:
    virtual ~Delegate() = default;

    virtual int DoThreadWork() = 0;
  };

  PlatformThread() = delete;
  PlatformThread(const PlatformThread&) = delete;
  PlatformThread& operator=(const PlatformThread&) = delete;
  virtual ~PlatformThread();

  static std::unique_ptr<PlatformThread> Create(
      Delegate* delegate, const std::string& threading_name);
  static PlatformThreadHandle GetCurrentThreadHandle();

  std::string GetName();
  PlatformThreadHandle GetHandle();

  int Join();

  void Detach() &&;

 private:
  PlatformThread(Delegate* delegate, const std::string& threading_name);
  SDL_Thread* sdl_thread_;
};

}  // namespace base

#endif  // BASE_THREAD_PLATFORM_THREAD_H_