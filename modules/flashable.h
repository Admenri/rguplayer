// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MODULES_FLASHABLE_H_
#define MODULES_FLASHABLE_H_

#include "modules/utility.h"

namespace modules {

class Flashable {
 public:
  Flashable() = default;
  virtual ~Flashable() = default;

  Flashable(const Flashable &) = delete;
  Flashable &operator=(const Flashable &) = delete;

  void Flash(Color *color, int duration) {
    if (duration_ <= 0) return;

    flashing_ = true;
    duration_ = duration;
    counter_ = 0;

    if (!color) {
      flash_empty_ = true;
      return;
    }

    flash_color_ = color->ToFloatColor();
    alpha_ = flash_color_.w;
  }

  virtual void Update() {
    if (!flashing_) return;

    if (++counter_ > duration_) {
      flash_color_ = base::Vec4();
      flashing_ = false;
      flash_empty_ = false;
      return;
    }

    if (!flash_empty_) {
      float progress = (float)counter_ / duration_;
      flash_color_.w = alpha_ * (1 - progress);
    }
  }

 protected:
  base::Vec4 flash_color_;
  bool flashing_ = false;
  bool flash_empty_ = false;

 private:
  float alpha_ = 0.f;
  int duration_ = 0;
  int counter_ = 0;
};

}  // namespace modules

#endif  // MODULES_FLASHABLE_H_