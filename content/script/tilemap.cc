// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/script/tilemap.h"

namespace content {

// Reference: https://www.tktkgame.com/tkool/memo/vx/tile_id.html
class GroundLayer : public ViewportChild {
 public:
  GroundLayer(base::WeakPtr<Tilemap2> tilemap)
      : ViewportChild(tilemap->viewport_), tilemap_(tilemap) {}
  ~GroundLayer() override {}

  GroundLayer(const GroundLayer&) = delete;
  GroundLayer& operator=(const GroundLayer&) = delete;

  void BeforeComposite() override {}

  void Composite() override {}

  void CheckDisposed() const override { tilemap_->CheckIsDisposed(); }

  void OnViewportRectChanged(
      const DrawableParent::ViewportInfo& rect) override {}

  base::WeakPtr<Tilemap2> tilemap_;
};

class AboveLayer : public ViewportChild {
 public:
  AboveLayer(base::WeakPtr<Tilemap2> tilemap)
      : ViewportChild(tilemap->viewport_, 200), tilemap_(tilemap) {}
  ~AboveLayer() override {}

  AboveLayer(const AboveLayer&) = delete;
  AboveLayer& operator=(const AboveLayer&) = delete;

  void BeforeComposite() override {}

  void Composite() override {}

  void CheckDisposed() const override { tilemap_->CheckIsDisposed(); }

  void OnViewportRectChanged(
      const DrawableParent::ViewportInfo& rect) override {}

  base::WeakPtr<Tilemap2> tilemap_;
};

Tilemap2::Tilemap2(scoped_refptr<Viewport> viewport) : viewport_(viewport) {
  ground_ = std::make_unique<GroundLayer>(weak_ptr_factory_.GetWeakPtr());
  above_ = std::make_unique<AboveLayer>(weak_ptr_factory_.GetWeakPtr());
}

Tilemap2::~Tilemap2() { Dispose(); }

void Tilemap2::Update() {}

scoped_refptr<Bitmap> Tilemap2::GetBitmap(int index) const {
  CheckIsDisposed();
  return bitmaps_[index];
}

void Tilemap2::SetBitmap(int index, scoped_refptr<Bitmap> bitmap) {
  CheckIsDisposed();

  if (bitmaps_[index] == bitmap) return;
  bitmaps_[index] = bitmap;
}

scoped_refptr<Table> Tilemap2::GetMapData() const {
  CheckIsDisposed();
  return map_data_;
}

void Tilemap2::SetMapData(scoped_refptr<Table> map_data) {
  CheckIsDisposed();

  if (map_data_ == map_data) return;
  map_data_ = map_data;
}

scoped_refptr<Table> Tilemap2::GetFlashData() const {
  CheckIsDisposed();
  return flash_data_;
}

void Tilemap2::SetFlashData(scoped_refptr<Table> flash_data) {
  CheckIsDisposed();

  if (flash_data_ == flash_data) return;
  flash_data_ = flash_data;
}

scoped_refptr<Table> Tilemap2::GetFlags() const {
  CheckIsDisposed();
  return flags_;
}

void Tilemap2::SetFlags(scoped_refptr<Table> flags) {
  CheckIsDisposed();

  if (flags_ == flags) return;
  flags_ = flags;
}

scoped_refptr<Viewport> Tilemap2::GetViewport() const {
  CheckIsDisposed();
  return viewport_;
}

void Tilemap2::SetViewport(scoped_refptr<Viewport> viewport) {
  CheckIsDisposed();

  if (viewport_ == viewport) return;
  ground_->SetViewport(viewport);
  above_->SetViewport(viewport);
}

bool Tilemap2::GetVisible() const {
  CheckIsDisposed();
  return visible_;
}

void Tilemap2::SetVisible(bool visible) {
  CheckIsDisposed();

  ground_->SetVisible(visible);
  above_->SetVisible(visible);
}

int Tilemap2::GetOX() const {
  CheckIsDisposed();
  return origin_.x;
}

void Tilemap2::SetOX(int ox) {
  CheckIsDisposed();

  if (origin_.x == ox) return;
  origin_.x = ox;
}

int Tilemap2::GetOY() const {
  CheckIsDisposed();
  return origin_.y;
}

void Tilemap2::SetOY(int oy) {
  CheckIsDisposed();

  if (origin_.y == oy) return;
  origin_.y = oy;
}

void Tilemap2::OnObjectDisposed() {
  ground_.reset();
  above_.reset();
}

}  // namespace content