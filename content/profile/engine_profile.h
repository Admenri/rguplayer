// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PROFILE_ENGINE_PROFILE_H_
#define CONTENT_PROFILE_ENGINE_PROFILE_H_

#include "base/math/rectangle.h"
#include "base/memory/ref_counted.h"
#include "components/filesystem/filesystem.h"

#include <unordered_map>

namespace content {

// Set engine RGSS kernel adapt version
enum class APIVersion : int {
  Null = 0,
  RGSS1 = 1,
  RGSS2 = 2,
  RGSS3 = 3,
};

class Profile : public base::RefCounted<Profile> {
 public:
  Profile() = default;

  Profile(const Profile&) = delete;
  Profile& operator=(const Profile&) = delete;

  void LoadCommandLine(int argc, char** argv);
  bool LoadConfigure(filesystem::Filesystem::FileStream* file,
                     const std::string& app);

  inline std::vector<std::string>& args() { return args_; }
  inline std::string& executable_file() { return executable_file_; }

  inline std::string& game_rtp() { return game_rtp_; }
  inline std::string& game_title() { return game_title_; }
  inline std::string& game_scripts() { return game_scripts_; }
  inline bool& game_debug() { return game_debug_; }
  inline bool& game_battle_test() { return game_battle_test_; }

  inline APIVersion& content_version() { return api_version_; }
  inline std::string& default_font_path() { return default_font_path_; }
  inline base::Vec2i& initial_resolution() { return initial_resolution_; }
  inline base::Vec2i& window_size() { return window_size_; }
  inline bool& allow_frame_skip() { return allow_frame_skip_; }

 private:
  std::vector<std::string> args_;
  std::string executable_file_;

  std::string game_rtp_;
  std::string game_title_;
  std::string game_scripts_;
  bool game_debug_;
  bool game_battle_test_;

  APIVersion api_version_;
  std::string default_font_path_;
  base::Vec2i initial_resolution_;
  base::Vec2i window_size_;
  bool allow_frame_skip_;
};

}  // namespace content

#endif  //! CONTENT_PROFILE_ENGINE_PROFILE_H_
