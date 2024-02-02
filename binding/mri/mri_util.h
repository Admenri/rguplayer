// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef BINDING_MRI_MRI_UTIL_H_
#define BINDING_MRI_MRI_UTIL_H_

#include "ruby.h"
#include "ruby/encoding.h"

#include "base/exceptions/exception.h"
#include "content/worker/binding_worker.h"

#include <string>

namespace binding {

using MriException = enum {
  ContentError = 0,
  OpenGLError,
  SDLError,

  RGSSReset,

  ErrnoENOENT,

  MriExceptionNum,
};

#define MRI_DECLARE_DATATYPE(Klass) \
  extern const rb_data_type_t k##Klass##DataType;
#define MRI_DEFINE_DATATYPE(Klass, Name, FreeFunc) \
  const rb_data_type_t k##Klass##DataType = {      \
      Name, {0, FreeFunc, 0, 0, 0}, 0, 0, 0}

#define MRI_DEFINE_DATATYPE_PTR(Klass, Name, FreeTy) \
  const rb_data_type_t k##Klass##DataType = {        \
      Name, {0, MriFreeInstance<FreeTy>, 0, 0, 0}, 0, 0, 0}

#define MRI_DEFINE_DATATYPE_REF(Klass, Name, FreeTy) \
  const rb_data_type_t k##Klass##DataType = {        \
      Name, {0, MriFreeInstanceRef<FreeTy>, 0, 0, 0}, 0, 0, 0}

template <typename Ty>
void MriFreeInstance(void* ptr) {
  delete static_cast<Ty*>(ptr);
}

template <typename Ty>
void MriFreeInstanceRef(void* ptr) {
  static_cast<Ty*>(ptr)->Release();
}

template <const rb_data_type_t* DataType>
VALUE MriClassAllocate(VALUE klass) {
  return rb_data_typed_object_wrap(klass, nullptr, DataType);
}

scoped_refptr<content::BindingRunner> MriGetGlobalRunner();

/// <summary>
/// Parse format args into pointer variable
/// </summary>
/// <param name="argc"></param>
/// <param name="argv"></param>
/// <param name="format">o s z f i b n |</param>
/// <param name=""></param>
/// <returns></returns>
int MriParseArgsTo(int argc, VALUE* argv, const char* fmt, ...);

void MriInitException(bool rgss3);
void MriProcessException(const base::Exception& exception);
void MriCheckArgc(int actual, int expected);
VALUE MriGetException(MriException exception);

#define MRI_GUARD_BEGIN try {
#define MRI_GUARD_END                        \
  }                                          \
  catch (const base::Exception& exception) { \
    MriProcessException(exception);          \
  }

#define MRI_GUARD(exp) \
  MRI_GUARD_BEGIN      \
  exp MRI_GUARD_END

template <typename Ty>
void MriSetStructData(VALUE obj, Ty* ptr) {
  RTYPEDDATA_DATA(obj) = ptr;
}

template <typename Ty>
Ty* MriGetStructData(VALUE obj) {
  return static_cast<Ty*>(RTYPEDDATA_DATA(obj));
}

template <typename Ty>
Ty* MriCheckStructData(VALUE obj, const rb_data_type_t& type) {
  return static_cast<Ty*>(Check_TypedStruct(obj, &type));
}

using RubyMethod = VALUE (*)(int argc, VALUE* argv, VALUE self);

inline void MriDefineMethod(VALUE klass, const char* name, RubyMethod func) {
  rb_define_method(klass, name, RUBY_METHOD_FUNC(func), -1);
}

inline void MriDefineClassMethod(VALUE klass,
                                 const char* name,
                                 RubyMethod func) {
  rb_define_singleton_method(klass, name, RUBY_METHOD_FUNC(func), -1);
}

inline void MriDefineModuleFunction(VALUE module,
                                    const char* name,
                                    RubyMethod func) {
  rb_define_module_function(module, name, RUBY_METHOD_FUNC(func), -1);
}

#define MRI_METHOD(name) static VALUE name(int argc, VALUE* argv, VALUE self)

inline VALUE MriStringUTF8(const char* string, long length) {
  return rb_enc_str_new(string, length, rb_utf8_encoding());
}

template <typename Ty>
VALUE MriWrapObject(scoped_refptr<Ty> ptr,
                    const rb_data_type_t& type,
                    VALUE super = rb_cObject) {
  VALUE klass = rb_const_get(super, rb_intern(type.wrap_struct_name));
  VALUE obj = rb_obj_alloc(klass);

  ptr->AddRef();
  MriSetStructData<Ty>(obj, ptr.get());

  return obj;
}

template <typename Ty>
VALUE MriWrapProperty(VALUE self,
                      scoped_refptr<Ty> ptr,
                      const std::string& name,
                      const rb_data_type_t& type,
                      VALUE super = rb_cObject) {
  VALUE obj = MriWrapObject<Ty>(ptr, type, super);

  rb_iv_set(self, name.c_str(), obj);
  return obj;
}

#define MriDefineAttr(klass, rb_name, ktype, ctype)     \
  MriDefineMethod(klass, rb_name, ktype##_get_##ctype); \
  MriDefineMethod(klass, rb_name "=", ktype##_set_##ctype);

#define MriDefineClassAttr(klass, rb_name, ktype, ctype)     \
  MriDefineClassMethod(klass, rb_name, ktype##_get_##ctype); \
  MriDefineClassMethod(klass, rb_name "=", ktype##_set_##ctype);

#define MRI_BOOL_NEW(x) ((x) ? Qtrue : Qfalse)

}  // namespace binding

#endif  // !BINDING_MRI_MRI_UTIL_H_
