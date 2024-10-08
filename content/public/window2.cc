// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/window2.h"

#include "content/common/tilequad.h"
#include "content/common/tileutils.h"

namespace content {

/* Asset rect fix data from Other runtime source */
static const base::Rect background_stretch_src = {0, 0, 64, 64};
static const base::Rect frame_tile_src = {64, 0, 64, 64};
static const base::Rect background_tile_src = {0, 64, 64, 64};

static const Corners<base::Rect> corner_src = {
    base::Rect(64, 0, 16, 16), base::Rect(112, 0, 16, 16),
    base::Rect(64, 48, 16, 16), base::Rect(112, 48, 16, 16)};

static const Sides<base::Rect> border_src = {
    base::Rect(64, 16, 16, 32), base::Rect(112, 16, 16, 32),
    base::Rect(80, 0, 32, 16), base::Rect(80, 48, 32, 16)};

static const Sides<base::Rect> scroll_arrow_src = {
    base::Rect(80, 24, 8, 16), base::Rect(104, 24, 8, 16),
    base::Rect(88, 16, 16, 8), base::Rect(88, 40, 16, 8)};

static const base::Rect pause_src[4] = {
    base::Rect(96, 64, 16, 16), base::Rect(112, 64, 16, 16),
    base::Rect(96, 80, 16, 16), base::Rect(112, 80, 16, 16)};

static const uint8_t pause_quad[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};

static const uint8_t pause_alpha[] = {0x00, 0x20, 0x40, 0x60, 0x80,
                                      0xA0, 0xC0, 0xE0, 0xFF};

struct CursorSrc {
  Corners<base::Rect> corners;
  Sides<base::Rect> border;
  base::Rect background;
};

static const CursorSrc cursor_src = {
    {base::Rect(64, 64, 4, 4), base::Rect(92, 64, 4, 4),
     base::Rect(64, 92, 4, 4), base::Rect(92, 92, 4, 4)},
    {base::Rect(64, 68, 4, 24), base::Rect(92, 68, 4, 24),
     base::Rect(68, 64, 24, 4), base::Rect(68, 92, 24, 4)},
    base::Rect(68, 68, 24, 24)};

static const uint8_t cursor_alpha[] = {
    /* Fade out */
    0xFF, 0xF7, 0xEF, 0xE7, 0xDF, 0xD7, 0xCF, 0xC7, 0xBF, 0xB7, 0xAF, 0xA7,
    0x9F, 0x97, 0x8F, 0x87, 0x7F, 0x77, 0x6F, 0x67,
    /* Fade in */
    0x5F, 0x67, 0x6F, 0x77, 0x7F, 0x87, 0x8F, 0x97, 0x9F, 0xA7, 0xAF, 0xB7,
    0xBF, 0xC7, 0xCF, 0xD7, 0xDF, 0xE7, 0xEF, 0xF7};

static const uint8_t cursor_alpha_reset = 0x10;

Window2::Window2(scoped_refptr<Graphics> screen,
                 scoped_refptr<Viewport> viewport)
    : GraphicsElement(screen.get()),
      Disposable(screen.get()),
      ViewportChild(screen,
                    viewport,
                    (screen->api_version() >= APIVersion::RGSS3 ? 100 : 0),
                    (screen->api_version() >= APIVersion::RGSS3
                         ? std::numeric_limits<int>::max()
                         : 0)),
      rect_(0, 0, 0, 0) {
  InitWindow();
}

Window2::Window2(scoped_refptr<Graphics> screen, const base::Rect& rect)
    : GraphicsElement(screen.get()),
      Disposable(screen.get()),
      ViewportChild(screen,
                    nullptr,
                    (screen->api_version() >= APIVersion::RGSS3 ? 100 : 0),
                    (screen->api_version() >= APIVersion::RGSS3
                         ? std::numeric_limits<int>::max()
                         : 0)),
      rect_(rect) {
  InitWindow();
}

Window2::~Window2() {
  Dispose();
}

void Window2::Update() {
  CheckIsDisposed();

  update_required_ = true;
}

void Window2::Move(int x, int y, int width, int height) {
  CheckIsDisposed();

  width = std::max(0, width);
  height = std::max(0, height);

  if (rect_ == base::Rect(x, y, width, height))
    return;

  rect_ = base::Rect(x, y, width, height);
  base_tex_need_update_ = true;
  base_quad_need_update_ = true;
  arrows_quad_need_update_ = true;
}

void Window2::SetWindowskin(scoped_refptr<Bitmap> windowskin) {
  CheckIsDisposed();

  if (windowskin_ == windowskin)
    return;

  windowskin_ = windowskin;
  base_tex_need_update_ = true;

  if (IsObjectValid(windowskin_.get()))
    windowskin_observer_ = windowskin_->AddBitmapObserver(base::BindRepeating(
        &Window2::WindowskinChangedInternal, base::Unretained(this)));
}

void Window2::SetContents(scoped_refptr<Bitmap> contents) {
  CheckIsDisposed();

  if (contents_ == contents)
    return;

  contents_ = contents;
  arrows_quad_need_update_ = true;
  contents_quad_need_update_ = true;
}

void Window2::SetCursorRect(scoped_refptr<Rect> cursor_rect) {
  CheckIsDisposed();

  if (cursor_rect_->IsSame(*cursor_rect))
    return;

  *cursor_rect_ = *cursor_rect;
  cursor_quad_need_update_ = true;
}

void Window2::SetActive(bool active) {
  CheckIsDisposed();

  if (active_ == active)
    return;

  active_ = active;
  cursor_alpha_index_ = cursor_alpha_reset;
  cursor_step_need_update_ = true;
}

void Window2::SetArrowsVisible(bool arrows_visible) {
  CheckIsDisposed();

  if (arrows_visible_ == arrows_visible)
    return;

  arrows_visible_ = arrows_visible;
  arrows_quad_need_update_ = true;
}

void Window2::SetPause(bool pause) {
  CheckIsDisposed();

  if (pause_ == pause)
    return;

  pause_ = pause;
  pause_alpha_index_ = 0;
  pause_quad_index_ = 0;
  arrows_quad_need_update_ = true;
}

void Window2::SetX(int x) {
  CheckIsDisposed();
  rect_.x = x;
}

void Window2::SetY(int y) {
  CheckIsDisposed();
  rect_.y = y;
}

void Window2::SetWidth(int width) {
  CheckIsDisposed();

  width = std::max(0, width);

  if (rect_.width == width)
    return;

  rect_.width = width;
  base_tex_need_update_ = true;
  base_quad_need_update_ = true;
  arrows_quad_need_update_ = true;
}

void Window2::SetHeight(int height) {
  CheckIsDisposed();

  height = std::max(0, height);

  if (rect_.height == height)
    return;

  rect_.height = height;
  base_tex_need_update_ = true;
  base_quad_need_update_ = true;
  arrows_quad_need_update_ = true;
}

void Window2::SetOX(int ox) {
  CheckIsDisposed();

  if (ox_ == ox)
    return;

  ox_ = ox;
  arrows_quad_need_update_ = true;
}

void Window2::SetOY(int oy) {
  CheckIsDisposed();

  if (oy_ == oy)
    return;

  oy_ = oy;
  arrows_quad_need_update_ = true;
}

void Window2::SetPadding(int padding) {
  CheckIsDisposed();

  if (padding_ == padding)
    return;

  padding_ = padding;
  padding_bottom_ = padding;
}

void Window2::SetPaddingBottom(int padding_bottom) {
  CheckIsDisposed();

  if (padding_bottom_ == padding_bottom)
    return;

  padding_bottom_ = padding_bottom;
}

void Window2::SetOpacity(int opacity) {
  CheckIsDisposed();

  opacity = std::clamp(opacity, 0, 255);

  if (opacity_ == opacity)
    return;

  opacity_ = opacity;
  base_quad_need_update_ = true;
}

void Window2::SetBackOpacity(int back_opacity) {
  CheckIsDisposed();

  back_opacity = std::clamp(back_opacity, 0, 255);

  if (back_opacity_ == back_opacity)
    return;

  back_opacity_ = back_opacity;
  base_tex_need_update_ = true;
}

void Window2::SetContentsOpacity(int contents_opacity) {
  CheckIsDisposed();

  contents_opacity = std::clamp(contents_opacity, 0, 255);

  if (contents_opacity_ == contents_opacity)
    return;

  contents_opacity_ = contents_opacity;
  contents_quad_need_update_ = true;
}

void Window2::SetOpenness(int openness) {
  CheckIsDisposed();

  openness = std::clamp(openness, 0, 255);

  if (openness_ == openness)
    return;

  openness_ = openness;
  base_quad_need_update_ = true;
}

void Window2::SetTone(scoped_refptr<Tone> tone) {
  CheckIsDisposed();

  if (tone_->IsSame(*tone))
    return;

  *tone_ = *tone;
  base_tex_need_update_ = true;
}

void Window2::OnObjectDisposed() {
  RemoveFromList();

  renderer_data_.reset();
  if (bgfx::isValid(base_tiled_texture_.handle))
    bgfx::destroy(base_tiled_texture_.handle);
}

void Window2::PrepareDraw(bgfx::Encoder* encoder, bgfx::ViewId* render_view) {
  if (update_required_) {
    update_required_ = false;
    UpdateInternal();
  }

  padding_rect_ =
      base::Rect(padding_, padding_, std::max(0, rect_.width - padding_ * 2),
                 std::max(0, rect_.height - (padding_ + padding_bottom_)));

  if (base_tex_need_update_) {
    base_tex_need_update_ = false;
    UpdateBaseTextureInternal(encoder, render_view);
  }

  if (base_quad_need_update_) {
    base_quad_need_update_ = false;
    UpdateBaseQuadInternal();
  }

  if (arrows_quad_need_update_) {
    arrows_quad_need_update_ = false;
    CalcArrowsQuadArrayInternal();
    UpdatePauseStepInternal();
  }

  if (cursor_quad_need_update_) {
    cursor_quad_need_update_ = false;
    UpdateCursorQuadsInternal();
    cursor_step_need_update_ = true;
  }

  if (cursor_step_need_update_) {
    cursor_step_need_update_ = false;
    if (renderer_data_->cursor_quads->count() > 0) {
      base::Vec4 color(0, 0, 0, cursor_alpha[cursor_alpha_index_] / 255.0f);
      for (size_t i = 0; i < renderer_data_->cursor_quads->count(); ++i)
        renderer::GeometryVertexLayout::SetColor(
            &renderer_data_->cursor_quads->vertices()[i * 4], color);
      cursor_data_need_update_ = true;
    }
  }

  if (contents_quad_need_update_) {
    contents_quad_need_update_ = false;
    if (IsObjectValid(contents_.get())) {
      base::Rect contents_rect = contents_->GetSize();
      renderer_data_->content_quad->SetTexcoord(contents_rect);
      renderer_data_->content_quad->SetPosition(contents_rect);
      renderer_data_->content_quad->SetColor(
          base::Vec4(0, 0, 0, static_cast<float>(contents_opacity_) / 255.0f));
      cursor_data_need_update_ = true;
    }
  }

  if (cursor_data_need_update_) {
    cursor_data_need_update_ = false;
    renderer_data_->cursor_quads->Update();
  }
}

void Window2::OnDraw(CompositeTargetInfo* target_info) {
  bool windowskin_valid = IsObjectValid(windowskin_.get());
  bool contents_valid = IsObjectValid(contents_.get());

  base::Vec2i trans_offset = rect_.Position() + parent_rect().GetRealOffset();
  base::Rect window_scissor = base::Rect(trans_offset, rect_.Size());
  base::Rect scissor_region = window_scissor;
  if (target_info->render_scissor.enable)
    scissor_region =
        base::MakeIntersect(target_info->render_scissor.region, window_scissor);

  /* Stretch background & frame */
  auto& shader = screen()->device()->pipelines().basealpha;
  base::Vec4 offset_size;

  if (windowskin_valid) {
    /* Base window draw */
    if (opacity_ > 0) {
      target_info->encoder->setTexture(
          0, shader.Texture(), bgfx::getTexture(base_tiled_texture_.handle));

      offset_size =
          base::MakeVec4(trans_offset, base::MakeInvert(rect_.Size()));
      target_info->encoder->setUniform(shader.OffsetTexSize(), &offset_size);

      target_info->encoder->setState(
          BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A |
          renderer::MakeColorBlendState(renderer::BlendType::Normal));
      target_info->SetScissorRegion(scissor_region);

      renderer_data_->base_quad->Draw(target_info->encoder, shader.GetProgram(),
                                      target_info->render_view);
    }

    /* Controls draw */
    if (openness_ >= 255 && arrows_quad_count_) {
      target_info->encoder->setTexture(
          0, shader.Texture(), bgfx::getTexture(windowskin_->GetHandle()));

      offset_size = base::MakeVec4(trans_offset,
                                   base::MakeInvert(windowskin_->GetSize()));
      target_info->encoder->setUniform(shader.OffsetTexSize(), &offset_size);

      target_info->encoder->setState(
          BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A |
          renderer::MakeColorBlendState(renderer::BlendType::Normal));
      target_info->SetScissorRegion(scissor_region);

      renderer_data_->arrows_quads->Draw(
          target_info->encoder, shader.GetProgram(), 0, arrows_quad_count_,
          target_info->render_view);
    }
  }

  if (openness_ < 255)
    return;

  base::Rect padding_trans_rect = padding_rect_;
  padding_trans_rect.x += trans_offset.x;
  padding_trans_rect.y += trans_offset.y;

  if (screen()->api_version() >= APIVersion::RGSS3)
    scissor_region = base::MakeIntersect(scissor_region, padding_trans_rect);

  /* Cursor draw */
  if (renderer_data_->cursor_quads->count() > 0 && windowskin_valid) {
    base::Vec2i cursor_trans = padding_trans_rect.Position();
    cursor_trans.x += cursor_rect_->GetX();
    cursor_trans.y += cursor_rect_->GetY();

    if (screen()->api_version() >= APIVersion::RGSS3)
      cursor_trans = cursor_trans - base::Vec2i(ox_, oy_);

    offset_size =
        base::MakeVec4(cursor_trans, base::MakeInvert(windowskin_->GetSize()));
    target_info->encoder->setUniform(shader.OffsetTexSize(), &offset_size);

    target_info->encoder->setTexture(
        0, shader.Texture(), bgfx::getTexture(windowskin_->GetHandle()));

    target_info->encoder->setState(
        BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A |
        renderer::MakeColorBlendState(renderer::BlendType::Normal));
    target_info->SetScissorRegion(scissor_region);

    renderer_data_->cursor_quads->Draw(
        target_info->encoder, shader.GetProgram(), target_info->render_view);
  }

  /* Window contents */
  if (contents_valid && contents_opacity_ > 0) {
    if (screen()->api_version() < APIVersion::RGSS3)
      scissor_region = base::MakeIntersect(scissor_region, padding_trans_rect);

    base::Vec2i content_trans = padding_trans_rect.Position();
    content_trans = content_trans - base::Vec2i(ox_, oy_);

    offset_size =
        base::MakeVec4(content_trans, base::MakeInvert(contents_->GetSize()));
    target_info->encoder->setUniform(shader.OffsetTexSize(), &offset_size);

    target_info->encoder->setTexture(0, shader.Texture(),
                                     bgfx::getTexture(contents_->GetHandle()));

    target_info->encoder->setState(
        BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A |
        renderer::MakeColorBlendState(renderer::BlendType::Normal));
    target_info->SetScissorRegion(scissor_region);

    renderer_data_->content_quad->Draw(
        target_info->encoder, shader.GetProgram(), target_info->render_view);
  }
}

void Window2::OnParentViewportRectChanged(
    const DrawableParent::ViewportInfo& rect) {
  // Passed on composite trans calculate
}

void Window2::InitWindow() {
  padding_ = screen()->api_version() >= APIVersion::RGSS3 ? 12 : 16;
  back_opacity_ = screen()->api_version() >= APIVersion::RGSS3 ? 192 : 255;

  tone_ = new Tone();
  tone_observer_ = tone_->AddChangedObserver(base::BindRepeating(
      &Window2::ToneChangedInternal, base::Unretained(this)));

  contents_ = new Bitmap(static_cast<Graphics*>(screen()), base::Vec2i(1, 1));

  cursor_rect_ = new Rect();
  cursor_rect_observer_ = cursor_rect_->AddChangedObserver(base::BindRepeating(
      &Window2::CursorRectChangedInternal, base::Unretained(this)));

  renderer_data_ = std::make_unique<Window2RendererData>();

  renderer_data_->base_quad = std::make_unique<renderer::QuadDrawable>(
      screen()->device()->quad_indices());
  renderer_data_->content_quad = std::make_unique<renderer::QuadDrawable>(
      screen()->device()->quad_indices());
  renderer_data_->arrows_quads =
      std::make_unique<renderer::QuadArray>(screen()->device()->quad_indices());
  renderer_data_->cursor_quads =
      std::make_unique<renderer::QuadArray>(screen()->device()->quad_indices());
  renderer_data_->base_tex_quad_array =
      std::make_unique<renderer::QuadArray>(screen()->device()->quad_indices());
  renderer_data_->cursor_quads->Resize(1);
  renderer_data_->arrows_quads->Resize(5);

  pause_vertex_ = nullptr;
  contents_quad_need_update_ = true;

  /*
   * Stretch layer
   * Frame layer
   * Cursor layer
   * Control layer
   * Contents layer
   */
}

void Window2::CalcBaseQuadArrayInternal() {
  size_t quad_count = 0;

  /* Base layer (background layer & frame layer) */
  /* Stretch layer x 1 */
  quad_count += 1;

  /* Tiled layer x 2 */
  /* Background layer */
  const base::Rect background_rect{2, 2, rect_.width - 4, rect_.height - 4};
  base_bg_tile_count_ =
      QuadTileCount2D(frame_tile_src.width, frame_tile_src.height,
                      background_rect.width, background_rect.height);
  quad_count += base_bg_tile_count_;

  /* Frame layer */
  base_frame_tile_count_ = 4;
  /* Frame sides */
  const base::Vec2i frame_size{rect_.width - 16 * 2, rect_.height - 16 * 2};
  if (frame_size.x > 0) {
    base_frame_tile_count_ += QuadTileCount(32, frame_size.x) * 2;
  }
  if (frame_size.y > 0) {
    base_frame_tile_count_ += QuadTileCount(32, frame_size.y) * 2;
  }
  quad_count += base_frame_tile_count_;

  /* Set vertex data */
  renderer_data_->base_tex_quad_array->Resize(quad_count);

  /* Fill vertex data */
  renderer::GeometryVertexLayout::Data* vertex =
      renderer_data_->base_tex_quad_array->vertices().data();

  int i = 0;
  /* Stretch background */
  i += renderer::GeometryVertexLayout::SetTexPos(
      &vertex[i * 4], background_stretch_src, background_rect);

  /* Tiled background */
  i += BuildTiles<renderer::GeometryVertexLayout::Data>(
      &vertex[i * 4], background_tile_src, background_rect);

  /* Frame Corners */
  const Corners<base::Rect> corner_pos = {
      base::Rect(0, 0, 16, 16),                               /* Top left */
      base::Rect(rect_.width - 16, 0, 16, 16),                /* Top right */
      base::Rect(0, rect_.height - 16, 16, 16),               /* Bottom left */
      base::Rect(rect_.width - 16, rect_.height - 16, 16, 16) /* Bottom right */
  };

  i += renderer::GeometryVertexLayout::SetTexPos(
      &vertex[i * 4], corner_src.top_left, corner_pos.top_left);
  i += renderer::GeometryVertexLayout::SetTexPos(
      &vertex[i * 4], corner_src.top_right, corner_pos.top_right);
  i += renderer::GeometryVertexLayout::SetTexPos(
      &vertex[i * 4], corner_src.bottom_left, corner_pos.bottom_left);
  i += renderer::GeometryVertexLayout::SetTexPos(
      &vertex[i * 4], corner_src.bottom_right, corner_pos.bottom_right);

  /* Frame tile sides */
  if (frame_size.x > 0) {
    i += BuildTileH<renderer::GeometryVertexLayout::Data>(
        &vertex[i * 4], border_src.top, frame_size.x, 16, 0);
    i += BuildTileH<renderer::GeometryVertexLayout::Data>(
        &vertex[i * 4], border_src.bottom, frame_size.x, 16, rect_.height - 16);
  }

  if (frame_size.y > 0) {
    i += BuildTileV<renderer::GeometryVertexLayout::Data>(
        &vertex[i * 4], border_src.left, frame_size.y, 0, 16);
    i += BuildTileV<renderer::GeometryVertexLayout::Data>(
        &vertex[i * 4], border_src.right, frame_size.y, rect_.width - 16, 16);
  }

  /* Update vertex buffer data */
  renderer_data_->base_tex_quad_array->Update();
}

void Window2::UpdateBaseTextureInternal(bgfx::Encoder* encoder,
                                        bgfx::ViewId* render_view) {
  if (rect_.width <= 0 || rect_.height <= 0 ||
      !IsObjectValid(windowskin_.get()))
    return;

  CalcBaseQuadArrayInternal();

  if (bgfx::isValid(base_tiled_texture_.handle))
    bgfx::destroy(base_tiled_texture_.handle);

  base_tiled_texture_.size = rect_.Size();
  base_tiled_texture_.handle = bgfx::createFrameBuffer(
      base_tiled_texture_.size.x, base_tiled_texture_.size.y,
      bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_RT);

  screen()->device()->BindRenderView(*render_view, base_tiled_texture_.size,
                                     base_tiled_texture_.handle, 0);

  {
    auto& shader = screen()->device()->pipelines().plane;

    base::Vec4 offset_size =
        base::MakeVec4(base::Vec2(), base::MakeInvert(windowskin_->GetSize()));
    encoder->setUniform(shader.OffsetTexSize(), &offset_size);

    base::Vec4 norm;
    encoder->setUniform(shader.Color(), &norm);

    norm = tone_->AsBase();
    encoder->setUniform(shader.Tone(), &norm);

    base::Vec4 uopacity;
    uopacity.x = back_opacity_ / 255.0f;
    encoder->setUniform(shader.Opacity(), &uopacity);

    encoder->setTexture(0, shader.Texture(),
                        bgfx::getTexture(windowskin_->GetHandle()));

    /* Draw stretch layer */
    encoder->setUniform(shader.OffsetTexSize(), &offset_size);
    encoder->setUniform(shader.Color(), &norm);

    norm = tone_->AsBase();
    encoder->setUniform(shader.Tone(), &norm);
    encoder->setUniform(shader.Opacity(), &uopacity);
    encoder->setTexture(0, shader.Texture(),
                        bgfx::getTexture(windowskin_->GetHandle()));

    encoder->setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
    renderer_data_->base_tex_quad_array->Draw(encoder, shader.GetProgram(), 0,
                                              1, *render_view);

    /* Background tiles */
    encoder->setState(
        BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A |
        renderer::MakeColorBlendState(renderer::BlendType::KeepDestAlpha));
    renderer_data_->base_tex_quad_array->Draw(
        encoder, shader.GetProgram(), 1, base_bg_tile_count_, *render_view);
  }

  {
    /* Frame corner and sides */
    auto& frame_shader = screen()->device()->pipelines().base;
    base::Vec4 offset_size =
        base::MakeVec4(base::Vec2(), base::MakeInvert(windowskin_->GetSize()));
    encoder->setUniform(frame_shader.OffsetTexSize(), &offset_size);

    encoder->setTexture(0, frame_shader.Texture(),
                        bgfx::getTexture(windowskin_->GetHandle()));

    encoder->setState(
        BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A |
        renderer::MakeColorBlendState(renderer::BlendType::Normal));
    renderer_data_->base_tex_quad_array->Draw(
        encoder, frame_shader.GetProgram(), 1 + base_bg_tile_count_,
        base_frame_tile_count_, *render_view);
  }

  // Next render pass
  (*render_view)++;
}

void Window2::UpdateBaseQuadInternal() {
  const float openness = openness_ / 255.0f;
  const base::Rect tex(0, 0, rect_.width, rect_.height);
  const base::Rect pos(0, (rect_.height / 2.0f) * (1.0f - openness),
                       rect_.width, rect_.height * openness);

  renderer_data_->base_quad->SetTexcoord(tex);
  renderer_data_->base_quad->SetPosition(pos);
  renderer_data_->base_quad->SetColor(base::Vec4(0, 0, 0, opacity_ / 255.0f));
}

void Window2::CalcArrowsQuadArrayInternal() {
  /* Scroll arrow position: Top Bottom X, Left Right Y */
  const base::Vec2i arrow_size =
      (rect_.Size() - base::Vec2i(16, 16)) / base::Vec2i(2, 2);

  const Sides<base::Rect> arrowPos = {
      base::Rect(4, arrow_size.y, 8, 16),                /* Left */
      base::Rect(rect_.width - 12, arrow_size.y, 8, 16), /* Right */
      base::Rect(arrow_size.x, 4, 16, 8),                /* Top */
      base::Rect(arrow_size.x, rect_.height - 12, 16, 8) /* Bottom */
  };

  size_t i = 0;
  renderer_data_->arrows_quads->Resize(5);
  renderer::GeometryVertexLayout::Data* vert =
      renderer_data_->arrows_quads->vertices().data();

  if (IsObjectValid(contents_.get()) && arrows_visible_) {
    if (ox_ > 0)
      i += renderer::GeometryVertexLayout::SetTexPos(
          &vert[i * 4], scroll_arrow_src.left, arrowPos.left);
    if (oy_ > 0)
      i += renderer::GeometryVertexLayout::SetTexPos(
          &vert[i * 4], scroll_arrow_src.top, arrowPos.top);

    if (padding_rect_.width < (contents_->GetSize().x - ox_))
      i += renderer::GeometryVertexLayout::SetTexPos(
          &vert[i * 4], scroll_arrow_src.right, arrowPos.right);
    if (padding_rect_.height < (contents_->GetSize().y - oy_))
      i += renderer::GeometryVertexLayout::SetTexPos(
          &vert[i * 4], scroll_arrow_src.bottom, arrowPos.bottom);
  }

  pause_vertex_ = nullptr;
  if (pause_) {
    const base::Rect pausePos(arrow_size.x, rect_.height - 16, 16, 16);
    pause_vertex_ = &vert[i * 4];

    i += renderer::GeometryVertexLayout::SetTexPos(&vert[i * 4], pause_src[0],
                                                   pausePos);
  }

  arrows_quad_count_ = i;
  renderer_data_->arrows_quads->Update();
}

void Window2::UpdateInternal() {
  if (active_)
    if (++cursor_alpha_index_ ==
        (sizeof(cursor_alpha) / sizeof(cursor_alpha[0])))
      cursor_alpha_index_ = 0;

  if (pause_) {
    if (pause_alpha_index_ <
        static_cast<int>(sizeof(pause_alpha) / sizeof(pause_alpha[0]) - 1))
      ++pause_alpha_index_;

    if (++pause_quad_index_ ==
        static_cast<int>(sizeof(pause_quad) / sizeof(pause_quad[0])))
      pause_quad_index_ = 0;
  }

  UpdatePauseStepInternal();
  cursor_step_need_update_ = true;
}

void Window2::ToneChangedInternal() {
  base_tex_need_update_ = true;
}

void Window2::CursorRectChangedInternal() {
  cursor_quad_need_update_ = true;
}

void Window2::WindowskinChangedInternal() {
  base_tex_need_update_ = true;
}

void Window2::UpdatePauseStepInternal() {
  if (pause_vertex_) {
    renderer::GeometryVertexLayout::SetTexcoord(
        pause_vertex_, pause_src[pause_quad[pause_quad_index_]]);
    renderer::GeometryVertexLayout::SetColor(
        pause_vertex_,
        base::Vec4(0, 0, 0, pause_alpha[pause_alpha_index_] / 255.0f));
    renderer_data_->arrows_quads->Update();
  }
}

void Window2::UpdateCursorQuadsInternal() {
  const base::Rect rect = cursor_rect_->AsBase();
  const CursorSrc& src = cursor_src;

  if (rect.width == 0 || rect.height == 0) {
    renderer_data_->cursor_quads->Clear();
    cursor_data_need_update_ = true;
    return;
  }

  const base::Vec2 corOff(rect.width - 4, rect.height - 4);

  const Corners<base::Rect> cornerPos = {
      base::Rect(0, 0, 4, 4),              /* Top left */
      base::Rect(corOff.x, 0, 4, 4),       /* Top right */
      base::Rect(0, corOff.y, 4, 4),       /* Bottom left */
      base::Rect(corOff.x, corOff.y, 4, 4) /* Bottom right */
  };

  const base::Vec2i sideLen(rect.width - 4 * 2, rect.height - 4 * 2);

  const Sides<base::Rect> sidePos = {
      base::Rect(0, 4, 4, sideLen.y),        /* Left */
      base::Rect(corOff.x, 4, 4, sideLen.y), /* Right */
      base::Rect(4, 0, sideLen.x, 4),        /* Top */
      base::Rect(4, corOff.y, sideLen.x, 4)  /* Bottom */
  };

  const base::Rect bgPos(4, 4, sideLen.x, sideLen.y);

  bool drawSidesLR = rect.height > 8;
  bool drawSidesTB = rect.width > 8;
  bool drawBg = drawSidesLR && drawSidesTB;

  size_t quads = 0;
  quads += 4; /* 4 corners */

  if (drawSidesLR)
    quads += 2;

  if (drawSidesTB)
    quads += 2;

  if (drawBg)
    quads += 1;

  renderer_data_->cursor_quads->Resize(quads);
  renderer::GeometryVertexLayout::Data* vert =
      renderer_data_->cursor_quads->vertices().data();
  size_t i = 0;

  i += renderer::GeometryVertexLayout::SetTexPos(
      &vert[i * 4], src.corners.top_left, cornerPos.top_left);
  i += renderer::GeometryVertexLayout::SetTexPos(
      &vert[i * 4], src.corners.top_right, cornerPos.top_right);
  i += renderer::GeometryVertexLayout::SetTexPos(
      &vert[i * 4], src.corners.bottom_left, cornerPos.bottom_left);
  i += renderer::GeometryVertexLayout::SetTexPos(
      &vert[i * 4], src.corners.bottom_right, cornerPos.bottom_right);

  if (drawSidesLR) {
    i += renderer::GeometryVertexLayout::SetTexPos(
        &vert[i * 4], src.border.left, sidePos.left);
    i += renderer::GeometryVertexLayout::SetTexPos(
        &vert[i * 4], src.border.right, sidePos.right);
  }

  if (drawSidesTB) {
    i += renderer::GeometryVertexLayout::SetTexPos(&vert[i * 4], src.border.top,
                                                   sidePos.top);
    i += renderer::GeometryVertexLayout::SetTexPos(
        &vert[i * 4], src.border.bottom, sidePos.bottom);
  }

  if (drawBg)
    renderer::GeometryVertexLayout::SetTexPos(&vert[i * 4], src.background,
                                              bgPos);

  cursor_data_need_update_ = true;
}

}  // namespace content
