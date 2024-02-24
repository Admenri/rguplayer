// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_FPSLIMITER_FPSLIMITER_H_
#define COMPONENTS_FPSLIMITER_FPSLIMITER_H_

#include <stdint.h>

namespace fpslimiter {

class FPSLimiter {
 public:
  FPSLimiter(int frame_rate);

  FPSLimiter(const FPSLimiter&) = delete;
  FPSLimiter& operator=(const FPSLimiter&) = delete;

  void SetFrameRate(int frame_rate);
  void Delay();
  bool RequireFrameSkip();
  void Reset();

 private:
  uint64_t last_ticks_;
  uint64_t counter_freq_;
  double freq_ns_;
  int64_t error_ticks_;
  double interval_ticks_;

  struct {
    uint64_t last_ticks;
    int64_t frame_diff;
    bool reset;
  } adjust_;
};

}  // namespace fpslimiter

#endif  // !COMPONENTS_FPSLIMITER_FPSLIMITER_H_
