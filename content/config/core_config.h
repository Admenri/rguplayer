// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef CONTENT_CONFIG_CORE_CONFIG_H_
#define CONTENT_CONFIG_CORE_CONFIG_H_

#include "base/math/math.h"
#include "base/memory/ref_counted.h"

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

  RGSSVersion& version() { return rgss_version_; }
  bool& allow_frame_skip() { return allow_frame_skip_; }
  std::string& game_title() { return game_title_; }
  std::string& game_scripts() { return game_scripts_; }
  base::Vec2i& initial_resolution() { return initial_resolution_; }
  bool& renderer_debug_output() { return renderer_debug_output_; }

 private:
  RGSSVersion rgss_version_ = RGSS3;
  bool allow_frame_skip_ = false;
  std::string game_title_ = "RGU Widget";
  std::string game_scripts_ = "Data/Scripts.rvdata2";
  base::Vec2i initial_resolution_ = base::Vec2i(544, 416);
  bool renderer_debug_output_ = true;
};

}  // namespace content

#endif  //! CONTENT_CONFIG_CORE_CONFIG_H_
