// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_COMMON_CONTENT_UTILS_H_
#define CONTENT_COMMON_CONTENT_UTILS_H_

namespace content {

// Set engine RGSS kernel adapt version
enum class APIVersion : int {
  Null = 0,
  RGSS1 = 1,
  RGSS2 = 2,
  RGSS3 = 3,
};

}  // namespace content

#endif  //! CONTENT_COMMON_CONTENT_UTILS_H_
