// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "binding/mri/init_shader.h"

#include "binding/mri/init_bitmap.h"
#include "binding/mri/mri_template.h"
#include "content/public/shader.h"

namespace binding {

MRI_DEFINE_DATATYPE_REF(Shader, "Shader", content::Shader);

MRI_METHOD(shader_initialize) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  scoped_refptr<content::Shader> shader;
  MRI_GUARD(shader = new content::Shader(screen););

  shader->AddRef();
  MriSetStructData(self, shader.get());

  VALUE tex_pool = rb_hash_new();
  rb_iv_set(self, "_textures", tex_pool);

  return self;
}

MRI_METHOD(shader_compile) {
  scoped_refptr<content::Shader> obj = MriGetStructData<content::Shader>(self);

  VALUE tex_pool = rb_iv_get(self, "_textures");
  rb_hash_clear(tex_pool);

  std::string vert, frag;
  MriParseArgsTo(argc, argv, "ss", &vert, &frag);
  MRI_GUARD(obj->Compile(vert, frag););

  return self;
}

MRI_METHOD(shader_reset) {
  scoped_refptr<content::Shader> obj = MriGetStructData<content::Shader>(self);

  VALUE tex_pool = rb_iv_get(self, "_textures");
  rb_hash_clear(tex_pool);

  MRI_GUARD(obj->Reset(););

  return self;
}

MRI_METHOD(shader_set_blend) {
  scoped_refptr<content::Shader> obj = MriGetStructData<content::Shader>(self);

  int mode, sRGB, dRGB, sAlpha, dAlpha;
  MriParseArgsTo(argc, argv, "iiiii", &mode, &sRGB, &dRGB, &sAlpha, &dAlpha);
  MRI_GUARD(obj->SetBlend(mode, sRGB, dRGB, sAlpha, dAlpha););

  return self;
}

MRI_METHOD(shader_set_param) {
  scoped_refptr<content::Shader> obj = MriGetStructData<content::Shader>(self);

  MRI_GUARD_BEGIN;

  std::string uniform;
  if (argc == 3) {
    if (rb_type(argv[1]) == RUBY_T_ARRAY) {
      // Vec param
      VALUE data;
      int element;
      MriParseArgsTo(argc, argv, "soi", &uniform, &data, &element);

      std::vector<float> array_data;
      for (int i = 0; i < RARRAY_LEN(data); ++i)
        array_data.push_back(RFLOAT_VALUE(rb_to_float(rb_ary_entry(data, i))));

      obj->SetParam(uniform, array_data, element);
    } else {
      // Bitmap param
      VALUE bitmap;
      int unit;
      MriParseArgsTo(argc, argv, "soi", &uniform, &bitmap, &unit);
      scoped_refptr<content::Bitmap> texture;
      if (!NIL_P(bitmap))
        texture = MriCheckStructData<content::Bitmap>(bitmap, kBitmapDataType);
      obj->SetParam(uniform, texture, unit);

      // Add ruby ref
      VALUE tex_pool = rb_iv_get(self, "_textures");
      rb_hash_aset(tex_pool, INT2FIX(unit), bitmap);
    }
  } else {
    // Matrix param
    VALUE matrix;
    int element;
    bool transpose;
    MriParseArgsTo(argc, argv, "soib", &uniform, &matrix, &element, &transpose);

    if (rb_type(matrix) != RUBY_T_ARRAY)
      rb_raise(rb_eArgError, "invalid type for matrix param. (expect Array)");

    std::vector<float> matrix_data;
    for (int i = 0; i < RARRAY_LEN(matrix); ++i)
      matrix_data.push_back(RFLOAT_VALUE(rb_to_float(rb_ary_entry(matrix, i))));

    obj->SetParam(uniform, matrix_data, element, transpose);
  }

  MRI_GUARD_END;

  return self;
}

void InitShaderBinding() {
  VALUE klass = rb_define_class("Shader", rb_cObject);
  rb_define_alloc_func(klass, MriClassAllocate<&kShaderDataType>);

  MriInitDisposableBinding<content::Shader>(klass);

  MriDefineMethod(klass, "initialize", shader_initialize);
  MriDefineMethod(klass, "compile", shader_compile);
  MriDefineMethod(klass, "reset", shader_reset);
  MriDefineMethod(klass, "set_blend", shader_set_blend);
  MriDefineMethod(klass, "set_param", shader_set_param);

  rb_const_set(klass, rb_intern("FUNC_ADD"), INT2FIX(GL_FUNC_ADD));
  rb_const_set(klass, rb_intern("FUNC_SUBTRACT"), INT2FIX(GL_FUNC_SUBTRACT));
  rb_const_set(klass, rb_intern("FUNC_REVERSE_SUBTRACT"),
               INT2FIX(GL_FUNC_REVERSE_SUBTRACT));
  rb_const_set(klass, rb_intern("MIN"), INT2FIX(GL_MIN));
  rb_const_set(klass, rb_intern("MAX"), INT2FIX(GL_MAX));

  rb_const_set(klass, rb_intern("ZERO"), INT2FIX(GL_ZERO));
  rb_const_set(klass, rb_intern("ONE"), INT2FIX(GL_ONE));
  rb_const_set(klass, rb_intern("SRC_COLOR"), INT2FIX(GL_SRC_COLOR));
  rb_const_set(klass, rb_intern("ONE_MINUS_SRC_COLOR"),
               INT2FIX(GL_ONE_MINUS_SRC_COLOR));
  rb_const_set(klass, rb_intern("SRC_ALPHA"), INT2FIX(GL_SRC_ALPHA));
  rb_const_set(klass, rb_intern("ONE_MINUS_SRC_ALPHA"),
               INT2FIX(GL_ONE_MINUS_SRC_ALPHA));
  rb_const_set(klass, rb_intern("DST_ALPHA"), INT2FIX(GL_DST_ALPHA));
  rb_const_set(klass, rb_intern("ONE_MINUS_DST_ALPHA"),
               INT2FIX(GL_ONE_MINUS_DST_ALPHA));
  rb_const_set(klass, rb_intern("DST_COLOR"), INT2FIX(GL_DST_COLOR));
  rb_const_set(klass, rb_intern("ONE_MINUS_DST_COLOR"),
               INT2FIX(GL_ONE_MINUS_DST_COLOR));
}

}  // namespace binding
