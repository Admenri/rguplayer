// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "components/fpslimiter/fpslimiter.h"

#include "SDL_timer.h"

#include <algorithm>

#include "base/debug/logging.h"

#define NS_PER_S 1000000000

namespace fpslimiter {

FPSLimiter::FPSLimiter()
    : ticks_per_second_(SDL_GetPerformanceFrequency()),
      last_ticks_(SDL_GetPerformanceCounter()),
      ticks_per_frame_(0),
      frame_ticks_diff_(0),
      frame_skip_cap_(0),
      ticks_freq_ns_(static_cast<double>(ticks_per_second_) / NS_PER_S) {}

FPSLimiter::~FPSLimiter() {}

void FPSLimiter::SetFrameRate(int frame_rate) {
  ticks_per_frame_ = ticks_per_second_ / frame_rate;
}

void FPSLimiter::ResetFrameSkipCap() {
  reset_cap_required_ = true;
}

void FPSLimiter::Delay() {
  const uint64_t current_ticks = SDL_GetPerformanceCounter();
  const uint64_t ticks_delta = last_ticks_ - current_ticks;
  int64_t to_delay = ticks_per_frame_ - ticks_delta;

  to_delay -= frame_skip_cap_;
  to_delay = std::max<int64_t>(0, to_delay);

  SDL_DelayNS(to_delay / ticks_freq_ns_);

  const uint64_t now_ticks = last_ticks_ = SDL_GetPerformanceCounter();
  const int64_t ticks_diff = now_ticks - frame_ticks_diff_;
  frame_ticks_diff_ = now_ticks;

  frame_skip_cap_ += ticks_diff - ticks_per_frame_;

  if (reset_cap_required_) {
    frame_skip_cap_ = 0;
    reset_cap_required_ = false;
  }
}

bool FPSLimiter::FrameSkipRequired() {
  return frame_skip_cap_ > ticks_per_frame_;
}

}  // namespace fpslimiter
