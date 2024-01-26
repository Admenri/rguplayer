// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_FPSLIMITER_FPSLIMITER_H_
#define COMPONENTS_FPSLIMITER_FPSLIMITER_H_

#include <stdint.h>

namespace fpslimiter {

class FPSLimiter {
 public:
  FPSLimiter();
  ~FPSLimiter();

  FPSLimiter(const FPSLimiter&) = delete;
  FPSLimiter& operator=(const FPSLimiter&) = delete;

  void SetFrameRate(int frame_rate);
  void ResetFrameSkipCap();

  void Delay();
  bool FrameSkipRequired();

 private:
  uint64_t ticks_per_second_;
  uint64_t last_ticks_;
  int64_t ticks_per_frame_;
  uint64_t frame_ticks_diff_;
  int64_t frame_skip_cap_;
  double ticks_freq_ns_;

  bool reset_cap_required_ = false;
};

}  // namespace fpslimiter

#endif  // !COMPONENTS_FPSLIMITER_FPSLIMITER_H_
