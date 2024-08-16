// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "binding/mri/init_aomdecode.h"

#include "binding/mri/init_bitmap.h"
#include "binding/mri/mri_template.h"
#include "content/public/aomdecoder.h"

namespace binding {

MRI_DEFINE_DATATYPE_REF(AOMDecoder, "AOMDecoder", content::AOMDecoder);

MRI_METHOD(aom_initialize) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  scoped_refptr<content::AOMDecoder> decoder = new content::AOMDecoder(screen);

  decoder->AddRef();
  MriSetStructData(self, decoder.get());

  return self;
}

MRI_METHOD(aom_load_video) {
  scoped_refptr<content::AOMDecoder> obj =
      MriGetStructData<content::AOMDecoder>(self);

  std::string filename;
  MriParseArgsTo(argc, argv, "s", &filename);

  uvpx::Player::LoadResult ret;
  MRI_GUARD(ret = obj->LoadVideo(filename););

  return INT2FIX((int)ret);
}

MRI_METHOD(aom_update) {
  scoped_refptr<content::AOMDecoder> obj =
      MriGetStructData<content::AOMDecoder>(self);

  obj->Update();

  return Qnil;
}

MRI_METHOD(aom_set_state) {
  scoped_refptr<content::AOMDecoder> obj =
      MriGetStructData<content::AOMDecoder>(self);

  int state = 0;
  MriParseArgsTo(argc, argv, "i", &state);
  obj->SetPlayState((content::AOMDecoder::Type)state);

  return Qnil;
}

MRI_METHOD(aom_get_state) {
  scoped_refptr<content::AOMDecoder> obj =
      MriGetStructData<content::AOMDecoder>(self);

  auto ret = obj->GetPlayState();

  return INT2FIX((int)ret);
}

MRI_METHOD(aom_render) {
  scoped_refptr<content::AOMDecoder> obj =
      MriGetStructData<content::AOMDecoder>(self);

  VALUE propObj = *argv;
  scoped_refptr<content::Bitmap> prop;
  if (!NIL_P(propObj))
    prop = MriCheckStructData<content::Bitmap>(propObj, kBitmapDataType);

  obj->Render(prop);

  return Qnil;
}

MRI_METHOD(aom_info) {
  scoped_refptr<content::AOMDecoder> obj =
      MriGetStructData<content::AOMDecoder>(self);
  auto& info = *obj->GetVideoInfo();

  VALUE info_hash = rb_hash_new();
  rb_hash_aset(info_hash, rb_str_new2("width"), INT2FIX(info.width));
  rb_hash_aset(info_hash, rb_str_new2("height"), INT2FIX(info.height));
  rb_hash_aset(info_hash, rb_str_new2("duration"), rb_float_new(info.duration));
  rb_hash_aset(info_hash, rb_str_new2("frame_rate"),
               rb_float_new(info.frameRate));

  return info_hash;
}

void InitAOMDecodeBinding() {
  VALUE klass = rb_define_class("AOMDecoder", rb_cObject);
  rb_define_alloc_func(klass, MriClassAllocate<&kAOMDecoderDataType>);

  MriInitDisposableBinding<content::AOMDecoder>(klass);

  MriDefineMethod(klass, "initialize", aom_initialize);
  MriDefineMethod(klass, "load_video", aom_load_video);
  MriDefineMethod(klass, "update", aom_update);
  MriDefineMethod(klass, "set_state", aom_set_state);
  MriDefineMethod(klass, "get_state", aom_get_state);
  MriDefineMethod(klass, "render", aom_render);
  MriDefineMethod(klass, "video_info", aom_info);
}

}  // namespace binding
