// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "binding/mri/mri_util.h"

#include "content/worker/binding_worker.h"

#include <stdarg.h>

namespace binding {

namespace {

VALUE g_exception_list[MriExceptionNum];

}  // namespace

extern scoped_refptr<content::BindingRunner> g_mri_manager;

scoped_refptr<content::BindingRunner> MriGetGlobalRunner() {
  return g_mri_manager;
}

int MriParseArgsTo(int argc, VALUE* argv, const char* fmt, ...) {
  va_list args_iter;
  bool is_arg_optional = false;
  int count = 0;
  std::string format(fmt);
  auto ch = format.begin();

  va_start(args_iter, fmt);

  while (ch != format.end()) {
    if (*ch != '|' && argc <= count)
      if (!is_arg_optional)
        rb_raise(rb_eArgError, "wrong number of arguments");
      else
        break;

    VALUE arg_element = argv[count];

    switch (*ch) {
      case 'o': {
        VALUE* ptr = va_arg(args_iter, VALUE*);
        *ptr = arg_element;
      }
        ++count;
        break;
      case 'i': {
        int* ptr = va_arg(args_iter, int*);
        switch (rb_type(arg_element)) {
          case RUBY_T_FLOAT:
            *ptr = NUM2LONG(arg_element);
            break;
          case RUBY_T_FIXNUM:
            *ptr = FIX2INT(arg_element);
            break;
          default:
            rb_raise(rb_eTypeError, "Argument %d: Expected fixnum", count);
        }
      }
        ++count;
        break;
      case 's': {
        if (!RB_TYPE_P(arg_element, RUBY_T_STRING))
          rb_raise(rb_eTypeError, "Argument %d: Expected string", count);

        std::string* ptr = va_arg(args_iter, std::string*);
        *ptr = std::string(RSTRING_PTR(arg_element), RSTRING_LEN(arg_element));
      }
        ++count;
        break;
      case 'z': {
        if (!RB_TYPE_P(arg_element, RUBY_T_STRING))
          rb_raise(rb_eTypeError, "Argument %d: Expected string", count);

        VALUE* ptr = va_arg(args_iter, VALUE*);
        *ptr = arg_element;
      }
        ++count;
        break;
      case 'f': {
        double* ptr = va_arg(args_iter, double*);

        switch (rb_type(arg_element)) {
          case RUBY_T_FLOAT:
            *ptr = RFLOAT_VALUE(arg_element);
            break;
          case RUBY_T_FIXNUM:
            *ptr = FIX2INT(arg_element);
            break;
          default:
            rb_raise(rb_eTypeError, "Argument %d: Expected float", count);
        }
      }
        ++count;
        break;
      case 'b': {
        bool* ptr = va_arg(args_iter, bool*);
        switch (rb_type(arg_element)) {
          case RUBY_T_TRUE:
            *ptr = true;
            break;
          case RUBY_T_FALSE:
          case RUBY_T_NIL:
            *ptr = false;
            break;
          default:
            rb_raise(rb_eTypeError, "Argument %d: Expected bool", count);
        }
      }
        ++count;
        break;
      case 'n': {
        std::string* ptr = va_arg(args_iter, std::string*);
        switch (rb_type(arg_element)) {
          case RUBY_T_SYMBOL:
            *ptr = std::string(rb_id2name(SYM2ID(arg_element)));
            break;
          case RUBY_T_STRING:
            *ptr =
                std::string(RSTRING_PTR(arg_element), RSTRING_LEN(arg_element));
            break;
          default:
            rb_raise(rb_eTypeError, "Argument %d: Expected symbol", count);
        }
      }
        ++count;
        break;
      case '|':
        is_arg_optional = true;
        break;
      default:
        rb_raise(rb_eFatal, "Invalid argument specifier %c", *ch);
    }

    ch++;
  }

  va_end(args_iter);

  /* Real args caller provide */
  return count;
}

void MriInitException(bool rgss3) {
  g_exception_list[ContentError] =
      rb_define_class("RGSSError", rb_eStandardError);
  g_exception_list[OpenGLError] =
      rb_define_class("OpenGLError", rb_eStandardError);
  g_exception_list[SDLError] = rb_define_class("SDLError", rb_eStandardError);

  g_exception_list[RGSSReset] =
      rb_define_class(rgss3 ? "RGSSReset" : "Reset", rb_eException);

  g_exception_list[ErrnoENOENT] = rb_const_get(
      rb_const_get(rb_cObject, rb_intern("Errno")), rb_intern("ENOENT"));
}

void MriProcessException(const base::Exception& exception) {
  VALUE rb_eCustom = g_exception_list[exception.GetType()];
  rb_raise(rb_eCustom, exception.GetErrorMessage().c_str());
}

void MriCheckArgc(int actual, int expected) {
  if (actual != expected)
    rb_raise(rb_eArgError, "wrong number of arguments (%d for %d)", actual,
             expected);
}

VALUE MriGetException(MriException exception) {
  return g_exception_list[exception];
}

}  // namespace binding
