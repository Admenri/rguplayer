// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/common/serializable.h"

namespace content {

Serializable::Serializable() {}

Serializable::~Serializable() {}

int32_t Serializable::ReadInt32(const char* data, int offset) {
  int32_t result;
  memcpy(&result, data + offset, sizeof(int32_t));
  return result;
}

float Serializable::ReadFloat(const char* data, int offset) {
  float result;
  memcpy(&result, data + offset, sizeof(float));
  return result;
}

void Serializable::WriteInt32(char* data, int offset, int32_t value) {
  memcpy(data + offset, &value, sizeof(int32_t));
}

void Serializable::WriteFloat(char* data, int offset, float value) {
  memcpy(data + offset, &value, sizeof(float));
}

}  // namespace content
