// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SCRIPT_DISPOSABLE_H_
#define CONTENT_SCRIPT_DISPOSABLE_H_

#include "base/bind/callback_list.h"
#include "base/buildflags/build.h"
#include "base/exceptions/exception.h"

namespace content {

class Disposable {
 public:
  Disposable() = default;
  virtual ~Disposable() = default;

  Disposable(const Disposable&) = delete;
  Disposable& operator=(const Disposable&) = delete;

  void Dispose() {
    if (is_disposed_) return;
    is_disposed_ = true;
    OnObjectDisposed();
    observers_.Notify();
  }

  bool IsDisposed() const { return is_disposed_; }

  base::CallbackListSubscription AddDisposeObserver(
      base::OnceClosure observer) {
    return observers_.Add(std::move(observer));
  }

 protected:
  void CheckIsDisposed() {
    throw base::Exception(base::Exception::RGSSError, "Disposed object: %s",
                          DisposedObjectName().data());
  }

  virtual void OnObjectDisposed() = 0;
  virtual std::string_view DisposedObjectName() = 0;

 private:
  base::OnceClosureList observers_;

  bool is_disposed_ = false;
};

}  // namespace content

#endif  // CONTENT_SCRIPT_DISPOSABLE_H_