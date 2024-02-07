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
  uint64_t counter_;
  uint64_t frequency_;
  int64_t error_counter_;
  double interval_;
  double interval_ticks_;
  bool reset_flag_;
};

}  // namespace fpslimiter

#endif  // !COMPONENTS_FPSLIMITER_FPSLIMITER_H_
