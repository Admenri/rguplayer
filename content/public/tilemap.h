// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_TILEMAP_H_
#define CONTENT_PUBLIC_TILEMAP_H_

#include <array>

#include "base/memory/ref_counted.h"
#include "content/public/bitmap.h"
#include "content/public/disposable.h"
#include "content/public/table.h"
#include "content/public/viewport.h"
#include "renderer/quad/quad_array.h"

namespace content {

class GroundLayer;
class AboveLayer;

class Tilemap2 : public base::RefCounted<Tilemap2>,
                 public GraphicElement,
                 public Disposable {
 public:
  Tilemap2(scoped_refptr<Graphics> screen,
           scoped_refptr<Viewport> viewport = nullptr);
  ~Tilemap2() override;

  Tilemap2(const Tilemap2&) = delete;
  Tilemap2& operator=(const Tilemap2&) = delete;

  void Update();

  scoped_refptr<Bitmap> GetBitmap(int index) const;
  void SetBitmap(int index, scoped_refptr<Bitmap> bitmap);

  scoped_refptr<Table> GetMapData() const;
  void SetMapData(scoped_refptr<Table> map_data);

  scoped_refptr<Table> GetFlashData() const;
  void SetFlashData(scoped_refptr<Table> flash_data);

  scoped_refptr<Table> GetFlags() const;
  void SetFlags(scoped_refptr<Table> flags);

  scoped_refptr<Viewport> GetViewport() const;
  void SetViewport(scoped_refptr<Viewport> viewport);

  bool GetVisible() const;
  void SetVisible(bool visible);

  int GetOX() const;
  void SetOX(int ox);

  int GetOY() const;
  void SetOY(int oy);

 private:
  friend class GroundLayer;
  friend class AboveLayer;

  void OnObjectDisposed() override;
  std::string_view DisposedObjectName() const override { return "Tilemap2"; }

  void InitTilemapInternal();

  std::unique_ptr<GroundLayer> ground_;
  std::unique_ptr<AboveLayer> above_;

  scoped_refptr<Viewport> viewport_;
  bool visible_ = true;
  base::Vec2i origin_;

  std::array<scoped_refptr<Bitmap>, 9> bitmaps_;

  scoped_refptr<Table> map_data_;
  scoped_refptr<Table> flash_data_;
  scoped_refptr<Table> flags_;

  std::vector<renderer::CommonVertex> tile_vertices_;
  renderer::VertexArray<renderer::CommonVertex> vao_;

  base::WeakPtrFactory<Tilemap2> weak_ptr_factory_{this};
};

}  // namespace content

#endif  // !CONTENT_PUBLIC_TILEMAP_H_