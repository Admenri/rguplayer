// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/config/core_config.h"

#include "base/debug/logging.h"
#include "inih/INIReader.h"

#include "SDL_messagebox.h"

namespace content {

namespace {

void ReplaceStringWidth(std::string& str, char before, char after) {
  for (size_t i = 0; i < str.size(); ++i)
    if (str[i] == before)
      str[i] = after;
}

}  // namespace

void CoreConfigure::LoadCommandLine(int argc, char** argv) {
  game_debug_ = false;
  game_battle_test_ = false;

  for (int i = 0; i < argc; i++) {
    if (std::string(argv[i]) == "test" || std::string(argv[i]) == "debug")
      game_debug_ = true;
    if (std::string(argv[i]) == "btest")
      game_battle_test_ = true;
  }

  if (game_debug_)
    LOG(INFO) << "[App] Running debug test.";
  if (game_battle_test_)
    LOG(INFO) << "[App] Running battle test.";
}

bool CoreConfigure::LoadConfigure(const std::string& filename) {
  /* Parse configure */
  INIReader reader(filename);
  if (reader.ParseError()) {
    std::string str = "Failed to load configure file: " + filename;
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "RGU Core", str.c_str(),
                             nullptr);
    return false;
  }

  /* RGSS config part */
  game_rtp_ = reader.Get("Game", "RTP", std::string());
  game_title_ = reader.Get("Game", "Title", "RGU Default Widget");
  game_scripts_ = reader.Get("Game", "Scripts", std::string());
  ReplaceStringWidth(game_scripts_, '\\', '/');

  /* Core config */
  disable_audio_ = reader.GetBoolean("Core", "DisableAudio", false);
  rgss_version_ = (RGSSVersion)reader.GetInteger("Core", "RGSSVerison", 0);
  if (rgss_version_ == RGSSVersion::Null) {
    if (!game_scripts_.empty()) {
      rgss_version_ = RGSSVersion::RGSS1;

      const char* p = &game_scripts_[game_scripts_.size()];
      const char* head = &game_scripts_[0];

      while (--p != head)
        if (*p == '.')
          break;

      if (!strcmp(p, ".rvdata"))
        rgss_version_ = RGSSVersion::RGSS2;
      else if (!strcmp(p, ".rvdata2"))
        rgss_version_ = RGSSVersion::RGSS3;
    }
  }

  /* Renderer config */
  angle_renderer_ = (ANGLERenderer)reader.GetInteger("Renderer", "UseANGLE", 0);
  renderer_debug_output_ = reader.GetBoolean("Renderer", "DebugOutput", false);
  initial_resolution_.x =
      reader.GetInteger("Renderer", "ScreenWidth",
                        rgss_version_ >= RGSSVersion::RGSS2 ? 544 : 640);
  initial_resolution_.y =
      reader.GetInteger("Renderer", "ScreenHeight",
                        rgss_version_ >= RGSSVersion::RGSS2 ? 416 : 480);
  window_size_.x =
      reader.GetInteger("Renderer", "WindowWidth", initial_resolution_.x);
  window_size_.y =
      reader.GetInteger("Renderer", "WindowHeight", initial_resolution_.y);
  allow_frame_skip_ = reader.GetBoolean("Renderer", "AllowFrameSkip", false);
  smooth_scale_ = reader.GetBoolean("Renderer", "SmoothScale", true);
  keep_ratio_ = reader.GetBoolean("Renderer", "KeepRatio", true);

  /* Filesystem */
  int size = reader.GetInteger("Filesystem", "LoadPathListSize", 0);
  for (int i = 0; i < size; ++i)
    load_paths_.push_back(
        reader.Get("Filesystem", "LoadPath" + std::to_string(i + 1), ""));

  return true;
}

}  // namespace content
