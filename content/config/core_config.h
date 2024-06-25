// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef CONTENT_CONFIG_CORE_CONFIG_H_
#define CONTENT_CONFIG_CORE_CONFIG_H_

#include "base/math/math.h"
#include "base/memory/ref_counted.h"
#include "components/filesystem/filesystem.h"

#include <unordered_map>

namespace content {

// Set engine RGSS kernel adapt version
enum class RGSSVersion : int {
  Null = 0,
  RGSS1 = 1,
  RGSS2 = 2,
  RGSS3 = 3,
};

class CoreConfigure : public base::RefCounted<CoreConfigure> {
 public:
  // ANGLE Graphics API backend choose
  enum class ANGLERenderer : int32_t {
    DefaultES = 0,
    D3D9,
    D3D11,
    Vulkan,
    Metal,
    Software,
  };

  CoreConfigure() = default;

  CoreConfigure(const CoreConfigure&) = delete;
  CoreConfigure& operator=(const CoreConfigure&) = delete;

  void LoadCommandLine(int argc, char** argv);
  bool LoadConfigure(SDL_IOStream* filestream, const std::string& app);
  void Loadi18nXML(filesystem::Filesystem* io);
  std::string GetI18NString(int ids, const std::string& default_value);

  inline std::vector<std::string>& args() { return args_; }
  inline std::string& executable_file() { return executable_file_; }

  inline std::string& game_rtp() { return game_rtp_; }
  inline std::string& game_title() { return game_title_; }
  inline std::string& game_scripts() { return game_scripts_; }
  inline bool& game_debug() { return game_debug_; }
  inline bool& game_battle_test() { return game_battle_test_; }

  inline RGSSVersion& content_version() { return rgss_version_; }
  inline bool& disable_audio() { return disable_audio_; }
  inline bool& async_renderer() { return async_renderer_; }

  inline ANGLERenderer& angle_renderer() { return angle_renderer_; }
  inline bool& renderer_debug_output() { return renderer_debug_output_; }
  inline base::Vec2i& initial_resolution() { return initial_resolution_; }
  inline base::Vec2i& window_size() { return window_size_; }
  inline bool& allow_frame_skip() { return allow_frame_skip_; }
  inline bool& smooth_scale() { return smooth_scale_; }
  inline bool& keep_ratio() { return keep_ratio_; }
  inline bool& fullscreen() { return fullscreen_; }

  inline std::vector<std::string>& load_paths() { return load_paths_; }
  inline std::string& default_font_path() { return default_font_path_; }

  inline std::string& i18n_xml_path() { return i18n_xml_path_; }

 private:
  std::vector<std::string> args_;
  std::string executable_file_;

  std::string game_rtp_;
  std::string game_title_;
  std::string game_scripts_;
  bool game_debug_;
  bool game_battle_test_;

  RGSSVersion rgss_version_;
  bool disable_audio_;
  bool async_renderer_;

  ANGLERenderer angle_renderer_;
  bool renderer_debug_output_;
  base::Vec2i initial_resolution_;
  base::Vec2i window_size_;
  bool allow_frame_skip_;
  bool smooth_scale_;
  bool keep_ratio_;
  bool fullscreen_;

  std::vector<std::string> load_paths_;
  std::string default_font_path_;

  std::string i18n_xml_path_;
  std::unordered_map<int, std::string> i18n_translation_;
};

}  // namespace content

#endif  //! CONTENT_CONFIG_CORE_CONFIG_H_
