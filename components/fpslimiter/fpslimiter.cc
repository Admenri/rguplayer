// zlib License
//
// copyright (C) 2024 Admenri
// copyright (C) 2023 Guoxiaomi and Krimiston
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "components/fpslimiter/fpslimiter.h"

#include "SDL_timer.h"
#include "base/debug/logging.h"

#include <algorithm>

#define NS_PER_S 1e9

namespace fpslimiter {

FPSLimiter::FPSLimiter(int frame_rate)
    : counter_(SDL_GetPerformanceCounter()),
      frequency_(SDL_GetPerformanceFrequency()),
      error_counter_(0),
      interval_(1.0 / frame_rate),
      interval_ticks_(std::round(frequency_ * interval_)),
      reset_flag_(false) {}

void FPSLimiter::SetFrameRate(int frame_rate) {
  interval_ = 1.0 / frame_rate;
  interval_ticks_ = std::round(frequency_ * interval_);
}

void FPSLimiter::Delay() {
  uint64_t next_counter = counter_ + interval_ticks_;
  uint64_t before_counter = SDL_GetPerformanceCounter();

  if (before_counter < next_counter) [[likely]] {
    uint64_t delta_counter = (next_counter - before_counter) - error_counter_;
    time_t delay_ns =
        static_cast<time_t>(delta_counter * (NS_PER_S / frequency_));

    SDL_DelayNS(delay_ns);

    counter_ = SDL_GetPerformanceCounter();
    uint64_t real_delay_counter = counter_ - before_counter;
    error_counter_ = static_cast<int64_t>(real_delay_counter - delta_counter);
  } else [[unlikely]] {
    counter_ = before_counter;
    error_counter_ = 0;
  }

  if (reset_flag_) {
    reset_flag_ = false;
    error_counter_ = 0;
  }
}

bool FPSLimiter::RequireFrameSkip() {
  return error_counter_ > interval_ticks_;
}

void FPSLimiter::Reset() {
  reset_flag_ = true;
}

}  // namespace fpslimiter
