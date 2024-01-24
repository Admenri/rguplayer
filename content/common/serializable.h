// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_COMMON_SERIALIZABLE_H_
#define CONTENT_COMMON_SERIALIZABLE_H_

#include <stdint.h>
#include <memory>
#include <vector>

#include "base/memory/ref_counted.h"

namespace content {

class Serializable {
 public:
  using ByteElement = uint8_t;
  using ByteType = std::vector<ByteElement>;

  Serializable();
  virtual ~Serializable();

  Serializable(const Serializable&) = delete;
  Serializable& operator=(const Serializable&) = delete;

  virtual std::unique_ptr<ByteType> Serialize() = 0;

  template <class T>
  static scoped_refptr<T> Deserialize(std::unique_ptr<ByteType> data) {
    return T::Deserialize(data);
  }

  static ByteElement* RawData(ByteType* data);

  static int32_t ReadInt32(ByteType* data, int offset);
  static double ReadDouble(ByteType* data, int offset);
  static void WriteInt32(ByteType* data, int offset, int32_t value);
  static void WriteDouble(ByteType* data, int offset, double value);
};

}  // namespace content

#endif  // !CONTENT_COMMON_SERIALIZABLE_H_
