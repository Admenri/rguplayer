// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/tilemap.h"

#include "content/common/tilemap_common.h"

#include <algorithm>

namespace content {

namespace {

using AutotileSubPos = enum {
  TopLeft = 0,
  TopRight,
  BottomLeft,
  BottomRight,
};

const base::RectF kAutotileSrcRects[] = {
    {1, 2, 0.5, 0.5},     {1.5, 2, 0.5, 0.5},   {1, 2.5, 0.5, 0.5},
    {1.5, 2.5, 0.5, 0.5}, {2, 0, 0.5, 0.5},     {1.5, 2, 0.5, 0.5},
    {1, 2.5, 0.5, 0.5},   {1.5, 2.5, 0.5, 0.5}, {1, 2, 0.5, 0.5},
    {2.5, 0, 0.5, 0.5},   {1, 2.5, 0.5, 0.5},   {1.5, 2.5, 0.5, 0.5},
    {2, 0, 0.5, 0.5},     {2.5, 0, 0.5, 0.5},   {1, 2.5, 0.5, 0.5},
    {1.5, 2.5, 0.5, 0.5}, {1, 2, 0.5, 0.5},     {1.5, 2, 0.5, 0.5},
    {1, 2.5, 0.5, 0.5},   {2.5, 0.5, 0.5, 0.5}, {2, 0, 0.5, 0.5},
    {1.5, 2, 0.5, 0.5},   {1, 2.5, 0.5, 0.5},   {2.5, 0.5, 0.5, 0.5},
    {1, 2, 0.5, 0.5},     {2.5, 0, 0.5, 0.5},   {1, 2.5, 0.5, 0.5},
    {2.5, 0.5, 0.5, 0.5}, {2, 0, 0.5, 0.5},     {2.5, 0, 0.5, 0.5},
    {1, 2.5, 0.5, 0.5},   {2.5, 0.5, 0.5, 0.5}, {1, 2, 0.5, 0.5},
    {1.5, 2, 0.5, 0.5},   {2, 0.5, 0.5, 0.5},   {1.5, 2.5, 0.5, 0.5},
    {2, 0, 0.5, 0.5},     {1.5, 2, 0.5, 0.5},   {2, 0.5, 0.5, 0.5},
    {1.5, 2.5, 0.5, 0.5}, {1, 2, 0.5, 0.5},     {2.5, 0, 0.5, 0.5},
    {2, 0.5, 0.5, 0.5},   {1.5, 2.5, 0.5, 0.5}, {2, 0, 0.5, 0.5},
    {2.5, 0, 0.5, 0.5},   {2, 0.5, 0.5, 0.5},   {1.5, 2.5, 0.5, 0.5},
    {1, 2, 0.5, 0.5},     {1.5, 2, 0.5, 0.5},   {2, 0.5, 0.5, 0.5},
    {2.5, 0.5, 0.5, 0.5}, {2, 0, 0.5, 0.5},     {1.5, 2, 0.5, 0.5},
    {2, 0.5, 0.5, 0.5},   {2.5, 0.5, 0.5, 0.5}, {1, 2, 0.5, 0.5},
    {2.5, 0, 0.5, 0.5},   {2, 0.5, 0.5, 0.5},   {2.5, 0.5, 0.5, 0.5},
    {2, 0, 0.5, 0.5},     {2.5, 0, 0.5, 0.5},   {2, 0.5, 0.5, 0.5},
    {2.5, 0.5, 0.5, 0.5}, {0, 2, 0.5, 0.5},     {0.5, 2, 0.5, 0.5},
    {0, 2.5, 0.5, 0.5},   {0.5, 2.5, 0.5, 0.5}, {0, 2, 0.5, 0.5},
    {2.5, 0, 0.5, 0.5},   {0, 2.5, 0.5, 0.5},   {0.5, 2.5, 0.5, 0.5},
    {0, 2, 0.5, 0.5},     {0.5, 2, 0.5, 0.5},   {0, 2.5, 0.5, 0.5},
    {2.5, 0.5, 0.5, 0.5}, {0, 2, 0.5, 0.5},     {2.5, 0, 0.5, 0.5},
    {0, 2.5, 0.5, 0.5},   {2.5, 0.5, 0.5, 0.5}, {1, 1, 0.5, 0.5},
    {1.5, 1, 0.5, 0.5},   {1, 1.5, 0.5, 0.5},   {1.5, 1.5, 0.5, 0.5},
    {1, 1, 0.5, 0.5},     {1.5, 1, 0.5, 0.5},   {1, 1.5, 0.5, 0.5},
    {2.5, 0.5, 0.5, 0.5}, {1, 1, 0.5, 0.5},     {1.5, 1, 0.5, 0.5},
    {2, 0.5, 0.5, 0.5},   {1.5, 1.5, 0.5, 0.5}, {1, 1, 0.5, 0.5},
    {1.5, 1, 0.5, 0.5},   {2, 0.5, 0.5, 0.5},   {2.5, 0.5, 0.5, 0.5},
    {2, 2, 0.5, 0.5},     {2.5, 2, 0.5, 0.5},   {2, 2.5, 0.5, 0.5},
    {2.5, 2.5, 0.5, 0.5}, {2, 2, 0.5, 0.5},     {2.5, 2, 0.5, 0.5},
    {2, 0.5, 0.5, 0.5},   {2.5, 2.5, 0.5, 0.5}, {2, 0, 0.5, 0.5},
    {2.5, 2, 0.5, 0.5},   {2, 2.5, 0.5, 0.5},   {2.5, 2.5, 0.5, 0.5},
    {2, 0, 0.5, 0.5},     {2.5, 2, 0.5, 0.5},   {2, 0.5, 0.5, 0.5},
    {2.5, 2.5, 0.5, 0.5}, {1, 3, 0.5, 0.5},     {1.5, 3, 0.5, 0.5},
    {1, 3.5, 0.5, 0.5},   {1.5, 3.5, 0.5, 0.5}, {2, 0, 0.5, 0.5},
    {1.5, 3, 0.5, 0.5},   {1, 3.5, 0.5, 0.5},   {1.5, 3.5, 0.5, 0.5},
    {1, 3, 0.5, 0.5},     {2.5, 0, 0.5, 0.5},   {1, 3.5, 0.5, 0.5},
    {1.5, 3.5, 0.5, 0.5}, {2, 0, 0.5, 0.5},     {2.5, 0, 0.5, 0.5},
    {1, 3.5, 0.5, 0.5},   {1.5, 3.5, 0.5, 0.5}, {0, 2, 0.5, 0.5},
    {2.5, 2, 0.5, 0.5},   {0, 2.5, 0.5, 0.5},   {2.5, 2.5, 0.5, 0.5},
    {1, 1, 0.5, 0.5},     {1.5, 1, 0.5, 0.5},   {1, 3.5, 0.5, 0.5},
    {1.5, 3.5, 0.5, 0.5}, {0, 1, 0.5, 0.5},     {0.5, 1, 0.5, 0.5},
    {0, 1.5, 0.5, 0.5},   {0.5, 1.5, 0.5, 0.5}, {0, 1, 0.5, 0.5},
    {0.5, 1, 0.5, 0.5},   {0, 1.5, 0.5, 0.5},   {2.5, 0.5, 0.5, 0.5},
    {2, 1, 0.5, 0.5},     {2.5, 1, 0.5, 0.5},   {2, 1.5, 0.5, 0.5},
    {2.5, 1.5, 0.5, 0.5}, {2, 1, 0.5, 0.5},     {2.5, 1, 0.5, 0.5},
    {2, 0.5, 0.5, 0.5},   {2.5, 1.5, 0.5, 0.5}, {2, 3, 0.5, 0.5},
    {2.5, 3, 0.5, 0.5},   {2, 3.5, 0.5, 0.5},   {2.5, 3.5, 0.5, 0.5},
    {2, 0, 0.5, 0.5},     {2.5, 3, 0.5, 0.5},   {2, 3.5, 0.5, 0.5},
    {2.5, 3.5, 0.5, 0.5}, {0, 3, 0.5, 0.5},     {0.5, 3, 0.5, 0.5},
    {0, 3.5, 0.5, 0.5},   {0.5, 3.5, 0.5, 0.5}, {0, 3, 0.5, 0.5},
    {2.5, 0, 0.5, 0.5},   {0, 3.5, 0.5, 0.5},   {0.5, 3.5, 0.5, 0.5},
    {0, 1, 0.5, 0.5},     {2.5, 1, 0.5, 0.5},   {0, 1.5, 0.5, 0.5},
    {2.5, 1.5, 0.5, 0.5}, {0, 1, 0.5, 0.5},     {0.5, 1, 0.5, 0.5},
    {0, 3.5, 0.5, 0.5},   {0.5, 3.5, 0.5, 0.5}, {0, 3, 0.5, 0.5},
    {2.5, 3, 0.5, 0.5},   {0, 3.5, 0.5, 0.5},   {2.5, 3.5, 0.5, 0.5},
    {2, 1, 0.5, 0.5},     {2.5, 1, 0.5, 0.5},   {2, 3.5, 0.5, 0.5},
    {2.5, 3.5, 0.5, 0.5}, {0, 1, 0.5, 0.5},     {2.5, 1, 0.5, 0.5},
    {0, 3.5, 0.5, 0.5},   {2.5, 3.5, 0.5, 0.5}, {0, 0, 0.5, 0.5},
    {0.5, 0, 0.5, 0.5},   {0, 0.5, 0.5, 0.5},   {0.5, 0.5, 0.5, 0.5},
};

const int kGroundLayerDefaultZ = 0;
const int kZLayerDefaultZ = 0;

}  // namespace

class TilemapGroundLayer : public ViewportChild {
 public:
  TilemapGroundLayer(scoped_refptr<Graphics> screen,
                     base::WeakPtr<Tilemap> tilemap)
      : ViewportChild(screen, tilemap->viewport_, kGroundLayerDefaultZ),
        tilemap_(tilemap) {}
  ~TilemapGroundLayer() override {}

  TilemapGroundLayer(const TilemapGroundLayer&) = delete;
  TilemapGroundLayer& operator=(const TilemapGroundLayer&) = delete;

  void InitDrawableData() override { tilemap_->InitTilemapData(); }
  void BeforeComposite() override {}
  void Composite() override {}

  void CheckDisposed() const override { tilemap_->CheckIsDisposed(); }
  void OnViewportRectChanged(
      const DrawableParent::ViewportInfo& rect) override {}

  base::WeakPtr<Tilemap> tilemap_;
};

class TilemapZLayer : public ViewportChild {
 public:
  TilemapZLayer(scoped_refptr<Graphics> screen, base::WeakPtr<Tilemap> tilemap)
      : ViewportChild(screen, tilemap->viewport_, kZLayerDefaultZ),
        tilemap_(tilemap) {}
  ~TilemapZLayer() override {}

  TilemapZLayer(const TilemapZLayer&) = delete;
  TilemapZLayer& operator=(const TilemapZLayer&) = delete;

  void InitDrawableData() override {}
  void BeforeComposite() override {}
  void Composite() override {}

  void CheckDisposed() const override { tilemap_->CheckIsDisposed(); }
  void OnViewportRectChanged(
      const DrawableParent::ViewportInfo& rect) override {}

  base::WeakPtr<Tilemap> tilemap_;
};

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
  return autotiles_[index].bitmap;
}

void Tilemap::SetAutotiles(int index, scoped_refptr<Bitmap> bitmap) {
  CheckIsDisposed();

  if (autotiles_[index].bitmap == bitmap)
    return;

  autotiles_[index].bitmap = bitmap;
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

void Tilemap::OnObjectDisposed() {
  weak_ptr_factory_.InvalidateWeakPtrs();

  tilemap_quads_.reset();
  flash_layer_.reset();

  renderer::TextureFrameBuffer::Del(atlas_tfb_);
}

void Tilemap::InitTilemapData() {
  if (flash_layer_)
    return;

  tilemap_quads_ =
      std::make_unique<renderer::QuadDrawableArray<renderer::CommonVertex>>();
  flash_layer_ = std::make_unique<TilemapFlashLayer>(tile_size_);

  atlas_tfb_ = renderer::TextureFrameBuffer::Gen();
  renderer::TextureFrameBuffer::Alloc(atlas_tfb_, tile_size_, tile_size_);
  renderer::TextureFrameBuffer::LinkFrameBuffer(atlas_tfb_);
}

void Tilemap::UpdateTilemapParameters() {}

void Tilemap::BeforeTilemapComposite() {
  flash_layer_->BeforeComposite();

  if (map_buffer_need_update_) {
    map_buffer_need_update_ = false;
    UpdateMapBufferInternal();
  }

  if (atlas_need_update_) {
    atlas_need_update_ = false;
    MakeAtlasInternal();
  }
}

void Tilemap::MakeAtlasInternal() {
  int atlas_height = 28 * tile_size_;
  if (tileset_ && !tileset_->IsDisposed())
    atlas_height = tileset_->GetSize().x;
  renderer::TextureFrameBuffer::Alloc(atlas_tfb_, tile_size_ * 20,
                                      atlas_height);

  // Autotile part
  int offset = 0;
  for (auto& it : autotiles_) {
    if (!it.bitmap || it.bitmap->IsDisposed()) {
      ++offset;
      continue;
    }

    auto autotile_size = it.bitmap->GetSize();

    base::Rect dst_pos(autotile_size);
    dst_pos.x = 0;
    dst_pos.y = offset * tile_size_ * 4;

    if (autotile_size.x > 3 * tile_size_ && autotile_size.y > tile_size_) {
      // Animated autotile
      it.type = AutotileType::Animated;
    } else if (autotile_size.x <= 3 * tile_size_ &&
               autotile_size.y > tile_size_) {
      // Static autotile
      dst_pos.x += tile_size_ * 3;
      it.type = AutotileType::Static;
    } else if (autotile_size.x <= 4 * tile_size_ &&
               autotile_size.y <= tile_size_) {
      // Single animated tile
      it.type = AutotileType::SingleAnimated;

      base::Vec2i single_size(tile_size_, tile_size_);

      renderer::Blt::BeginDraw(atlas_tfb_);
      renderer::Blt::TexSource(it.bitmap->AsGLType());
      for (int i = 0; i < 4; ++i) {
        renderer::Blt::BltDraw(
            base::Rect(base::Vec2i(tile_size_ * i, 0), single_size),
            base::Rect(base::Vec2i(tile_size_ * i * 3, 0), single_size));
      }
      renderer::Blt::EndDraw();

      ++offset;
      continue;
    } else {
      NOTREACHED();
    }

    renderer::Blt::BeginDraw(atlas_tfb_);
    renderer::Blt::TexSource(it.bitmap->AsGLType());
    renderer::Blt::BltDraw(autotile_size, dst_pos);
    renderer::Blt::EndDraw();

    ++offset;
  }

  // Tileset part
  if (!tileset_ || tileset_->IsDisposed())
    return;
  if (tileset_->GetSize().x > tile_size_ * 8)
    return;

  auto tileset_size = tileset_->GetSize();
  auto dst_rect = tileset_size;
  dst_rect.x += 12 * tile_size_;

  renderer::Blt::BeginDraw(atlas_tfb_);
  renderer::Blt::TexSource(tileset_->AsGLType());
  renderer::Blt::BltDraw(tileset_size, dst_rect);
  renderer::Blt::EndDraw();
}

void Tilemap::UpdateViewportInternal() {
  const DrawableParent::ViewportInfo& viewport_rect =
      ground_layer_->parent_rect();

  const base::Vec2i tilemap_origin = origin_ + viewport_rect.GetRealOffset();
  const base::Vec2i viewport_size = viewport_rect.rect.Size();

  base::Rect new_tilemap_viewport;
  new_tilemap_viewport.x = tilemap_origin.x / tile_size_;
  new_tilemap_viewport.y = tilemap_origin.y / tile_size_ - 1;

  new_tilemap_viewport.width =
      (viewport_size.x / tile_size_) + !!(viewport_size.x % tile_size_) + 1;
  new_tilemap_viewport.height =
      (viewport_size.y / tile_size_) + !!(viewport_size.y % tile_size_) + 2;

  if (new_tilemap_viewport != tilemap_viewport_) {
    tilemap_viewport_ = new_tilemap_viewport;
    map_buffer_need_update_ = true;

    flash_layer_->SetViewport(tilemap_viewport_);
  }

  tilemap_offset_ = viewport_rect.rect.Position() -
                    vwrap(tilemap_origin, tile_size_) -
                    base::Vec2i(0, tile_size_);
}

void Tilemap::UpdateMapBufferInternal() {
  ground_vertices_.clear();
  above_vertices_.clear();

  auto autotile_subpos = [&](base::RectF& pos, int i) {
    switch (i) {
      case TopLeft:
        break;
      case TopRight:
        pos.x += tile_size_ / 2.0f;
        break;
      case BottomLeft:
        pos.y += tile_size_ / 2.0f;
        break;
      case BottomRight:
        pos.x += tile_size_ / 2.0f;
        pos.y += tile_size_ / 2.0f;
        break;
      default:
        break;
    }
  };

  auto get_priority = [&](int tileID) -> int {
    if (!priorities_ || tileID >= priorities_->GetXSize())
      return 0;

    int value = priorities_->At(tileID);
    if (value > 5)
      return -1;
    return value;
  };

  auto process_autotile = [&](int x, int y, int tileID,
                              std::vector<renderer::CommonVertex>* target) {
    // autotile index 0-7
    int autotileID = tileID / 48 - 1;
    // pattern id (0-47)
    int patternID = tileID % 48;

    AutotileInfo& info = autotiles_[autotileID];
    if (!info.bitmap || info.bitmap->IsDisposed())
      return;

    switch (info.type) {
      case AutotileType::Animated:
      case AutotileType::Static: {
        const base::RectF* autotile_src = &kAutotileSrcRects[patternID * 4];
        for (int i = 0; i < 4; ++i) {
          renderer::CommonVertex verts[4];
          base::RectF chunk_pos(x * tile_size_, y * tile_size_,
                                tile_size_ / 2.0f, tile_size_ / 2.0f);
          autotile_subpos(chunk_pos, i);
          renderer::QuadSetTexPosRect(verts, autotile_src[i], chunk_pos);
          for (int j = 0; j < 4; ++j)
            target->push_back(verts[i]);
        }
      } break;
      case AutotileType::SingleAnimated: {
        renderer::CommonVertex verts[4];
        const base::RectF single_tex(0.5, 0.5, tile_size_ - 1, tile_size_ - 1);
        const base::RectF single_pos(x * tile_size_, y * tile_size_, tile_size_,
                                     tile_size_);
        renderer::QuadSetTexPosRect(verts, single_tex, single_pos);
        for (int i = 0; i < 4; ++i)
          target->push_back(verts[i]);
      } break;
      default:
        NOTREACHED();
        break;
    }
  };

  auto process_tile = [&](int x, int y, int z) {
    int tileID = TableGetWrapped(map_data_, x + tilemap_viewport_.x,
                                 y + tilemap_viewport_.y, z);

    if (tileID < 48)
      return;

    int priority = get_priority(tileID);
    if (priority == -1)
      return;

    std::vector<renderer::CommonVertex>* target;
    if (!priority) {
      // Ground layer
      target = &ground_vertices_;
    } else {
      // Above multi layers
      target = &above_vertices_[y + priority];
    }

    if (tileID < 48 * 8)
      return process_autotile(x, y, tileID, target);

    int tilesetID = tileID - 48 * 8;
    int tileX = tilesetID % 8;
    int tileY = tilesetID / 8;

    base::Vec2i atlas_offset;
    base::RectF tex((float)atlas_offset.x * tile_size_ + 0.5f,
                    (float)atlas_offset.y * tile_size_ + 0.5f, tile_size_ - 1,
                    tile_size_ - 1);
    base::RectF pos(x * tile_size_, y * tile_size_, tile_size_, tile_size_);
    renderer::CommonVertex verts[4];
    renderer::QuadSetTexPosRect(verts, tex, pos);

    for (int i = 0; i < 4; ++i)
      target->push_back(verts[i]);
  };

  auto process_buffer = [&]() {
    for (int x = 0; x < tilemap_viewport_.width; ++x)
      for (int y = 0; y < tilemap_viewport_.height; ++y)
        for (int z = 0; z < map_data_->GetZSize(); ++z)
          process_tile(x, y, z);
  };

  // Process map buffer
  int zlayer_num = tilemap_viewport_.height + 5;
  above_vertices_.resize(zlayer_num);

  // Begin to parse buffer data
  process_buffer();

  // Update quads
  int vertex_count = ground_vertices_.size();
  for (auto& it : above_vertices_)
    vertex_count += it.size();
  tilemap_quads_->Resize(vertex_count / 4);
  if (!vertex_count)
    return;

  renderer::CommonVertex* vert = &tilemap_quads_->vertices()[0];
  memcpy(vert, ground_vertices_.data(),
         ground_vertices_.size() * sizeof(renderer::CommonVertex));
  vert += ground_vertices_.size();
  for (auto& it : above_vertices_) {
    memcpy(vert, it.data(), it.size() * sizeof(renderer::CommonVertex));
    vert += it.size();
  }

  tilemap_quads_->Update();
}

void Tilemap::ResetDrawLayerInternal() {
  int layer_num = tilemap_viewport_.height + 5;

  ground_layer_.reset(
      new TilemapGroundLayer(screen(), weak_ptr_factory_.GetWeakPtr()));
  above_layers_.clear();
  for (int i = 0; i < layer_num; ++i) {
    std::unique_ptr<TilemapZLayer> new_layer(
        new TilemapZLayer(screen(), weak_ptr_factory_.GetWeakPtr()));
    above_layers_.push_back(std::move(new_layer));
  }
}

}  // namespace content
