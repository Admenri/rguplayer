// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/tilemap.h"

#include "content/public/bitmap.h"

#include "SDL_pixels.h"
#include "SDL_surface.h"

namespace {

static const int kGroundLayerDefaultZ = 0;
static const int kAboveLayerDefaultZ = 200;

using TilemapBlock = struct {
  base::Vec2i pos;
  base::Vec2i size;
};

using TilemapVXAtlasBlock = struct {
  content::Tilemap2::TilemapBitmapID tile_id;
  TilemapBlock src;
  base::Vec2i dst;
};

static const TilemapVXAtlasBlock kTilemapAtlas[] = {
    /* A1 tilemap */
    {
        content::Tilemap2::TileA1,
        {
            base::Vec2i{0, 0},
            base::Vec2i{6, 6},
        },
        base::Vec2i{0, 0},
    },
    {
        content::Tilemap2::TileA1,
        {
            base::Vec2i{8, 0},
            base::Vec2i{6, 6},
        },
        base::Vec2i{6, 0},
    },
    {
        content::Tilemap2::TileA1,
        {
            base::Vec2i{0, 6},
            base::Vec2i{6, 6},
        },
        base::Vec2i{0, 6},
    },
    {
        content::Tilemap2::TileA1,
        {
            base::Vec2i{8, 6},
            base::Vec2i{6, 6},
        },
        base::Vec2i{6, 6},
    },
    {
        content::Tilemap2::TileA1,
        {
            base::Vec2i{6, 0},
            base::Vec2i{2, 12},
        },
        base::Vec2i{12, 0},
    },
    {
        content::Tilemap2::TileA1,
        {
            base::Vec2i{14, 0},
            base::Vec2i{2, 12},
        },
        base::Vec2i{14, 0},
    },
    /* A2 tilemap */
    {
        content::Tilemap2::TileA2,
        {
            base::Vec2i{0, 0},
            base::Vec2i{16, 12},
        },
        base::Vec2i{16, 0},
    },
    /* A3 tilemap */
    {
        content::Tilemap2::TileA3,
        {
            base::Vec2i{0, 0},
            base::Vec2i{16, 8},
        },
        base::Vec2i{0, 12},
    },
    /* A4 tilemap */
    {
        content::Tilemap2::TileA4,
        {
            base::Vec2i{0, 0},
            base::Vec2i{16, 15},
        },
        base::Vec2i{16, 12},
    },
    /* A5 tilemap */
    {
        content::Tilemap2::TileA5,
        {
            base::Vec2i{0, 0},
            base::Vec2i{8, 8},
        },
        base::Vec2i{0, 20},
    },
    {
        content::Tilemap2::TileA5,
        {
            base::Vec2i{0, 8},
            base::Vec2i{8, 8},
        },
        base::Vec2i{8, 20},
    },
    /* B tilemap */
    {
        content::Tilemap2::TileB,
        {
            base::Vec2i{0, 0},
            base::Vec2i{16, 16},
        },
        base::Vec2i{32, 0},
    },
    /* C tilemap */
    {
        content::Tilemap2::TileC,
        {
            base::Vec2i{0, 0},
            base::Vec2i{16, 16},
        },
        base::Vec2i{48, 0},
    },
    /* D tilemap */
    {
        content::Tilemap2::TileD,
        {
            base::Vec2i{0, 0},
            base::Vec2i{16, 16},
        },
        base::Vec2i{32, 16},
    },
    /* E tilemap */
    {
        content::Tilemap2::TileE,
        {
            base::Vec2i{0, 0},
            base::Vec2i{16, 16},
        },
        base::Vec2i{48, 16},
    },
};

static const size_t kTilemapAtlasSize =
    sizeof(kTilemapAtlas) / sizeof(kTilemapAtlas[0]);

static const TilemapVXAtlasBlock kShadowAtlasArea = {
    content::Tilemap2::TilemapBitmapID(),
    {
        base::Vec2i{0, 0},
        base::Vec2i{16, 1},
    },
    base::Vec2i{16, 27},
};

static const TilemapVXAtlasBlock kFreeAtlasArea = {
    content::Tilemap2::TilemapBitmapID(),
    {
        base::Vec2i{0, 0},
        base::Vec2i{16, 4},
    },
    base::Vec2i{0, 28},
};

static SDL_Surface* CreateShadowSet(int tilesize) {
  SDL_Surface* surf = SDL_CreateSurface(kShadowAtlasArea.src.size.x * tilesize,
                                        kShadowAtlasArea.src.size.y * tilesize,
                                        SDL_PIXELFORMAT_ABGR8888);

  std::vector<SDL_Rect> rects;
  SDL_Rect rect = {0, 0, tilesize / 2, tilesize / 2};

  for (int val = 0; val < 16; ++val) {
    int origY = val * tilesize;

    /* Top left */
    if (val & (1 << 0)) {
      rect.x = 0;
      rect.y = origY;
      rects.push_back(rect);
    }

    /* Top Right */
    if (val & (1 << 1)) {
      rect.x = tilesize / 2;
      rect.y = origY;
      rects.push_back(rect);
    }

    /* Bottom left */
    if (val & (1 << 2)) {
      rect.x = 0;
      rect.y = origY + tilesize / 2;
      rects.push_back(rect);
    }

    /* Bottom right */
    if (val & (1 << 3)) {
      rect.x = tilesize / 2;
      rect.y = origY + tilesize / 2;
      rects.push_back(rect);
    }
  }

  /* Fill rects with half opacity black */
  uint32_t color = (0x80808080 & surf->format->Amask);
  SDL_FillSurfaceRects(surf, rects.data(), rects.size(), color);

  return surf;
}

}  // namespace

namespace content {

// Reference: https://www.tktkgame.com/tkool/memo/vx/tile_id.html
class GroundLayer : public ViewportChild {
 public:
  GroundLayer(scoped_refptr<Graphics> screen, base::WeakPtr<Tilemap2> tilemap)
      : ViewportChild(screen, tilemap->viewport_, kGroundLayerDefaultZ),
        tilemap_(tilemap) {}
  ~GroundLayer() override {}

  GroundLayer(const GroundLayer&) = delete;
  GroundLayer& operator=(const GroundLayer&) = delete;

  void BeforeComposite() override { tilemap_->BeforeTilemapComposite(); }

  void Composite() override {}

  void CheckDisposed() const override { tilemap_->CheckIsDisposed(); }

  void OnViewportRectChanged(
      const DrawableParent::ViewportInfo& rect) override {}

  base::WeakPtr<Tilemap2> tilemap_;
};

class AboveLayer : public ViewportChild {
 public:
  AboveLayer(scoped_refptr<Graphics> screen, base::WeakPtr<Tilemap2> tilemap)
      : ViewportChild(screen, tilemap->viewport_, kAboveLayerDefaultZ),
        tilemap_(tilemap) {}
  ~AboveLayer() override {}

  AboveLayer(const AboveLayer&) = delete;
  AboveLayer& operator=(const AboveLayer&) = delete;

  void BeforeComposite() override { tilemap_->BeforeTilemapComposite(); }

  void Composite() override {}

  void CheckDisposed() const override { tilemap_->CheckIsDisposed(); }

  void OnViewportRectChanged(
      const DrawableParent::ViewportInfo& rect) override {}

  base::WeakPtr<Tilemap2> tilemap_;
};

Tilemap2::Tilemap2(scoped_refptr<Graphics> screen,
                   scoped_refptr<Viewport> viewport,
                   int tilesize)
    : GraphicElement(screen),
      Disposable(screen),
      viewport_(viewport),
      tile_size_(tilesize) {
  ground_ =
      std::make_unique<GroundLayer>(screen, weak_ptr_factory_.GetWeakPtr());
  above_ = std::make_unique<AboveLayer>(screen, weak_ptr_factory_.GetWeakPtr());
}

Tilemap2::~Tilemap2() {
  Dispose();
}

void Tilemap2::Update() {}

scoped_refptr<Bitmap> Tilemap2::GetBitmap(int index) const {
  CheckIsDisposed();
  return bitmaps_[index];
}

void Tilemap2::SetBitmap(int index, scoped_refptr<Bitmap> bitmap) {
  CheckIsDisposed();

  if (bitmaps_[index] == bitmap)
    return;
  bitmaps_[index] = bitmap;
}

scoped_refptr<Table> Tilemap2::GetMapData() const {
  CheckIsDisposed();
  return map_data_;
}

void Tilemap2::SetMapData(scoped_refptr<Table> map_data) {
  CheckIsDisposed();

  if (map_data_ == map_data)
    return;
  map_data_ = map_data;
}

scoped_refptr<Table> Tilemap2::GetFlashData() const {
  CheckIsDisposed();
  return flash_data_;
}

void Tilemap2::SetFlashData(scoped_refptr<Table> flash_data) {
  CheckIsDisposed();

  if (flash_data_ == flash_data)
    return;
  flash_data_ = flash_data;
}

scoped_refptr<Table> Tilemap2::GetFlags() const {
  CheckIsDisposed();
  return flags_;
}

void Tilemap2::SetFlags(scoped_refptr<Table> flags) {
  CheckIsDisposed();

  if (flags_ == flags)
    return;
  flags_ = flags;
}

scoped_refptr<Viewport> Tilemap2::GetViewport() const {
  CheckIsDisposed();
  return viewport_;
}

void Tilemap2::SetViewport(scoped_refptr<Viewport> viewport) {
  CheckIsDisposed();

  if (viewport_ == viewport)
    return;

  ground_->SetViewport(viewport_);
  above_->SetViewport(viewport_);
  above_->SetZ(kAboveLayerDefaultZ);
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

  if (origin_.x == ox)
    return;
  origin_.x = ox;
}

int Tilemap2::GetOY() const {
  CheckIsDisposed();
  return origin_.y;
}

void Tilemap2::SetOY(int oy) {
  CheckIsDisposed();

  if (origin_.y == oy)
    return;
  origin_.y = oy;
}

void Tilemap2::OnObjectDisposed() {
  ground_.reset();
  above_.reset();

  weak_ptr_factory_.InvalidateWeakPtrs();

  screen()->renderer()->PostTask(
      base::BindOnce(&renderer::VertexArray<renderer::CommonVertex>::Uninit,
                     base::OwnedRef(std::move(vao_))));
  screen()->renderer()->PostTask(
      base::BindOnce(&renderer::TextureFrameBuffer::Del,
                     base::OwnedRef(std::move(atlas_tfb_))));
}

void Tilemap2::BeforeTilemapComposite() {
  if (atlas_need_update_) {
    CreateTileAtlasInternal();
    atlas_need_update_ = false;
  }
}

void Tilemap2::InitTilemapInternal() {
  vao_.ibo = renderer::GSM.quad_ibo->ibo;
  vao_.vbo = renderer::VertexBuffer::Gen();
  renderer::VertexArray<renderer::CommonVertex>::Init(vao_);

  atlas_tfb_ = renderer::TextureFrameBuffer::Gen();
  renderer::TextureFrameBuffer::Alloc(atlas_tfb_, tile_size_, tile_size_);
  renderer::TextureFrameBuffer::LinkFrameBuffer(atlas_tfb_);
}

void Tilemap2::CreateTileAtlasInternal() {
  renderer::TextureFrameBuffer::Alloc(atlas_tfb_, tile_size_ * 64,
                                      tile_size_ * 32);

  /* tilemap bitmap atlas */
  for (int i = 0; i < kTilemapAtlasSize; ++i) {
    auto& atlas_info = kTilemapAtlas[i];
    scoped_refptr<Bitmap> atlas_bitmap = bitmaps_[atlas_info.tile_id];

    base::Rect src_rect(
        atlas_info.src.pos.x * tile_size_, atlas_info.src.pos.y * tile_size_,
        atlas_info.src.size.x * tile_size_, atlas_info.src.size.y * tile_size_);
    base::Rect dst_rect(
        atlas_info.dst.x * tile_size_, atlas_info.dst.y * tile_size_,
        atlas_info.src.size.x * tile_size_, atlas_info.src.size.y * tile_size_);

    renderer::Blt::BeginDraw(atlas_tfb_);
    renderer::Blt::TexSource(atlas_bitmap->AsGLType());
    renderer::Blt::EndDraw(src_rect, dst_rect);
  }

  /* shadow set atlas */
  SDL_Surface* shadow_set = CreateShadowSet(tile_size_);
  renderer::Texture::Bind(atlas_tfb_.tex);
  renderer::Texture::TexSubImage2D(
      kShadowAtlasArea.dst.x * tile_size_, kShadowAtlasArea.dst.y * tile_size_,
      kShadowAtlasArea.src.size.x * tile_size_,
      kShadowAtlasArea.src.size.y * tile_size_, GL_RGBA, shadow_set->pixels);
  SDL_DestroySurface(shadow_set);
}

}  // namespace content