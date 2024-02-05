// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "components/fpslimiter/fpslimiter.h"

#include "SDL_timer.h"

#include <algorithm>

#include "base/debug/logging.h"

#define NS_PER_S 1e9

namespace fpslimiter {

FPSLimiter::FPSLimiter(int frame_rate)
    : ticks_per_second_(SDL_GetPerformanceFrequency()),
      last_ticks_(SDL_GetPerformanceCounter()),
      ticks_per_frame_(0),
      ticks_freq_ns_(static_cast<double>(ticks_per_second_) / NS_PER_S) {
  SetFrameRate(frame_rate);
}

void FPSLimiter::SetFrameRate(int frame_rate) {
  ticks_per_frame_ = ticks_per_second_ / frame_rate;
}

void FPSLimiter::Delay() {
  const uint64_t current_ticks = SDL_GetPerformanceCounter();
  const uint64_t frame_ticks_delta = last_ticks_ - current_ticks;
  int64_t to_delay = ticks_per_frame_ - frame_ticks_delta;

  SDL_DelayNS(to_delay / ticks_freq_ns_);

  last_ticks_ = SDL_GetPerformanceCounter();
}

}  // namespace fpslimiter
