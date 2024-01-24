// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "binding/mri/binding_util.h"

#include <stdarg.h>
#include <string>

namespace binding {

namespace {

void rb_float_arg(VALUE arg, double* out, int argPos = 0) {
  switch (rb_type(arg)) {
    case RUBY_T_FLOAT:
      *out = RFLOAT_VALUE(arg);
      break;

    case RUBY_T_FIXNUM:
      *out = FIX2INT(arg);
      break;

    default:
      rb_raise(rb_eTypeError, "Argument %d: Expected float", argPos);
  }
}

void rb_int_arg(VALUE arg, int* out, int argPos = 0) {
  switch (rb_type(arg)) {
    case RUBY_T_FLOAT:
      *out = NUM2LONG(arg);
      break;

    case RUBY_T_FIXNUM:
      *out = FIX2INT(arg);
      break;

    default:
      rb_raise(rb_eTypeError, "Argument %d: Expected fixnum", argPos);
  }
}

void rb_bool_arg(VALUE arg, bool* out, int argPos = 0) {
  switch (rb_type(arg)) {
    case RUBY_T_TRUE:
      *out = true;
      break;

    case RUBY_T_FALSE:
    case RUBY_T_NIL:
      *out = false;
      break;

    default:
      rb_warning("Warning: Argument %d: Expected bool.", argPos);

      *out = true;
      break;
  }
}

}  // namespace

int GetArgsFrom(int argc, VALUE* argv, const char* format, ...) {
  char c;
  VALUE* arg = argv;
  va_list ap;
  bool opt = false;
  int argI = 0;

  va_start(ap, format);

  while ((c = *format++)) {
    switch (c) {
      case '|':
        break;
      default:
        if (argc <= argI && !opt)
          rb_raise(rb_eArgError, "wrong number of arguments");

        break;
    }

    if (argI >= argc)
      break;

    switch (c) {
      case 'o': {
        if (argI >= argc)
          break;

        VALUE* obj = va_arg(ap, VALUE*);

        *obj = *arg++;
        ++argI;

        break;
      }

      case 'S': {
        if (argI >= argc)
          break;

        VALUE* str = va_arg(ap, VALUE*);
        VALUE tmp = *arg;

        if (!RB_TYPE_P(tmp, RUBY_T_STRING))
          rb_raise(rb_eTypeError, "Argument %d: Expected string", argI);

        *str = tmp;
        ++argI;

        break;
      }

      case 's': {
        if (argI >= argc)
          break;

        const char** s = va_arg(ap, const char**);
        int* len = va_arg(ap, int*);

        VALUE tmp = *arg;

        if (!RB_TYPE_P(tmp, RUBY_T_STRING))
          rb_raise(rb_eTypeError, "Argument %d: Expected string", argI);

        *s = RSTRING_PTR(tmp);
        *len = RSTRING_LEN(tmp);
        ++argI;

        break;
      }

      case 'z': {
        if (argI >= argc)
          break;

        const char** s = va_arg(ap, const char**);

        VALUE tmp = *arg++;

        if (!RB_TYPE_P(tmp, RUBY_T_STRING))
          rb_raise(rb_eTypeError, "Argument %d: Expected string", argI);

        *s = RSTRING_PTR(tmp);
        ++argI;

        break;
      }

      case 'f': {
        if (argI >= argc)
          break;

        double* f = va_arg(ap, double*);
        VALUE fVal = *arg++;

        rb_float_arg(fVal, f, argI);

        ++argI;
        break;
      }

      case 'i': {
        if (argI >= argc)
          break;

        int* i = va_arg(ap, int*);
        VALUE iVal = *arg++;

        rb_int_arg(iVal, i, argI);

        ++argI;
        break;
      }

      case 'b': {
        if (argI >= argc)
          break;

        bool* b = va_arg(ap, bool*);
        VALUE bVal = *arg++;

        rb_bool_arg(bVal, b, argI);

        ++argI;
        break;
      }

      case 'n': {
        if (argI >= argc)
          break;

        ID* sym = va_arg(ap, ID*);

        VALUE symVal = *arg++;

        if (!SYMBOL_P(symVal))
          rb_raise(rb_eTypeError, "Argument %d: Expected symbol", argI);

        *sym = SYM2ID(symVal);
        ++argI;

        break;
      }

      case '|':
        opt = true;
        break;

      default:
        rb_raise(rb_eFatal, "invalid argument specifier %c", c);
    }
  }

  va_end(ap);

  return argI;
}

}  // namespace binding
