// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_TILEMAP_H_
#define CONTENT_PUBLIC_TILEMAP_H_

#include "base/memory/ref_counted.h"
#include "content/public/bitmap.h"
#include "content/public/disposable.h"
#include "content/public/table.h"
#include "content/public/viewport.h"

#include <array>

namespace content {

class Tilemap : public base::RefCounted<Tilemap>,
                public GraphicElement,
                public Disposable {
 public:
  Tilemap(scoped_refptr<Graphics> screen,
          scoped_refptr<Viewport> viewport = nullptr,
          int tilesize = 32);
  ~Tilemap();

  Tilemap(const Tilemap&) = delete;
  Tilemap& operator=(const Tilemap&) = delete;

  void Update();

  scoped_refptr<Bitmap> GetTileset() const;
  void SetTileset(scoped_refptr<Bitmap> bitmap);

  scoped_refptr<Bitmap> GetAutotiles(int index) const;
  void SetAutotiles(int index, scoped_refptr<Bitmap> bitmap);

  scoped_refptr<Table> GetMapData() const;
  void SetMapData(scoped_refptr<Table> map_data);

  scoped_refptr<Table> GetFlashData() const;
  void SetFlashData(scoped_refptr<Table> flash_data);

  scoped_refptr<Table> GetPriorities() const;
  void SetPriorities(scoped_refptr<Table> flags);

  scoped_refptr<Viewport> GetViewport() const;
  void SetViewport(scoped_refptr<Viewport> viewport);

  bool GetVisible() const;
  void SetVisible(bool visible);

  int GetOX() const;
  void SetOX(int ox);

  int GetOY() const;
  void SetOY(int oy);

 private:
  void OnObjectDisposed() override;
  std::string_view DisposedObjectName() const override { return "Tilemap"; }

  scoped_refptr<Bitmap> tileset_;
  std::array<scoped_refptr<Bitmap>, 7> autotiles_;

  scoped_refptr<Table> map_data_;
  scoped_refptr<Table> flash_data_;
  scoped_refptr<Table> priorities_;

  scoped_refptr<Viewport> viewport_;
  bool visible_ = true;
  base::Vec2i origin_;
  int tile_size_;
};

}  // namespace content

#endif  //! CONTENT_PUBLIC_TILEMAP_H_
