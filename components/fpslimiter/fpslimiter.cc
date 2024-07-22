// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "components/fpslimiter/fpslimiter.h"

#include "SDL_timer.h"
#include "base/debug/logging.h"

#include <stdint.h>
#include <algorithm>
#include <cmath>

#define NS_PER_S 1e9

namespace fpslimiter {

FPSLimiter::FPSLimiter(int frame_rate)
    : last_tick_count_(SDL_GetPerformanceCounter()),
      tick_freq_(SDL_GetPerformanceFrequency()),
      tick_freq_ns_((double)tick_freq_ / NS_PER_S),
      skip_last_(SDL_GetPerformanceCounter()),
      skip_ideal_diff_(0),
      skip_reset_flag_(false) {
  SetFrameRate(frame_rate);
}

void FPSLimiter::SetFrameRate(int frame_rate) {
  ticks_per_frame_ = tick_freq_ / frame_rate;
}

void FPSLimiter::Delay() {
  {
    int64_t frame_delta = SDL_GetPerformanceCounter() - last_tick_count_;
    int64_t delay_tick = ticks_per_frame_ - frame_delta;

    delay_tick -= skip_ideal_diff_;
    delay_tick = std::max<int64_t>(0, delay_tick);

    SDL_DelayNS(delay_tick / tick_freq_ns_);

    last_tick_count_ = SDL_GetPerformanceCounter();
  }

  {
    uint64_t skip_now = last_tick_count_;
    int64_t frame_diff = skip_now - skip_last_;
    skip_last_ = skip_now;

    skip_ideal_diff_ += frame_diff - ticks_per_frame_;

    if (skip_reset_flag_)
      skip_ideal_diff_ = 0;
    skip_reset_flag_ = false;
  }
}

bool FPSLimiter::RequireFrameSkip() {
  return skip_ideal_diff_ > ticks_per_frame_;
}

void FPSLimiter::Reset() {
  skip_reset_flag_ = true;
}

}  // namespace fpslimiter
