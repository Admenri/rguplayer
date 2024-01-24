// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef BINDING_MRI_BINDING_UTIL_H_
#define BINDING_MRI_BINDING_UTIL_H_

#include "ruby.h"

#include "base/memory/ref_counted.h"

namespace binding {

#define DEF_TYPE(Klass, Name, ktype) \
  rb_data_type_t Klass##Type = {     \
      Name, {0, FreeInstance<ktype>, 0, 0, 0}, 0, 0, 0}

#define DECL_TYPE(Klass) extern rb_data_type_t Klass##Type

template <class C>
static void FreeInstance(void* inst) {
  static_cast<C*>(inst)->Release();
}

template <rb_data_type_t* rbType>
static VALUE ClassAllocate(VALUE klass) {
  return rb_data_typed_object_wrap(klass, 0, rbType);
}

template <class C>
inline scoped_refptr<C> GetPrivateData(VALUE self) {
  return static_cast<C*>(RTYPEDDATA_DATA(self));
}

template <class C>
static inline scoped_refptr<C> GetPrivateDataCheck(VALUE self,
                                                   const rb_data_type_t& type) {
  void* obj = Check_TypedStruct(self, &type);
  return static_cast<C*>(obj);
}

template <class C>
static inline void SetPrivateData(VALUE self, scoped_refptr<C> p) {
  p->AddRef();
  RTYPEDDATA_DATA(self) = p.get();
}

template <class C>
inline VALUE WrapObject(scoped_refptr<C> p,
                        const rb_data_type_t& type,
                        VALUE underKlass = rb_cObject) {
  VALUE klass = rb_const_get(underKlass, rb_intern(type.wrap_struct_name));
  VALUE obj = rb_obj_alloc(klass);

  SetPrivateData<C>(obj, p);

  return obj;
}

template <class C>
inline VALUE WrapProperty(VALUE self,
                          scoped_refptr<C> prop,
                          const char* iv,
                          const rb_data_type_t& type,
                          VALUE underKlass = rb_cObject) {
  VALUE propObj = WrapObject<C>(prop, type, underKlass);

  rb_iv_set(self, iv, propObj);

  return propObj;
}

/* Implemented: oSszfibn| */
int GetArgsFrom(int argc, VALUE* argv, const char* format, ...);

inline void CheckArgc(int actual, int expected) {
  if (actual != expected)
    rb_raise(rb_eArgError, "wrong number of arguments (%d for %d)", actual,
             expected);
}

typedef VALUE (*RubyMethod)(int argc, VALUE* argv, VALUE self);

static inline void DefineMethod(VALUE klass,
                                const char* name,
                                RubyMethod func) {
  rb_define_method(klass, name, RUBY_METHOD_FUNC(func), -1);
}

static inline void DefineClassMethod(VALUE klass,
                                     const char* name,
                                     RubyMethod func) {
  rb_define_singleton_method(klass, name, RUBY_METHOD_FUNC(func), -1);
}

static inline void DefineModuleFunction(VALUE module,
                                        const char* name,
                                        RubyMethod func) {
  rb_define_module_function(module, name, RUBY_METHOD_FUNC(func), -1);
}

#define MRI_METHOD(name) static VALUE name(int argc, VALUE* argv, VALUE self)

}  // namespace binding

#endif  // !BINDING_MRI_BINDING_UTIL_H_
