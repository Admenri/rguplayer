// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef CONTENT_CONFIG_CORE_CONFIG_H_
#define CONTENT_CONFIG_CORE_CONFIG_H_

#include "base/math/math.h"
#include "base/memory/ref_counted.h"

#include <optional>

namespace content {

class CoreConfigure : public base::RefCounted<CoreConfigure> {
 public:
  using RGSSVersion = enum {
    Null = 0,
    RGSS1,
    RGSS2,
    RGSS3,
  };

  using ANGLERenderer = enum {
    DefaultGLES = 0,
    D3D9,
    D3D11,
    Vulkan,
    Metal,
    Software,
  };

  CoreConfigure();
  ~CoreConfigure();

  CoreConfigure(const CoreConfigure&) = delete;
  CoreConfigure& operator=(const CoreConfigure&) = delete;

  std::string& base_path() { return base_path_; }
  RGSSVersion& content_version() { return rgss_version_; }
  std::string& game_title() { return game_title_; }
  std::string& game_scripts() { return game_scripts_; }
  base::Vec2i& initial_resolution() { return initial_resolution_; }
  bool& renderer_debug_output() { return renderer_debug_output_; }
  ANGLERenderer& angle_renderer() { return angle_renderer_; }

 private:
  std::string base_path_;
  RGSSVersion rgss_version_ = RGSS3;
  bool allow_frame_skip_ = false;
  std::string game_title_ = "RGU Widget";
  std::string game_scripts_ = "Data/Scripts.rvdata2";
  base::Vec2i initial_resolution_ = base::Vec2i(544, 416);
  bool renderer_debug_output_ = true;
  ANGLERenderer angle_renderer_ = DefaultGLES;
};

}  // namespace content

#endif  //! CONTENT_CONFIG_CORE_CONFIG_H_
