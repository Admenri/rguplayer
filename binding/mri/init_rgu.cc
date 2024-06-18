// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "binding/mri/init_rgu.h"

#include <iostream>

#include "SDL_locale.h"
#include "SDL_messagebox.h"
#include "SDL_misc.h"
#include "SDL_platform.h"
#include "SDL_timer.h"

namespace binding {

MRI_METHOD(rgu_get_locate) {
  auto* locale = SDL_GetPreferredLocales();

  VALUE hash = rb_hash_new();
  if (locale->country)
    rb_hash_aset(hash, rb_str_new2("country"), rb_str_new2(locale->country));
  if (locale->language)
    rb_hash_aset(hash, rb_str_new2("language"), rb_str_new2(locale->language));

  return hash;
}

MRI_METHOD(rgu_open_url) {
  std::string url;
  MriParseArgsTo(argc, argv, "s", &url);

  SDL_OpenURL(url.c_str());

  return Qnil;
}

MRI_METHOD(rgu_reset) {
  MriGetGlobalRunner()->RequestReset();
  return Qnil;
}

MRI_METHOD(rgu_get_counter) {
  return rb_uint2big(SDL_GetPerformanceCounter());
}

MRI_METHOD(rgu_get_counter_freq) {
  return rb_uint2big(SDL_GetPerformanceFrequency());
}

MRI_METHOD(rgu_msgbox) {
  int flags;
  std::string title;
  std::string msg;
  MriParseArgsTo(argc, argv, "iss", &flags, &title, &msg);

  return INT2FIX(
      SDL_ShowSimpleMessageBox(flags, title.c_str(), msg.c_str(), nullptr));
}

MRI_METHOD(rgu_console_puts) {
  VALUE dispString = rb_str_buf_new(128);
  ID conv = rb_intern("inspect");

  for (int i = 0; i < argc; ++i) {
    VALUE str = rb_funcall2(argv[i], conv, 0, NULL);
    rb_str_buf_append(dispString, str);

    if (i < argc)
      rb_str_buf_cat2(dispString, " ");
  }

  LOG(INFO) << "[Console] " << RSTRING_PTR(dispString);
  return Qnil;
}

MRI_METHOD(rgu_console_gets) {
  std::string str;
  std::cin >> str;

  return rb_utf8_str_new(str.c_str(), str.size());
}

MRI_METHOD(rgu_delay) {
  MriCheckArgc(argc, 1);
  SDL_Delay(FIX2INT(argv[0]));
  return Qnil;
}

void InitRGUBinding() {
  scoped_refptr<content::BindingRunner> runner = MriGetGlobalRunner();

  VALUE module = rb_define_module("RGU");

  // Constant define
  rb_const_set(module, rb_intern("CONTENTVERSION"),
               INT2FIX((int)runner->rgss_version()));
  rb_const_set(module, rb_intern("SDLVERSION"), INT2FIX(SDL_VERSION));
  rb_const_set(module, rb_intern("PLATFORM"), rb_str_new2(SDL_GetPlatform()));

  // Locale in host
  MriDefineModuleFunction(module, "get_locale", rgu_get_locate);

  // Open url
  MriDefineModuleFunction(module, "open_url", rgu_open_url);

  // Reset game
  MriDefineModuleFunction(module, "reset_engine", rgu_reset);

  // Graphics Etc
  MriDefineModuleFunction(module, "get_counter", rgu_get_counter);
  MriDefineModuleFunction(module, "get_counter_freq", rgu_get_counter_freq);
  MriDefineModuleFunction(module, "delay", rgu_delay);

  // Etc
  MriDefineModuleFunction(module, "msgbox", rgu_msgbox);

  // Console operate
  VALUE console = rb_define_module_under(module, "Console");
  MriDefineModuleFunction(console, "puts", rgu_console_puts);
  MriDefineModuleFunction(console, "gets", rgu_console_gets);
}

}  // namespace binding
