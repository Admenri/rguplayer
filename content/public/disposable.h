// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_DISPOSABLE_H_
#define CONTENT_PUBLIC_DISPOSABLE_H_

#include "base/bind/callback_list.h"
#include "base/buildflags/build.h"
#include "base/exceptions/exception.h"
#include "content/public/graphics.h"

namespace content {

class Disposable {
 public:
  Disposable(scoped_refptr<Graphics> screen) : screen_(screen), link_(this) {
    screen_->AddDisposable(this);
  }

  virtual ~Disposable() { screen_->RemoveDisposable(this); }

  Disposable(const Disposable&) = delete;
  Disposable& operator=(const Disposable&) = delete;

  void Dispose() {
    if (is_disposed_)
      return;

    OnObjectDisposed();
    is_disposed_ = true;
    observers_.Notify();
  }

  inline bool IsDisposed() const { return is_disposed_; }

  base::CallbackListSubscription AddDisposeObserver(
      base::OnceClosure observer) {
    return observers_.Add(std::move(observer));
  }

  inline void CheckIsDisposed() const {
    if (is_disposed_) {
      throw base::Exception(base::Exception::ContentError,
                            "Disposed object: %s", DisposedObjectName().data());
    }
  }

 protected:
  virtual void OnObjectDisposed() = 0;
  virtual std::string_view DisposedObjectName() const = 0;

 private:
  friend class Graphics;
  base::OnceClosureList observers_;
  scoped_refptr<Graphics> screen_;
  base::LinkNode<Disposable> link_;

  bool is_disposed_ = false;
};

inline static bool IsObjectValid(Disposable* target) {
  return target && !target->IsDisposed();
}

}  // namespace content

#endif  // !CONTENT_PUBLIC_DISPOSABLE_H_