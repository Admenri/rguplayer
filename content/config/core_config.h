// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef CONTENT_CONFIG_CORE_CONFIG_H_
#define CONTENT_CONFIG_CORE_CONFIG_H_

#include "base/math/math.h"
#include "base/memory/ref_counted.h"

namespace content {

enum class RGSSVersion : int {
  Null = 0,
  RGSS1,
  RGSS2,
  RGSS3,
};

class CoreConfigure : public base::RefCounted<CoreConfigure> {
 public:
  enum class ANGLERenderer : int {
    Default = 0,
    D3D9,
    D3D11,
    Vulkan,
    Metal,
    Software,
  };

  CoreConfigure() = default;

  CoreConfigure(const CoreConfigure&) = delete;
  CoreConfigure& operator=(const CoreConfigure&) = delete;

  bool LoadConfigure(const std::string& filename);

  RGSSVersion& content_version() { return rgss_version_; }

  ANGLERenderer& angle_renderer() { return angle_renderer_; }
  bool& renderer_debug_output() { return renderer_debug_output_; }
  base::Vec2i& initial_resolution() { return initial_resolution_; }
  bool& allow_frame_skip() { return allow_frame_skip_; }

  std::string& game_rtp() { return game_rtp_; }
  std::string& game_title() { return game_title_; }
  std::string& game_scripts() { return game_scripts_; }

  std::vector<std::string>& load_paths() { return load_paths_; }

 private:
  RGSSVersion rgss_version_;

  ANGLERenderer angle_renderer_;
  bool renderer_debug_output_;
  base::Vec2i initial_resolution_;
  bool allow_frame_skip_;

  std::string game_rtp_;
  std::string game_title_;
  std::string game_scripts_;

  std::vector<std::string> load_paths_;
};

}  // namespace content

#endif  //! CONTENT_CONFIG_CORE_CONFIG_H_
