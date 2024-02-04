// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "binding/mri/init_graphics.h"

#include "binding/mri/init_bitmap.h"
#include "content/public/graphics.h"

namespace binding {

MRI_METHOD(graphics_update) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  screen->Update();

  return Qnil;
}

MRI_METHOD(graphics_freeze) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  screen->Freeze();

  return Qnil;
}

MRI_METHOD(graphics_transition) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  int duration = 8;
  std::string filename;
  int vague = 40;
  MriParseArgsTo(argc, argv, "|izi", &duration, &filename, &vague);

  scoped_refptr<content::Bitmap> transmap;
  if (!filename.empty())
    transmap = new content::Bitmap(screen, filename);

  MRI_GUARD(screen->Transition(duration, transmap, vague););

  return Qnil;
}

MRI_METHOD(graphics_wait) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  int v;
  MriParseArgsTo(argc, argv, "i", &v);

  screen->Wait(v);

  return Qnil;
}

MRI_METHOD(graphics_fadein) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  int v;
  MriParseArgsTo(argc, argv, "i", &v);

  screen->FadeIn(v);

  return Qnil;
}

MRI_METHOD(graphics_fadeout) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  int v;
  MriParseArgsTo(argc, argv, "i", &v);

  screen->FadeOut(v);

  return Qnil;
}

MRI_METHOD(graphics_snap_to_bitmap) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  scoped_refptr<content::Bitmap> snap;
  MRI_GUARD(snap = screen->SnapToBitmap(););

  VALUE obj = MriWrapObject(snap, kBitmapDataType);
  bitmap_init_prop(snap, obj);

  return obj;
}

MRI_METHOD(graphics_frame_reset) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  screen->FrameReset();

  return Qnil;
}

MRI_METHOD(graphics_width) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  return rb_fix_new(screen->GetWidth());
}

MRI_METHOD(graphics_height) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  return rb_fix_new(screen->GetHeight());
}

MRI_METHOD(graphics_resize_screen) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  int w, h;
  MriParseArgsTo(argc, argv, "ii", &w, &h);

  screen->ResizeScreen(base::Vec2i(w, h));

  return Qnil;
}

#define GRAPHICS_DEFINE_ATTR(name)            \
  MRI_METHOD(graphics_get_##name) {           \
    scoped_refptr<content::Graphics> screen = \
        MriGetGlobalRunner()->graphics();     \
    return rb_fix_new(screen->Get##name());   \
  }                                           \
  MRI_METHOD(graphics_set_##name) {           \
    scoped_refptr<content::Graphics> screen = \
        MriGetGlobalRunner()->graphics();     \
    int v;                                    \
    MriParseArgsTo(argc, argv, "i", &v);      \
    screen->Set##name(v);                     \
    return Qnil;                              \
  }

GRAPHICS_DEFINE_ATTR(FrameRate);
GRAPHICS_DEFINE_ATTR(FrameCount);
GRAPHICS_DEFINE_ATTR(Brightness);

#define MriDefineModuleAttr(klass, rb_name, ktype, ctype)       \
  MriDefineModuleFunction(klass, rb_name, ktype##_get_##ctype); \
  MriDefineModuleFunction(klass, rb_name "=", ktype##_set_##ctype);

void InitGraphicsBinding() {
  VALUE module = rb_define_module("Graphics");

  MriDefineModuleFunction(module, "update", graphics_update);
  MriDefineModuleFunction(module, "wait", graphics_wait);
  MriDefineModuleFunction(module, "fadeout", graphics_fadeout);
  MriDefineModuleFunction(module, "fadein", graphics_fadein);
  MriDefineModuleFunction(module, "freeze", graphics_freeze);
  MriDefineModuleFunction(module, "transition", graphics_transition);
  MriDefineModuleFunction(module, "snap_to_bitmap", graphics_snap_to_bitmap);
  MriDefineModuleFunction(module, "frame_reset", graphics_frame_reset);
  MriDefineModuleFunction(module, "width", graphics_width);
  MriDefineModuleFunction(module, "height", graphics_height);
  MriDefineModuleFunction(module, "resize_screen", graphics_resize_screen);

  MriDefineModuleAttr(module, "frame_rate", graphics, FrameRate);
  MriDefineModuleAttr(module, "frame_count", graphics, FrameCount);
  MriDefineModuleAttr(module, "brightness", graphics, Brightness);
}

}  // namespace binding
