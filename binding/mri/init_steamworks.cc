// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "binding/mri/init_steamworks.h"

#include "steam/steam_api.h"

namespace binding {

MRI_METHOD(steam_init) {
  SteamErrMsg msg;
  auto ret = SteamAPI_InitEx(&msg);
  if (ret != k_ESteamAPIInitResult_OK)
    LOG(INFO) << "[Steamworks] " << msg;

  return MRI_BOOL_NEW(ret == k_ESteamAPIInitResult_OK);
}

MRI_METHOD(steam_shutdown) {
  SteamAPI_Shutdown();

  return Qnil;
}

MRI_METHOD(steam_restart) {
  int appid;
  MriParseArgsTo(argc, argv, "i", &appid);

  SteamAPI_RestartAppIfNecessary(appid);

  return Qnil;
}

MRI_METHOD(steam_runcallbacks) {
  SteamAPI_RunCallbacks();

  return Qnil;
}

MRI_METHOD(steam_release_thread) {
  SteamAPI_ReleaseCurrentThreadMemory();

  return Qnil;
}

MRI_METHOD(steam_is_running) {
  return MRI_BOOL_NEW(SteamAPI_IsSteamRunning());
}

MRI_METHOD(steam_install_path) {
  return rb_utf8_str_new_cstr(SteamAPI_GetSteamInstallPath());
}

MRI_METHOD(userstats_request_current) {
  return MRI_BOOL_NEW(SteamUserStats()->RequestCurrentStats());
}

MRI_METHOD(userstats_get_stat_i) {
  std::string chname;
  MriParseArgsTo(argc, argv, "s", &chname);

  int data = 0;
  SteamUserStats()->GetStat(chname.c_str(), &data);

  return rb_fix_new(data);
}

MRI_METHOD(userstats_get_stat_f) {
  std::string chname;
  MriParseArgsTo(argc, argv, "s", &chname);

  float data = 0;
  SteamUserStats()->GetStat(chname.c_str(), &data);

  return rb_float_new(data);
}

MRI_METHOD(userstats_put_stat_i) {
  std::string chname;
  int data;
  MriParseArgsTo(argc, argv, "si", &chname, &data);

  return MRI_BOOL_NEW(SteamUserStats()->SetStat(chname.c_str(), data));
}

MRI_METHOD(userstats_put_stat_f) {
  std::string chname;
  double data;
  MriParseArgsTo(argc, argv, "sf", &chname, &data);

  return MRI_BOOL_NEW(SteamUserStats()->SetStat(chname.c_str(), (float)data));
}

MRI_METHOD(userstats_get_achievement) {
  std::string chname;
  MriParseArgsTo(argc, argv, "s", &chname);

  bool achieved = false;
  SteamUserStats()->GetAchievement(chname.c_str(), &achieved);

  return MRI_BOOL_NEW(achieved);
}

MRI_METHOD(userstats_set_achievement) {
  std::string chname;
  MriParseArgsTo(argc, argv, "s", &chname);

  return MRI_BOOL_NEW(SteamUserStats()->SetAchievement(chname.c_str()));
}

MRI_METHOD(userstats_clear_achievement) {
  std::string chname;
  MriParseArgsTo(argc, argv, "s", &chname);

  return MRI_BOOL_NEW(SteamUserStats()->ClearAchievement(chname.c_str()));
}

void InitSteamworksBinding() {
  // SteamAPI
  VALUE module = rb_define_module("Steamworks");

  MriDefineModuleFunction(module, "init", steam_init);
  MriDefineModuleFunction(module, "shutdown", steam_shutdown);
  MriDefineModuleFunction(module, "restart_if_necessary", steam_restart);
  MriDefineModuleFunction(module, "run_callbacks", steam_runcallbacks);
  MriDefineModuleFunction(module, "release_current_thread_memory",
                          steam_release_thread);
  MriDefineModuleFunction(module, "running?", steam_is_running);
  MriDefineModuleFunction(module, "install_path", steam_install_path);

  // SteamUserStats
  module = rb_define_module("SteamUserStats");
  MriDefineModuleFunction(module, "request_current_stats",
                          userstats_request_current);
  MriDefineModuleFunction(module, "get_stat", userstats_get_stat_i);
  MriDefineModuleFunction(module, "get_stat_f", userstats_get_stat_f);
  MriDefineModuleFunction(module, "set_stat", userstats_put_stat_i);
  MriDefineModuleFunction(module, "set_stat_f", userstats_put_stat_f);

  MriDefineModuleFunction(module, "get_achievement", userstats_get_achievement);
  MriDefineModuleFunction(module, "set_achievement", userstats_set_achievement);
  MriDefineModuleFunction(module, "clear_achievement",
                          userstats_clear_achievement);
}

}  // namespace binding
