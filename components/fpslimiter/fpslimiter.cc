// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "components/fpslimiter/fpslimiter.h"

#include "SDL_timer.h"
#include "base/debug/logging.h"

#include <algorithm>
#include <cmath>

#define NS_PER_S 1e9

namespace fpslimiter {

FPSLimiter::FPSLimiter(int frame_rate)
    : last_ticks_(SDL_GetPerformanceCounter()),
      counter_freq_(SDL_GetPerformanceFrequency()),
      freq_ns_(NS_PER_S / counter_freq_),
      error_ticks_(0),
      interval_ticks_(std::round((double)counter_freq_ / frame_rate)),
      adjust_{last_ticks_, 0, false} {}

void FPSLimiter::SetFrameRate(int frame_rate) {
  interval_ticks_ = std::round((double)counter_freq_ / frame_rate);
}

void FPSLimiter::Delay() {
  uint64_t expect_ticks = last_ticks_ + interval_ticks_;
  uint64_t real_ticks = SDL_GetPerformanceCounter();
  uint64_t diff_counter = 0;

  if (expect_ticks > real_ticks) [[likely]] {
    int64_t delta_ticks = (expect_ticks - real_ticks) - error_ticks_;
    time_t delay_ns = static_cast<time_t>(delta_ticks * freq_ns_);

    if (delay_ns > 0 && delay_ns < NS_PER_S) {
      SDL_DelayNS(delay_ns);
    }

    last_ticks_ = SDL_GetPerformanceCounter();
    uint64_t real_delay_counter = last_ticks_ - real_ticks;
    error_ticks_ = static_cast<int64_t>(real_delay_counter - delta_ticks);
    diff_counter = last_ticks_;
  } else [[unlikely]] {
    last_ticks_ = real_ticks;
    error_ticks_ = 0;
  }

  if (!diff_counter)
    diff_counter = SDL_GetPerformanceCounter();
  int64_t frame_diff = diff_counter - adjust_.last_ticks;
  adjust_.last_ticks = diff_counter;
  adjust_.frame_diff += frame_diff - interval_ticks_;

  if (adjust_.reset) {
    adjust_.reset = false;
    adjust_.frame_diff = 0;
  }
}

bool FPSLimiter::RequireFrameSkip() {
  return adjust_.frame_diff >= interval_ticks_;
}

void FPSLimiter::Reset() {
  adjust_.reset = true;
}

}  // namespace fpslimiter
