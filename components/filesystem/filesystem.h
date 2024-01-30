// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_FILESYSTEM_FILESYSTEM_H_
#define COMPONENTS_FILESYSTEM_FILESYSTEM_H_

#include "base/memory/ref_counted.h"

#include "SDL_rwops.h"

namespace filesystem {

class Filesystem : public base::RefCounted<Filesystem> {
 public:
  Filesystem();
  ~Filesystem();

  Filesystem(const Filesystem&) = delete;
  Filesystem& operator=(const Filesystem&) = delete;

 private:
};

}  // namespace filesystem

#endif  //! COMPONENTS_FILESYSTEM_FILESYSTEM_H_
