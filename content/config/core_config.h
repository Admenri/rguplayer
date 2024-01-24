// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef CONTENT_CONFIG_CORE_CONFIG_H_
#define CONTENT_CONFIG_CORE_CONFIG_H_

#include "base/memory/ref_counted.h"
#include "nlohmann/json.hpp"

namespace content {

class CoreConfigure : public base::RefCounted<CoreConfigure> {
 public:
  using RGSSVersion = enum {
    RGSS1 = 0,
    RGSS2,
    RGSS3,
  };

  CoreConfigure();
  ~CoreConfigure();

  CoreConfigure(const CoreConfigure&) = delete;
  CoreConfigure& operator=(const CoreConfigure&) = delete;

  RGSSVersion version() { return rgss_version_; }

 private:
  RGSSVersion rgss_version_ = RGSS3;
};

}  // namespace content

#endif  //! CONTENT_CONFIG_CORE_CONFIG_H_
