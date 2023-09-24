// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/thread/platform_thread.h"

#include <SDL_thread.h>

namespace base {

namespace {

int SDLCALL ThreadWorkProcess(void* userdata) {
  PlatformThread::Delegate* delegate =
      static_cast<PlatformThread::Delegate*>(userdata);

  return delegate->DoThreadWork();
}

}  // namespace

PlatformThread::~PlatformThread() { SDL_DetachThread(sdl_thread_); }

std::unique_ptr<PlatformThread> PlatformThread::Create(
    Delegate* delegate, const std::string& threading_name) {
  return std::unique_ptr<PlatformThread>(
      new PlatformThread(delegate, threading_name));
}

PlatformThreadHandle PlatformThread::GetCurrentThreadHandle() {
  return SDL_ThreadID();
}

std::string PlatformThread::GetName() { return SDL_GetThreadName(sdl_thread_); }

PlatformThreadHandle PlatformThread::GetHandle() {
  return SDL_GetThreadID(sdl_thread_);
}

int PlatformThread::Join() {
  int status = 0;
  SDL_WaitThread(sdl_thread_, &status);
  return status;
}

void PlatformThread::Detach() && { SDL_DetachThread(sdl_thread_); }

PlatformThread::PlatformThread(Delegate* delegate,
                               const std::string& threading_name) {
  sdl_thread_ =
      SDL_CreateThread(ThreadWorkProcess, threading_name.c_str(), delegate);
}

}  // namespace base
