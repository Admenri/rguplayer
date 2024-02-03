// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "binding/mri/init_corefile.h"

#include "components/filesystem/filesystem.h"

namespace binding {

namespace {

struct CoreFileInfo {
  SDL_RWops* ops;
  bool closed;
  bool free;
};

VALUE CreateCoreFileFrom(const std::string& filename, bool mri_exc) {
  CoreFileInfo* info = new CoreFileInfo;

  // TODO: filesystem required
  std::string path(MriGetGlobalRunner()->config()->base_path());
  path += filename;
  info->ops = SDL_RWFromFile(path.c_str(), "r+");
  info->closed = false;
  info->free = false;

  if (!info->ops) {
    delete info;

    if (mri_exc) {
      rb_raise(rb_eRuntimeError, "Failed to load file: %s", filename.c_str());
    } else {
      throw base::Exception::Exception(base::Exception::SDLError,
                                       "Failed to load file: %s",
                                       filename.c_str());
    }
  }

  VALUE klass = rb_const_get(rb_cObject, rb_intern("CoreFile"));
  VALUE obj = rb_obj_alloc(klass);
  MriSetStructData(obj, info);

  return obj;
}

void CoreFileFreeInstance(void* ptr) {
  CoreFileInfo* info = static_cast<CoreFileInfo*>(ptr);

  if (!info->closed) {
    SDL_RWclose(info->ops);
  }

  if (info->free) {
    SDL_DestroyRW(info->ops);
  }

  delete info;
}

VALUE StringForceUTF8(VALUE arg) {
  if (RB_TYPE_P(arg, RUBY_T_STRING) && ENCODING_IS_ASCII8BIT(arg))
    rb_enc_associate_index(arg, rb_utf8_encindex());

  return arg;
}

VALUE Utf8CustomProc(VALUE arg, VALUE proc) {
  VALUE obj = StringForceUTF8(arg);
  obj = rb_funcall2(proc, rb_intern("call"), 1, &obj);

  return obj;
}

}  // namespace

MRI_DEFINE_DATATYPE(CoreFile, "CoreFile", CoreFileFreeInstance);

MRI_METHOD(corefile_read) {
  CoreFileInfo* info = MriGetStructData<CoreFileInfo>(self);

  int length = -1;
  MriParseArgsTo(argc, argv, "i", &length);

  if (length == -1) {
    Sint64 cur = SDL_RWtell(info->ops);
    Sint64 end = SDL_RWseek(info->ops, 0, SEEK_END);
    length = end - cur;
    SDL_RWseek(info->ops, cur, SEEK_SET);
  }

  if (!length)
    return Qnil;

  VALUE data = rb_str_new(0, length);
  SDL_RWread(info->ops, RSTRING_PTR(data), length);

  return data;
}

MRI_METHOD(corefile_getbyte) {
  CoreFileInfo* info = MriGetStructData<CoreFileInfo>(self);

  unsigned char byte;
  size_t result = SDL_RWread(info->ops, &byte, 1);

  return (result == 1) ? rb_fix_new(byte) : Qnil;
}

MRI_METHOD(corefile_binmode) {
  return Qnil;
}

MRI_METHOD(corefile_close) {
  CoreFileInfo* info = MriGetStructData<CoreFileInfo>(self);

  if (!info->closed) {
    SDL_RWclose(info->ops);
    info->closed = true;
  }

  return Qnil;
}

VALUE MriLoadData(const std::string& filename, bool mri_exc) {
  rb_gc_start();

  VALUE port = CreateCoreFileFrom(filename, mri_exc);

  VALUE marshal_klass = rb_const_get(rb_cObject, rb_intern("Marshal"));

  VALUE result = rb_funcall2(marshal_klass, rb_intern("load"), 1, &port);
  rb_funcall2(port, rb_intern("close"), 0, NULL);

  return result;
}

MRI_METHOD(kernel_load_data) {
  std::string filename;
  MriParseArgsTo(argc, argv, "s", &filename);

  return MriLoadData(filename, true);
}

MRI_METHOD(kernel_save_data) {
  VALUE obj;
  VALUE filename;
  MriParseArgsTo(argc, argv, "oz", &obj, &filename);

  VALUE file = rb_file_open_str(filename, "wb");
  VALUE marshal_klass = rb_const_get(rb_cObject, rb_intern("Marshal"));

  VALUE v[] = {obj, file};
  rb_funcall2(marshal_klass, rb_intern("dump"), 2, v);

  rb_io_close(file);

  return Qnil;
}

MRI_METHOD(marshal_load_utf8) {
  VALUE port, proc = Qnil;

  MriParseArgsTo(argc, argv, "o|o", &port, &proc);

  VALUE utf8Proc;
  if (NIL_P(proc))
    utf8Proc = rb_proc_new(RUBY_METHOD_FUNC(StringForceUTF8), Qnil);
  else
    utf8Proc = rb_proc_new(RUBY_METHOD_FUNC(Utf8CustomProc), proc);

  VALUE marsh = rb_const_get(rb_cObject, rb_intern("Marshal"));

  VALUE v[] = {port, utf8Proc};
  return rb_funcall2(marsh, rb_intern("load_utf8_alias_"), 2, v);
}

void InitCoreFileBinding() {
  VALUE klass = rb_define_class("CoreFile", rb_cIO);
  rb_define_alloc_func(klass, MriClassAllocate<&kCoreFileDataType>);

  MriDefineMethod(klass, "read", corefile_read);
  MriDefineMethod(klass, "getbyte", corefile_getbyte);
  MriDefineMethod(klass, "binmode", corefile_binmode);
  MriDefineMethod(klass, "close", corefile_close);

  MriDefineModuleFunction(rb_mKernel, "load_data", kernel_load_data);
  MriDefineModuleFunction(rb_mKernel, "save_data", kernel_save_data);

  VALUE marshal_klass = rb_const_get(rb_cObject, rb_intern("Marshal"));
  rb_define_alias(rb_singleton_class(marshal_klass), "load_utf8_alias_",
                  "load");
  MriDefineModuleFunction(marshal_klass, "load", marshal_load_utf8);
}

}  // namespace binding
