// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/window2.h"

#include "content/common/tilemap_common.h"
#include "content/public/tilequad.h"

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
    : GraphicElement(screen),
      Disposable(screen),
      ViewportChild(screen,
                    viewport,
                    (screen->content_version() >= RGSSVersion::RGSS3 ? 100 : 0),
                    (screen->content_version() >= RGSSVersion::RGSS3
                         ? std::numeric_limits<int>::max()
                         : 0)),
      rect_(0, 0, 0, 0) {
  InitWindow();
}

Window2::Window2(scoped_refptr<Graphics> screen,
                 int x,
                 int y,
                 int width,
                 int height)
    : GraphicElement(screen),
      Disposable(screen),
      ViewportChild(screen,
                    nullptr,
                    (screen->content_version() >= RGSSVersion::RGSS3 ? 100 : 0),
                    (screen->content_version() >= RGSSVersion::RGSS3
                         ? std::numeric_limits<int>::max()
                         : 0)),
      rect_(x, y, width, height) {
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

  if (windowskin_ && !windowskin_->IsDisposed())
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

  base_quad_.reset();
  content_quad_.reset();
  arrows_quads_.reset();
  cursor_quads_.reset();
  base_tex_quad_array_.reset();

  renderer::TextureFrameBuffer::Del(base_tfb_);
}

void Window2::InitDrawableData() {
  base_quad_.reset(new renderer::QuadDrawable());
  content_quad_.reset(new renderer::QuadDrawable());
  arrows_quads_.reset(
      new renderer::QuadDrawableArray<renderer::CommonVertex>());
  cursor_quads_.reset(
      new renderer::QuadDrawableArray<renderer::CommonVertex>());
  base_tex_quad_array_.reset(
      new renderer::QuadDrawableArray<renderer::CommonVertex>());

  arrows_quads_->Resize(5);
  pause_vertex_ = nullptr;
  cursor_quads_->Resize(1);

  base_tfb_ = renderer::TextureFrameBuffer::Gen();
  renderer::TextureFrameBuffer::Alloc(base_tfb_, rect_.width, rect_.height);
  renderer::TextureFrameBuffer::LinkFrameBuffer(base_tfb_);

  contents_quad_need_update_ = true;

  /*
   * Stretch layer
   * Frame layer
   * Cursor layer
   * Control layer
   * Contents layer
   */
}

void Window2::BeforeComposite() {
  if (update_required_) {
    update_required_ = false;
    UpdateInternal();
  }

  padding_rect_ =
      base::Rect(padding_, padding_, std::max(0, rect_.width - padding_ * 2),
                 std::max(0, rect_.height - (padding_ + padding_bottom_)));

  if (base_tex_need_update_) {
    base_tex_need_update_ = false;
    UpdateBaseTextureInternal();
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
    if (cursor_quads_->count() > 0) {
      base::Vec4 color(0, 0, 0, cursor_alpha[cursor_alpha_index_] / 255.0f);
      for (size_t i = 0; i < cursor_quads_->count(); ++i)
        renderer::QuadSetColor(&cursor_quads_->vertices()[i * 4], -1, color);
      cursor_data_need_update_ = true;
    }
  }

  if (contents_quad_need_update_) {
    contents_quad_need_update_ = false;
    if (contents_ && !contents_->IsDisposed()) {
      base::Rect contents_rect = contents_->GetSize();
      content_quad_->SetTexCoordRect(contents_rect);
      content_quad_->SetPositionRect(contents_rect);
      content_quad_->SetColor(
          -1,
          base::Vec4(0, 0, 0, static_cast<float>(contents_opacity_) / 255.0f));
      cursor_data_need_update_ = true;
    }
  }

  if (cursor_data_need_update_) {
    cursor_data_need_update_ = false;
    cursor_quads_->Update();
  }
}

void Window2::Composite() {
  bool windowskin_valid = windowskin_ && !windowskin_->IsDisposed();
  bool contents_valid = contents_ && !contents_->IsDisposed();

  base::Vec2i trans_offset = rect_.Position() + parent_rect().GetRealOffset();

  renderer::GSM.states.scissor.Push(true);
  renderer::GSM.states.scissor_rect.PushOnly();
  renderer::GSM.states.scissor_rect.SetIntersect(
      base::Rect(trans_offset, rect_.Size()));

  /* Stretch background & frame */
  auto& shader = renderer::GSM.shaders->base_alpha;
  shader.Bind();
  shader.SetProjectionMatrix(renderer::GSM.states.viewport.Current().Size());
  shader.SetTransOffset(trans_offset);
  renderer::Texture::SetFilter(GL_LINEAR);

  if (windowskin_valid) {
    /* Base window draw */
    if (opacity_ > 0) {
      shader.SetTexture(base_tfb_.tex);
      shader.SetTextureSize(rect_.Size());

      base_quad_->Draw();
    }

    /* Controls draw */
    if (openness_ >= 255) {
      shader.SetTexture(windowskin_->AsGLType().tex);
      shader.SetTextureSize(windowskin_->GetSize());

      if (arrows_quad_count_)
        arrows_quads_->Draw(0, arrows_quad_count_);
    }
  }

  if (openness_ < 255) {
    renderer::GSM.states.scissor.Pop();
    renderer::GSM.states.scissor_rect.Pop();
    renderer::Texture::SetFilter();
    return;
  }

  base::Rect padding_trans_rect = padding_rect_;
  padding_trans_rect.x += trans_offset.x;
  padding_trans_rect.y += trans_offset.y;

  if (screen()->content_version() >= RGSSVersion::RGSS3)
    renderer::GSM.states.scissor_rect.SetIntersect(padding_trans_rect);

  /* Cursor draw */
  if (cursor_quads_->count() > 0 && windowskin_valid) {
    base::Vec2i cursor_trans = padding_trans_rect.Position();
    cursor_trans.x += cursor_rect_->GetX();
    cursor_trans.y += cursor_rect_->GetY();

    if (screen()->content_version() >= RGSSVersion::RGSS3)
      cursor_trans = cursor_trans - base::Vec2i(ox_, oy_);
    shader.SetTransOffset(cursor_trans);

    cursor_quads_->Draw();
  }

  renderer::Texture::SetFilter();

  /* Window contents */
  if (contents_valid && contents_opacity_ > 0) {
    if (screen()->content_version() < RGSSVersion::RGSS3)
      renderer::GSM.states.scissor_rect.SetIntersect(padding_trans_rect);

    base::Vec2i content_trans = padding_trans_rect.Position();
    content_trans = content_trans - base::Vec2i(ox_, oy_);

    shader.SetTransOffset(content_trans);
    shader.SetTexture(contents_->AsGLType().tex);
    shader.SetTextureSize(contents_->GetSize());

    content_quad_->Draw();
  }

  renderer::GSM.states.scissor_rect.Pop();
  renderer::GSM.states.scissor.Pop();
}

void Window2::CheckDisposed() const {
  CheckIsDisposed();
}

void Window2::OnViewportRectChanged(const DrawableParent::ViewportInfo& rect) {
  // Passed on composite trans calculate
}

void Window2::InitWindow() {
  padding_ = screen()->content_version() >= RGSSVersion::RGSS3 ? 12 : 16;
  back_opacity_ = screen()->content_version() >= RGSSVersion::RGSS3 ? 192 : 255;

  tone_ = new Tone();
  tone_observer_ = tone_->AddChangedObserver(base::BindRepeating(
      &Window2::ToneChangedInternal, base::Unretained(this)));

  contents_ = new Bitmap(screen(), 1, 1);

  cursor_rect_ = new Rect();
  cursor_rect_observer_ = cursor_rect_->AddChangedObserver(base::BindRepeating(
      &Window2::CursorRectChangedInternal, base::Unretained(this)));
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
  base_tex_quad_array_->Resize(quad_count);

  /* Fill vertex data */
  renderer::CommonVertex* vertex = base_tex_quad_array_->vertices().data();

  int i = 0;
  /* Stretch background */
  i += renderer::QuadSetTexPosRect(&vertex[i * 4], background_stretch_src,
                                   background_rect);

  /* Tiled background */
  i += BuildTiles<renderer::CommonVertex>(&vertex[i * 4], background_tile_src,
                                          background_rect);

  /* Frame Corners */
  const Corners<base::Rect> corner_pos = {
      base::Rect(0, 0, 16, 16),                               /* Top left */
      base::Rect(rect_.width - 16, 0, 16, 16),                /* Top right */
      base::Rect(0, rect_.height - 16, 16, 16),               /* Bottom left */
      base::Rect(rect_.width - 16, rect_.height - 16, 16, 16) /* Bottom right */
  };

  i += renderer::QuadSetTexPosRect(&vertex[i * 4], corner_src.top_left,
                                   corner_pos.top_left);
  i += renderer::QuadSetTexPosRect(&vertex[i * 4], corner_src.top_right,
                                   corner_pos.top_right);
  i += renderer::QuadSetTexPosRect(&vertex[i * 4], corner_src.bottom_left,
                                   corner_pos.bottom_left);
  i += renderer::QuadSetTexPosRect(&vertex[i * 4], corner_src.bottom_right,
                                   corner_pos.bottom_right);

  /* Frame tile sides */
  if (frame_size.x > 0) {
    i += BuildTileH<renderer::CommonVertex>(&vertex[i * 4], border_src.top,
                                            frame_size.x, 16, 0);
    i += BuildTileH<renderer::CommonVertex>(
        &vertex[i * 4], border_src.bottom, frame_size.x, 16, rect_.height - 16);
  }

  if (frame_size.y > 0) {
    i += BuildTileV<renderer::CommonVertex>(&vertex[i * 4], border_src.left,
                                            frame_size.y, 0, 16);
    i += BuildTileV<renderer::CommonVertex>(&vertex[i * 4], border_src.right,
                                            frame_size.y, rect_.width - 16, 16);
  }

  /* Update vertex buffer data */
  base_tex_quad_array_->Update();
}

void Window2::UpdateBaseTextureInternal() {
  if (rect_.width <= 0 || rect_.height <= 0)
    return;

  CalcBaseQuadArrayInternal();

  renderer::TextureFrameBuffer::Alloc(base_tfb_, rect_.width, rect_.height);

  renderer::FrameBuffer::Bind(base_tfb_.fbo);
  renderer::GSM.states.clear_color.Push(base::Vec4());
  renderer::FrameBuffer::Clear();
  renderer::GSM.states.clear_color.Pop();

  if (!windowskin_ || windowskin_->IsDisposed())
    return;

  renderer::GSM.states.viewport.Push(rect_.Size());
  renderer::GSM.states.blend.Push(false);

  auto& shader = renderer::GSM.shaders->plane;

  shader.Bind();
  shader.SetProjectionMatrix(renderer::GSM.states.viewport.Current().Size());
  shader.SetTone(tone_->AsBase());
  shader.SetColor(base::Vec4());
  shader.SetOpacity(back_opacity_ / 255.0f);
  shader.SetTexture(windowskin_->AsGLType().tex);
  shader.SetTextureSize(windowskin_->GetSize());
  shader.SetTransOffset(base::Vec2());
  renderer::Texture::SetFilter(GL_LINEAR);

  /* Draw stretch layer */
  base_tex_quad_array_->Draw(0, 1);

  renderer::GSM.states.blend.Set(true);
  renderer::GSM.states.blend_func.Push(renderer::GLBlendType::KeepDestAlpha);

  /* Background tiles */
  base_tex_quad_array_->Draw(1, base_bg_tile_count_);

  renderer::GSM.states.blend_func.Set(renderer::GLBlendType::Normal);

  auto& frame_shader = renderer::GSM.shaders->base;
  frame_shader.Bind();
  frame_shader.SetProjectionMatrix(
      renderer::GSM.states.viewport.Current().Size());
  frame_shader.SetTexture(windowskin_->AsGLType().tex);
  frame_shader.SetTextureSize(windowskin_->GetSize());
  frame_shader.SetTransOffset(base::Vec2());

  /* Frame corner and sides */
  base_tex_quad_array_->Draw(1 + base_bg_tile_count_, base_frame_tile_count_);

  renderer::Texture::SetFilter();
  renderer::GSM.states.blend_func.Pop();
  renderer::GSM.states.viewport.Pop();
  renderer::GSM.states.blend.Pop();
}

void Window2::UpdateBaseQuadInternal() {
  const float openness = openness_ / 255.0f;
  const base::Rect tex(0, 0, rect_.width, rect_.height);
  const base::Rect pos(0, (rect_.height / 2.0f) * (1.0f - openness),
                       rect_.width, rect_.height * openness);

  base_quad_->SetTexCoordRect(tex);
  base_quad_->SetPositionRect(pos);
  base_quad_->SetColor(-1, base::Vec4(0, 0, 0, opacity_ / 255.0f));
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
  arrows_quads_->Resize(5);
  renderer::CommonVertex* vert = arrows_quads_->vertices().data();

  if (contents_ && !contents_->IsDisposed() && arrows_visible_) {
    if (ox_ > 0)
      i += renderer::QuadSetTexPosRect(&vert[i * 4], scroll_arrow_src.left,
                                       arrowPos.left);
    if (oy_ > 0)
      i += renderer::QuadSetTexPosRect(&vert[i * 4], scroll_arrow_src.top,
                                       arrowPos.top);

    if (padding_rect_.width < (contents_->GetWidth() - ox_))
      i += renderer::QuadSetTexPosRect(&vert[i * 4], scroll_arrow_src.right,
                                       arrowPos.right);
    if (padding_rect_.height < (contents_->GetHeight() - oy_))
      i += renderer::QuadSetTexPosRect(&vert[i * 4], scroll_arrow_src.bottom,
                                       arrowPos.bottom);
  }

  pause_vertex_ = nullptr;
  if (pause_) {
    const base::Rect pausePos(arrow_size.x, rect_.height - 16, 16, 16);
    pause_vertex_ = &vert[i * 4];

    i += renderer::QuadSetTexPosRect(&vert[i * 4], pause_src[0], pausePos);
  }

  arrows_quad_count_ = i;
  arrows_quads_->Update();
}

void Window2::UpdateInternal() {
  if (active_)
    if (++cursor_alpha_index_ ==
        (sizeof(cursor_alpha) / sizeof(cursor_alpha[0])))
      cursor_alpha_index_ = 0;

  if (pause_) {
    if (pause_alpha_index_ < (sizeof(pause_alpha) / sizeof(pause_alpha[0]) - 1))
      ++pause_alpha_index_;

    if (++pause_quad_index_ == (sizeof(pause_quad) / sizeof(pause_quad[0])))
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
    renderer::QuadSetTexCoordRect(pause_vertex_,
                                  pause_src[pause_quad[pause_quad_index_]]);
    renderer::QuadSetColor(
        pause_vertex_, -1,
        base::Vec4(0, 0, 0, pause_alpha[pause_alpha_index_] / 255.0f));
    arrows_quads_->Update();
  }
}

void Window2::UpdateCursorQuadsInternal() {
  const base::Rect rect = cursor_rect_->AsBase();
  const CursorSrc& src = cursor_src;

  if (rect.width == 0 || rect.height == 0) {
    cursor_quads_->Clear();
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

  cursor_quads_->Resize(quads);
  renderer::CommonVertex* vert = cursor_quads_->vertices().data();
  size_t i = 0;

  i += renderer::QuadSetTexPosRect(&vert[i * 4], src.corners.top_left,
                                   cornerPos.top_left);
  i += renderer::QuadSetTexPosRect(&vert[i * 4], src.corners.top_right,
                                   cornerPos.top_right);
  i += renderer::QuadSetTexPosRect(&vert[i * 4], src.corners.bottom_left,
                                   cornerPos.bottom_left);
  i += renderer::QuadSetTexPosRect(&vert[i * 4], src.corners.bottom_right,
                                   cornerPos.bottom_right);

  if (drawSidesLR) {
    i += renderer::QuadSetTexPosRect(&vert[i * 4], src.border.left,
                                     sidePos.left);
    i += renderer::QuadSetTexPosRect(&vert[i * 4], src.border.right,
                                     sidePos.right);
  }

  if (drawSidesTB) {
    i += renderer::QuadSetTexPosRect(&vert[i * 4], src.border.top, sidePos.top);
    i += renderer::QuadSetTexPosRect(&vert[i * 4], src.border.bottom,
                                     sidePos.bottom);
  }

  if (drawBg)
    renderer::QuadSetTexPosRect(&vert[i * 4], src.background, bgPos);

  cursor_data_need_update_ = true;
}

}  // namespace content
