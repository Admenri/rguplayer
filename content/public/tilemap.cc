// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/tilemap.h"

namespace content {

Tilemap::Tilemap(scoped_refptr<Graphics> screen,
                 scoped_refptr<Viewport> viewport,
                 int tilesize)
    : GraphicElement(screen),
      Disposable(screen),
      viewport_(viewport),
      tile_size_(tilesize) {}

Tilemap::~Tilemap() {
  Dispose();
}

void Tilemap::Update() {
  CheckIsDisposed();
}

scoped_refptr<Bitmap> Tilemap::GetTileset() const {
  CheckIsDisposed();
  return tileset_;
}

void Tilemap::SetTileset(scoped_refptr<Bitmap> bitmap) {
  CheckIsDisposed();

  if (tileset_ == bitmap)
    return;

  tileset_ = bitmap;
}

scoped_refptr<Bitmap> Tilemap::GetAutotiles(int index) const {
  CheckIsDisposed();
  return autotiles_[index];
}

void Tilemap::SetAutotiles(int index, scoped_refptr<Bitmap> bitmap) {
  CheckIsDisposed();

  if (autotiles_[index] == bitmap)
    return;

  autotiles_[index] = bitmap;
}

scoped_refptr<Table> Tilemap::GetMapData() const {
  CheckIsDisposed();
  return map_data_;
}

void Tilemap::SetMapData(scoped_refptr<Table> map_data) {
  CheckIsDisposed();

  if (map_data_ == map_data)
    return;

  map_data_ = map_data;
}

scoped_refptr<Table> Tilemap::GetFlashData() const {
  CheckIsDisposed();
  return flash_data_;
}

void Tilemap::SetFlashData(scoped_refptr<Table> flash_data) {
  CheckIsDisposed();

  if (flash_data_ == flash_data)
    return;

  flash_data_ = flash_data;
}

scoped_refptr<Table> Tilemap::GetPriorities() const {
  CheckIsDisposed();
  return priorities_;
}

void Tilemap::SetPriorities(scoped_refptr<Table> flags) {
  CheckIsDisposed();

  if (priorities_ == flags)
    return;

  priorities_ = flags;
}

scoped_refptr<Viewport> Tilemap::GetViewport() const {
  CheckIsDisposed();
  return viewport_;
}

void Tilemap::SetViewport(scoped_refptr<Viewport> viewport) {
  CheckIsDisposed();

  if (viewport_ == viewport)
    return;

  viewport_ = viewport;
}

bool Tilemap::GetVisible() const {
  CheckIsDisposed();
  return visible_;
}

void Tilemap::SetVisible(bool visible) {
  CheckIsDisposed();

  if (visible_ == visible)
    return;

  visible_ = visible;
}

int Tilemap::GetOX() const {
  CheckIsDisposed();
  return origin_.x;
}

void Tilemap::SetOX(int ox) {
  CheckIsDisposed();

  if (origin_.x == ox)
    return;

  origin_.x = ox;
}

int Tilemap::GetOY() const {
  CheckIsDisposed();
  return origin_.y;
}

void Tilemap::SetOY(int oy) {
  CheckIsDisposed();

  if (origin_.y == oy)
    return;

  origin_.y = oy;
}

void Tilemap::OnObjectDisposed() {}

}  // namespace content
