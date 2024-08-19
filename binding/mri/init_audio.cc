// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "binding/mri/init_audio.h"
#include "init_audio.h"

namespace binding {

MRI_METHOD(audio_setup_midi) {
  return Qnil;
}

MRI_METHOD(audio_bgm_play) {
  std::string filename;
  int volume = 100;
  int pitch = 100;
  double pos = 0.0;

  MriParseArgsTo(argc, argv, "s|iif", &filename, &volume, &pitch, &pos);

  scoped_refptr<content::Audio> audio = MriGetGlobalRunner()->audio();
  MRI_GUARD(audio->BGMPlay(filename, volume, pitch, pos););

  return Qnil;
}

MRI_METHOD(audio_bgm_stop) {
  scoped_refptr<content::Audio> audio = MriGetGlobalRunner()->audio();
  audio->BGMStop();

  return Qnil;
}

MRI_METHOD(audio_bgm_fade) {
  int duration;
  MriParseArgsTo(argc, argv, "i", &duration);

  scoped_refptr<content::Audio> audio = MriGetGlobalRunner()->audio();
  audio->BGMFade(duration);

  return Qnil;
}

MRI_METHOD(audio_bgm_pos) {
  scoped_refptr<content::Audio> audio = MriGetGlobalRunner()->audio();
  return rb_float_new(audio->BGMPos());
}

MRI_METHOD(audio_bgs_play) {
  std::string filename;
  int volume = 100;
  int pitch = 100;
  double pos = 0.0;

  MriParseArgsTo(argc, argv, "s|iif", &filename, &volume, &pitch, &pos);

  scoped_refptr<content::Audio> audio = MriGetGlobalRunner()->audio();
  MRI_GUARD(audio->BGSPlay(filename, volume, pitch, pos););

  return Qnil;
}

MRI_METHOD(audio_bgs_stop) {
  scoped_refptr<content::Audio> audio = MriGetGlobalRunner()->audio();
  audio->BGSStop();

  return Qnil;
}

MRI_METHOD(audio_bgs_fade) {
  int duration;
  MriParseArgsTo(argc, argv, "i", &duration);

  scoped_refptr<content::Audio> audio = MriGetGlobalRunner()->audio();
  audio->BGSFade(duration);

  return Qnil;
}

MRI_METHOD(audio_bgs_pos) {
  scoped_refptr<content::Audio> audio = MriGetGlobalRunner()->audio();
  return rb_float_new(audio->BGSPos());
}

MRI_METHOD(audio_me_play) {
  std::string filename;
  int volume = 100;
  int pitch = 100;

  MriParseArgsTo(argc, argv, "s|ii", &filename, &volume, &pitch);

  scoped_refptr<content::Audio> audio = MriGetGlobalRunner()->audio();
  MRI_GUARD(audio->MEPlay(filename, volume, pitch););

  return Qnil;
}

MRI_METHOD(audio_me_stop) {
  scoped_refptr<content::Audio> audio = MriGetGlobalRunner()->audio();
  audio->MEStop();

  return Qnil;
}

MRI_METHOD(audio_me_fade) {
  int duration;
  MriParseArgsTo(argc, argv, "i", &duration);

  scoped_refptr<content::Audio> audio = MriGetGlobalRunner()->audio();
  audio->MEFade(duration);

  return Qnil;
}

MRI_METHOD(audio_se_play) {
  std::string filename;
  int volume = 100;
  int pitch = 100;

  MriParseArgsTo(argc, argv, "s|ii", &filename, &volume, &pitch);

  scoped_refptr<content::Audio> audio = MriGetGlobalRunner()->audio();
  MRI_GUARD(audio->SEPlay(filename, volume, pitch););

  return Qnil;
}

MRI_METHOD(audio_se_stop) {
  scoped_refptr<content::Audio> audio = MriGetGlobalRunner()->audio();
  audio->SEStop();

  return Qnil;
}

MRI_METHOD(audio_reset) {
  scoped_refptr<content::Audio> audio = MriGetGlobalRunner()->audio();
  audio->Reset();

  return Qnil;
}

void InitAudioBinding() {
  VALUE module = rb_define_module("Audio");

  MriDefineModuleFunction(module, "setup_midi", audio_setup_midi);

  MriDefineModuleFunction(module, "bgm_play", audio_bgm_play);
  MriDefineModuleFunction(module, "bgm_stop", audio_bgm_stop);
  MriDefineModuleFunction(module, "bgm_fade", audio_bgm_fade);
  MriDefineModuleFunction(module, "bgm_pos", audio_bgm_pos);
  MriDefineModuleFunction(module, "bgs_play", audio_bgs_play);
  MriDefineModuleFunction(module, "bgs_stop", audio_bgs_stop);
  MriDefineModuleFunction(module, "bgs_fade", audio_bgs_fade);
  MriDefineModuleFunction(module, "bgs_pos", audio_bgs_pos);
  MriDefineModuleFunction(module, "me_play", audio_me_play);
  MriDefineModuleFunction(module, "me_stop", audio_me_stop);
  MriDefineModuleFunction(module, "me_fade", audio_me_fade);
  MriDefineModuleFunction(module, "se_play", audio_se_play);
  MriDefineModuleFunction(module, "se_stop", audio_se_stop);
  MriDefineModuleFunction(module, "__reset__", audio_reset);
}

}  // namespace binding
