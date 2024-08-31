// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "binding/mri/init_gamepad.h"

#include "binding/mri/mri_template.h"
#include "content/public/gamepad.h"
#include "content/worker/binding_worker.h"

namespace binding {

MRI_DEFINE_DATATYPE_REF(Gamepad, "Gamepad", content::Gamepad);

MRI_METHOD(gamepad_get_avai_devices) {
  std::vector<content::Gamepad::GamepadDevice> devices;
  content::Gamepad::GetAvailableDevices(devices);

  VALUE ary = rb_ary_new();
  for (auto& it : devices) {
    VALUE info = rb_hash_new();
    rb_hash_aset(info, rb_str_new2("id"), rb_fix_new(it.id));
    rb_hash_aset(info, rb_str_new2("name"),
                 rb_str_new2(it.device_name.c_str()));
    rb_hash_aset(info, rb_str_new2("path"),
                 rb_str_new2(it.device_path.c_str()));
    rb_hash_aset(info, rb_str_new2("player"), rb_fix_new(it.player_index));
    rb_hash_aset(info, rb_str_new2("guid"), rb_str_new2(it.guid.c_str()));
    rb_ary_push(ary, info);
  }

  return ary;
}

MRI_METHOD(gamepad_initialize) {
  SDL_JoystickID devid;
  MriParseArgsTo(argc, argv, "i", &devid);

  scoped_refptr<content::Gamepad> pad;
  MRI_GUARD(pad = new content::Gamepad(devid););

  pad->AddRef();
  MriSetStructData(self, pad.get());

  return self;
}

MRI_METHOD(gamepad_update) {
  scoped_refptr<content::Gamepad> obj =
      MriGetStructData<content::Gamepad>(self);

  obj->Update();

  return Qnil;
}

MRI_METHOD(gamepad_connect) {
  scoped_refptr<content::Gamepad> obj =
      MriGetStructData<content::Gamepad>(self);

  return MRI_BOOL_NEW(obj->Connect());
}

MRI_METHOD(gamepad_id) {
  scoped_refptr<content::Gamepad> obj =
      MriGetStructData<content::Gamepad>(self);

  return rb_fix_new(obj->GetInstanceID());
}

MRI_METHOD(gamepad_connect_state) {
  scoped_refptr<content::Gamepad> obj =
      MriGetStructData<content::Gamepad>(self);

  return rb_fix_new(obj->GetConnectState());
}

MRI_METHOD(gamepad_axis) {
  scoped_refptr<content::Gamepad> obj =
      MriGetStructData<content::Gamepad>(self);

  SDL_GamepadAxis axis_id;
  MriParseArgsTo(argc, argv, "i", &axis_id);

  return rb_fix_new(obj->GetAxisValue(axis_id));
}

MRI_METHOD(gamepad_pressed) {
  scoped_refptr<content::Gamepad> obj =
      MriGetStructData<content::Gamepad>(self);

  SDL_GamepadButton bid;
  MriParseArgsTo(argc, argv, "i", &bid);

  return MRI_BOOL_NEW(obj->IsButtonPressed(bid));
}

MRI_METHOD(gamepad_triggered) {
  scoped_refptr<content::Gamepad> obj =
      MriGetStructData<content::Gamepad>(self);

  SDL_GamepadButton bid;
  MriParseArgsTo(argc, argv, "i", &bid);

  return MRI_BOOL_NEW(obj->IsButtonTriggered(bid));
}

MRI_METHOD(gamepad_repeated) {
  scoped_refptr<content::Gamepad> obj =
      MriGetStructData<content::Gamepad>(self);

  SDL_GamepadButton bid;
  MriParseArgsTo(argc, argv, "i", &bid);

  return MRI_BOOL_NEW(obj->IsButtonRepeated(bid));
}

MRI_METHOD(gamepad_press_buttons) {
  scoped_refptr<content::Gamepad> obj =
      MriGetStructData<content::Gamepad>(self);

  VALUE ary = rb_ary_new();

  std::vector<SDL_GamepadButton> buttons;
  obj->GetPressedButtons(buttons);

  for (const auto& it : buttons)
    rb_ary_push(ary, rb_fix_new(it));

  return ary;
}

MRI_METHOD(gamepad_get_PlayerIndex) {
  scoped_refptr<content::Gamepad> obj =
      MriGetStructData<content::Gamepad>(self);

  return rb_fix_new(obj->GetPlayerIndex());
}

MRI_METHOD(gamepad_set_PlayerIndex) {
  int v;
  MriParseArgsTo(argc, argv, "i", &v);

  scoped_refptr<content::Gamepad> obj =
      MriGetStructData<content::Gamepad>(self);

  obj->SetPlayerIndex(v);

  return self;
}

void InitGamepadBinding() {
  VALUE klass = rb_define_class("Gamepad", rb_cObject);

  MriDefineClassMethod(klass, "available_devices", gamepad_get_avai_devices);

  MriDefineMethod(klass, "initialize", gamepad_initialize);
  MriDefineMethod(klass, "update", gamepad_update);
  MriDefineMethod(klass, "device_id", gamepad_id);
  MriDefineMethod(klass, "connect", gamepad_connect);
  MriDefineMethod(klass, "connect_state", gamepad_connect_state);

  MriDefineMethod(klass, "get_axis_value", gamepad_axis);
  MriDefineMethod(klass, "button_press?", gamepad_pressed);
  MriDefineMethod(klass, "button_trigger?", gamepad_triggered);
  MriDefineMethod(klass, "button_repeat?", gamepad_repeated);

  MriDefineMethod(klass, "pressed_buttons", gamepad_press_buttons);

  MriDefineAttr(klass, "player_index", gamepad, PlayerIndex);

#define GAMEPAD_CONST(x) \
  rb_const_set(klass, rb_intern(#x), INT2FIX(SDL_GAMEPAD_##x));

  GAMEPAD_CONST(AXIS_LEFTX);
  GAMEPAD_CONST(AXIS_LEFTY);
  GAMEPAD_CONST(AXIS_RIGHTX);
  GAMEPAD_CONST(AXIS_RIGHTY);
  GAMEPAD_CONST(AXIS_LEFT_TRIGGER);
  GAMEPAD_CONST(AXIS_RIGHT_TRIGGER);

  GAMEPAD_CONST(BUTTON_SOUTH);
  GAMEPAD_CONST(BUTTON_EAST);
  GAMEPAD_CONST(BUTTON_WEST);
  GAMEPAD_CONST(BUTTON_NORTH);
  GAMEPAD_CONST(BUTTON_BACK);
  GAMEPAD_CONST(BUTTON_GUIDE);
  GAMEPAD_CONST(BUTTON_START);
  GAMEPAD_CONST(BUTTON_LEFT_STICK);
  GAMEPAD_CONST(BUTTON_RIGHT_STICK);
  GAMEPAD_CONST(BUTTON_LEFT_SHOULDER);
  GAMEPAD_CONST(BUTTON_RIGHT_SHOULDER);
  GAMEPAD_CONST(BUTTON_DPAD_UP);
  GAMEPAD_CONST(BUTTON_DPAD_DOWN);
  GAMEPAD_CONST(BUTTON_DPAD_LEFT);
  GAMEPAD_CONST(BUTTON_DPAD_RIGHT);
  GAMEPAD_CONST(BUTTON_MISC1);
  GAMEPAD_CONST(BUTTON_MISC2);
  GAMEPAD_CONST(BUTTON_MISC3);
  GAMEPAD_CONST(BUTTON_MISC4);
  GAMEPAD_CONST(BUTTON_MISC5);
  GAMEPAD_CONST(BUTTON_MISC6);
  GAMEPAD_CONST(BUTTON_RIGHT_PADDLE1);
  GAMEPAD_CONST(BUTTON_RIGHT_PADDLE2);
  GAMEPAD_CONST(BUTTON_LEFT_PADDLE1);
  GAMEPAD_CONST(BUTTON_LEFT_PADDLE2);
  GAMEPAD_CONST(BUTTON_TOUCHPAD);
}

}  // namespace binding
