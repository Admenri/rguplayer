// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/profile/engine_profile.h"

#include "SDL3/SDL_iostream.h"
#include "SDL3/SDL_messagebox.h"
#include "inih/INIReader.h"

namespace content {

namespace {

void ReplaceStringWidth(std::string& str, char before, char after) {
  for (size_t i = 0; i < str.size(); ++i)
    if (str[i] == before)
      str[i] = after;
}

char* IniStreamReader(char* str, int num, void* stream) {
  SDL_IOStream* io = static_cast<SDL_IOStream*>(stream);

  memset(str, 0, num);
  char c;
  int i = 0;

  while (i < num - 1 && SDL_ReadIO(io, &c, 1)) {
    str[i++] = c;
    if (c == '\n')
      break;
  }

  str[i] = '\0';
  return i ? str : nullptr;
}

}  // namespace

void Profile::LoadCommandLine(int argc, char** argv) {
  for (int i = 0; i < argc; ++i)
    args_.push_back(argv[i]);

  game_debug_ = false;
  game_battle_test_ = false;

  for (int i = 0; i < argc; i++) {
    if (std::string(argv[i]) == "test" || std::string(argv[i]) == "debug")
      game_debug_ = true;
    if (std::string(argv[i]) == "btest")
      game_battle_test_ = true;
  }
}

bool Profile::LoadConfigure(filesystem::Filesystem::FileStream* file,
                            const std::string& app) {
  /* Parse configure */
  if (!file) {
    std::string str = "Failed to load configure file.";
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Engine Kernel", str.c_str(),
                             nullptr);
    return false;
  }

  INIReader reader(file, IniStreamReader);
  SDL_CloseIO(file);
  if (reader.ParseError()) {
    std::string str = "Error when parse configure.";
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Engine Kernel", str.c_str(),
                             nullptr);
    return false;
  }

  /* RGSS config part */
  game_rtp_ = reader.Get("Game", "RTP", std::string());
  game_title_ = reader.Get("Game", "Title", "RGU Default Widget");
  game_scripts_ = reader.Get("Game", "Scripts", std::string());
  ReplaceStringWidth(game_scripts_, '\\', '/');

  /* Engine config */
  api_version_ = (APIVersion)reader.GetInteger("Engine", "APIVerison", 0);
  if (api_version_ == APIVersion::Null) {
    if (!game_scripts_.empty()) {
      api_version_ = APIVersion::RGSS1;

      const char* p = &game_scripts_[game_scripts_.size()];
      const char* head = &game_scripts_[0];

      while (--p != head)
        if (*p == '.')
          break;

      if (!strcmp(p, ".rvdata"))
        api_version_ = APIVersion::RGSS2;
      else if (!strcmp(p, ".rvdata2"))
        api_version_ = APIVersion::RGSS3;
    }
  }

  default_font_path_ =
      reader.Get("Engine", "DefaultFontPath", "Fonts/Default.ttf");
  initial_resolution_.x = reader.GetInteger(
      "Engine", "ScreenWidth", api_version_ >= APIVersion::RGSS2 ? 544 : 640);
  initial_resolution_.y = reader.GetInteger(
      "Engine", "ScreenHeight", api_version_ >= APIVersion::RGSS2 ? 416 : 480);
  window_size_.x =
      reader.GetInteger("Engine", "WindowWidth", initial_resolution_.x);
  window_size_.y =
      reader.GetInteger("Engine", "WindowHeight", initial_resolution_.y);
  allow_frame_skip_ = reader.GetBoolean("Renderer", "AllowFrameSkip", false);

  return true;
}

}  // namespace content
