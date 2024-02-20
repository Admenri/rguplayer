// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_FILESYSTEM_FILESYSTEM_H_
#define COMPONENTS_FILESYSTEM_FILESYSTEM_H_

#include "base/bind/callback.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"

#include "SDL_rwops.h"

namespace filesystem {

class Filesystem {
 public:
  Filesystem(const std::string& argv0);
  ~Filesystem();

  Filesystem(const Filesystem&) = delete;
  Filesystem& operator=(const Filesystem&) = delete;

  void AddLoadPath(const std::string& path);

  bool Exists(const std::string& filename);

  using OpenCallback =
      base::RepeatingCallback<bool(SDL_RWops*, const std::string&)>;
  void OpenRead(const std::string& file_path, OpenCallback callback);
  void OpenReadRaw(const std::string& filename,
                   SDL_RWops& ops,
                   bool free_on_close);

  base::WeakPtr<Filesystem> AsWeakPtr() {
    return weak_ptr_factory_.GetWeakPtr();
  }

 private:
  base::WeakPtrFactory<Filesystem> weak_ptr_factory_{this};
};

}  // namespace filesystem

#endif  //! COMPONENTS_FILESYSTEM_FILESYSTEM_H_
