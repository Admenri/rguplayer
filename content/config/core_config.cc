// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/config/core_config.h"

#include "base/debug/logging.h"
#include "inih/INIReader.h"

namespace content {

bool CoreConfigure::LoadConfigure(const std::string& filename) {
  INIReader reader(filename);

  if (reader.ParseError()) {
    LOG(INFO) << "[Config] Failed to load configure file: " << filename;
    return false;
  }

  /* Core config */
  rgss_version_ = (RGSSVersion)reader.GetInteger("Core", "RGSSVerison", 3);

  /* Renderer config */
  angle_renderer_ = (ANGLERenderer)reader.GetInteger("Renderer", "UseANGLE", 0);
  renderer_debug_output_ = reader.GetBoolean("Renderer", "DebugOutput", false);
  initial_resolution_.x =
      reader.GetInteger("Renderer", "ScreenWidth",
                        rgss_version_ >= RGSSVersion::RGSS2 ? 544 : 640);
  initial_resolution_.y =
      reader.GetInteger("Renderer", "ScreenHeight",
                        rgss_version_ >= RGSSVersion::RGSS2 ? 416 : 480);
  allow_frame_skip_ = reader.GetBoolean("Renderer", "AllowFrameSkip", false);

  /* RGSS config part */
  game_rtp_ = reader.Get("Game", "RTP", "");

  std::string scripts_file;
  switch (rgss_version_) {
    default:
    case content::RGSSVersion::Null:
    case content::RGSSVersion::RGSS1:
      scripts_file = "Data/Scripts.rxdata";
      break;
    case content::RGSSVersion::RGSS2:
      scripts_file = "Data/Scripts.rvdata";
      break;
    case content::RGSSVersion::RGSS3:
      scripts_file = "Data/Scripts.rvdata2";
      break;
  }

  game_scripts_ = reader.Get("Game", "Scripts", scripts_file);
  game_title_ = reader.Get("Game", "Title", "RGU Default Widget");

  /* Filesystem */
  int size = reader.GetInteger("Filesystem", "LoadPathListSize", 0);
  for (int i = 0; i < size; ++i)
    load_paths_.push_back(
        reader.Get("Filesystem", "LoadPath" + std::to_string(i + 1), ""));

  return true;
}

}  // namespace content
