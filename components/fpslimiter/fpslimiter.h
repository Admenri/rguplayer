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

 private:
  uint64_t counter_;
  uint64_t frequency_;
  int64_t error_counter_;
  uint32_t period_min_;
  double interval_;
};

}  // namespace fpslimiter

#endif  // !COMPONENTS_FPSLIMITER_FPSLIMITER_H_
