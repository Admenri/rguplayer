// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "binding/mri/init_rgu.h"
#include "init_rgu.h"

#include "SDL_locale.h"
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
}

}  // namespace binding
