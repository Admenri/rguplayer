#ifndef MODULES_DISPOSABLE_H_
#define MODULES_DISPOSABLE_H_

#include "base/exceptions/exception.h"

namespace modules {

class Disposable {
 public:
  Disposable() = default;
  virtual ~Disposable() = default;

  Disposable(const Disposable&) = delete;
  Disposable& operator=(const Disposable) = delete;

  void Dispose() {
    if (is_disposed_) return;

    OnObjectDisposed();
    is_disposed_ = true;
  }

  bool IsDisosed() { return is_disposed_; }

  void CheckedForDispose() {
    if (IsDisosed())
      throw base::Exception(base::Exception::RGSSError, "Disposed object.");
  }

 protected:
  virtual void OnObjectDisposed() = 0;

 private:
  bool is_disposed_ = false;
};

}  // namespace modules

#endif  // MODULES_DISPOSABLE_H_