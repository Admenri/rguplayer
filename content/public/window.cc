// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/window.h"

#include "content/common/tilemap_common.h"
#include "content/public/tilequad.h"

namespace content {

static const base::Rect kBackgroundSrc = {0, 0, 128, 128};

static const base::Rect kCursorSrc = {128, 64, 32, 32};

static const base::Rect kPauseAniSrc[] = {
    {160, 64, 16, 16},
    {176, 64, 16, 16},
    {160, 80, 16, 16},
    {176, 80, 16, 16},
};

static const Sides<base::Rect> kBordersSrc = {
    {128, 16, 16, 32},
    {176, 16, 16, 32},
    {144, 0, 32, 16},
    {144, 48, 32, 16},
};

static const Corners<base::Rect> kCornersSrc = {
    {128, 0, 16, 16},
    {176, 0, 16, 16},
    {128, 48, 16, 16},
    {176, 48, 16, 16},
};

static const Sides<base::Rect> kScrollArrowSrc = {
    {144, 24, 8, 16},
    {168, 24, 8, 16},
    {152, 16, 16, 8},
    {152, 40, 16, 8},
};

static const uint8_t kCursorAniAlpha[] = {
    /* Fade out */
    0xFF, 0xF7, 0xEF, 0xE7, 0xDF, 0xD7, 0xCF, 0xC7, 0xBF, 0xB7, 0xAF, 0xA7,
    0x9F, 0x97, 0x8F, 0x87,
    /* Fade in */
    0x7F, 0x87, 0x8F, 0x97, 0x9F, 0xA7, 0xAF, 0xB7, 0xBF, 0xC7, 0xCF, 0xD7,
    0xDF, 0xE7, 0xEF, 0xF7};

static const int kCursorAniAlphaSize =
    sizeof(kCursorAniAlpha) / sizeof(kCursorAniAlpha[0]);

static const uint8_t kPauseAniQuad[] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
                                        1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2,
                                        2, 2, 3, 3, 3, 3, 3, 3, 3, 3};

static const int kPauseAniQuadSize =
    sizeof(kPauseAniQuad) / sizeof(kPauseAniQuad[0]);

static const uint8_t kPauseAniAlpha[] = {0x00, 0x20, 0x40, 0x60, 0x80,
                                         0xA0, 0xC0, 0xE0, 0xFF};

static const int kPauseAniAlphaSize =
    sizeof(kPauseAniAlpha) / sizeof(kPauseAniAlpha[0]);

class WindowControlLayer : public ViewportChild {
 public:
  WindowControlLayer(base::WeakPtr<Window> window)
      : ViewportChild(window->screen(),
                      window->GetViewport(),
                      window->GetZ() + 2),
        window_(window) {}

  WindowControlLayer(const WindowControlLayer&) = delete;
  WindowControlLayer& operator=(const WindowControlLayer&) = delete;

  void InitDrawableData() override {}
  void BeforeComposite() override {}

  void Composite() override { window_->CompositeControls(); }

  void CheckDisposed() const override {}
  void OnViewportRectChanged(
      const DrawableParent::ViewportInfo& rect) override {}

  base::WeakPtr<Window> window_;
};

Window::Window(scoped_refptr<Graphics> screen, scoped_refptr<Viewport> viewport)
    : GraphicElement(screen),
      Disposable(screen),
      ViewportChild(screen, viewport) {
  InitWindow();
}

Window::~Window() {
  Dispose();
}

void Window::Update() {
  CheckIsDisposed();

  if (++cursor_alpha_index_ == kCursorAniAlphaSize)
    cursor_alpha_index_ = 0;

  if (pause_alpha_index_ < kPauseAniAlphaSize - 1)
    ++pause_alpha_index_;

  if (++pause_quad_index_ == kPauseAniQuadSize)
    pause_quad_index_ = 0;
  update_required_ = true;
}

void Window::SetWindowskin(scoped_refptr<Bitmap> windowskin) {
  CheckIsDisposed();

  if (windowskin_ == windowskin)
    return;
  windowskin_ = windowskin;
  base_tex_need_update_ = true;
}

void Window::SetContents(scoped_refptr<Bitmap> contents) {
  CheckIsDisposed();

  if (contents_ == contents)
    return;
  contents_ = contents;
  contents_quad_need_update_ = true;
}

void Window::SetStretch(bool stretch) {
  CheckIsDisposed();

  if (stretch_ == stretch)
    return;
  stretch_ = stretch;
  base_tex_need_update_ = true;
}

void Window::SetCursorRect(scoped_refptr<Rect> cursor_rect) {
  CheckIsDisposed();

  if (cursor_rect_->IsSame(*cursor_rect))
    return;
  *cursor_rect_ = *cursor_rect;
  controls_quads_need_update_ = true;
}

void Window::SetActive(bool active) {
  CheckIsDisposed();
  if (active_ == active)
    return;
  active_ = active;
  cursor_alpha_index_ = 0;
}

void Window::SetPause(bool pause) {
  CheckIsDisposed();

  if (pause_ == pause)
    return;
  pause_ = pause;
  pause_alpha_index_ = 0;
  pause_quad_index_ = 0;
  controls_quads_need_update_ = true;
}

void Window::SetX(int v) {
  CheckIsDisposed();

  if (v == rect_.x)
    return;
  rect_.x = v;
}

void Window::SetY(int v) {
  CheckIsDisposed();

  if (v == rect_.y)
    return;
  rect_.y = v;
}

void Window::SetWidth(int v) {
  CheckIsDisposed();

  if (v == rect_.width)
    return;
  rect_.width = v;
  base_tex_need_update_ = true;
  base_quad_need_update_ = true;
}

void Window::SetHeight(int v) {
  CheckIsDisposed();

  if (v == rect_.height)
    return;
  rect_.height = v;
  base_tex_need_update_ = true;
  base_quad_need_update_ = true;
}

void Window::SetOX(int v) {
  CheckIsDisposed();

  if (origin_.x == v)
    return;
  origin_.x = v;
  controls_quads_need_update_ = true;
}

void Window::SetOY(int v) {
  CheckIsDisposed();

  if (origin_.y == v)
    return;
  origin_.y = v;
  controls_quads_need_update_ = true;
}

void Window::SetOpacity(int v) {
  CheckIsDisposed();
  v = std::clamp(v, 0, 255);
  if (v == opacity_)
    return;

  opacity_ = v;
  base_quad_need_update_ = true;
}

void Window::SetBackOpacity(int v) {
  CheckIsDisposed();
  v = std::clamp(v, 0, 255);
  if (v == back_opacity_)
    return;

  back_opacity_ = v;
  base_tex_need_update_ = true;
}

void Window::SetContentsOpacity(int v) {
  CheckIsDisposed();
  v = std::clamp(v, 0, 255);
  if (v == contents_opacity_)
    return;

  contents_opacity_ = v;
  contents_quad_need_update_ = true;
}

void Window::SetZ(int v) {
  Drawable::SetZ(v);
  control_layer_->SetZ(v + 2);
}

void Window::SetVisible(bool visible) {
  Drawable::SetVisible(visible);
  control_layer_->SetVisible(visible);
}

void Window::SetViewport(scoped_refptr<Viewport> viewport) {
  ViewportChild::SetViewport(viewport);
  control_layer_->SetViewport(viewport);
  control_layer_->SetZ(GetZ() + 2);
}

void Window::OnObjectDisposed() {
  weak_ptr_factory_.InvalidateWeakPtrs();

  RemoveFromList();

  control_layer_.reset();
  base_quad_.reset();
  content_quad_.reset();
  controls_quads_.reset();
  base_tex_quad_array_.reset();

  renderer::TextureFrameBuffer::Del(base_tfb_);
}

void Window::InitDrawableData() {
  base_quad_.reset(new renderer::QuadDrawable());
  content_quad_.reset(new renderer::QuadDrawable());
  controls_quads_.reset(
      new renderer::QuadDrawableArray<renderer::CommonVertex>());
  base_tex_quad_array_.reset(
      new renderer::QuadDrawableArray<renderer::CommonVertex>());

  base_tfb_ = renderer::TextureFrameBuffer::Gen();
  renderer::TextureFrameBuffer::Alloc(base_tfb_, 32, 32);
  renderer::TextureFrameBuffer::LinkFrameBuffer(base_tfb_);

  controls_quads_->Resize(14);
}

void Window::BeforeComposite() {
  if (update_required_) {
    update_required_ = false;
    UpdateControlsInternal();
  }

  if (base_tex_need_update_) {
    base_tex_need_update_ = false;
    UpdateBaseTexInternal();
  }

  if (base_quad_need_update_) {
    base_quad_need_update_ = false;
    base_quad_->SetTexCoordRect(base::Rect(rect_.Size()));
    base_quad_->SetPositionRect(base::Rect(rect_.Size()));
    base_quad_->SetColor(-1, base::Vec4(0, 0, 0, opacity_ / 255.0f));
  }

  if (controls_quads_need_update_) {
    controls_quads_need_update_ = false;
    UpdateControlsQuadsInternal();
    UpdateControlsInternal();
  }

  if (contents_quad_need_update_) {
    contents_quad_need_update_ = false;
    if (contents_ && !contents_->IsDisposed()) {
      base::Rect size = contents_->GetSize();
      content_quad_->SetTexCoordRect(size);
      content_quad_->SetPositionRect(size);
      content_quad_->SetColor(-1,
                              base::Vec4(0, 0, 0, contents_opacity_ / 255.0f));
    }
  }
}

void Window::Composite() {
  if (!windowskin_ || windowskin_->IsDisposed())
    return;

  if (rect_.width <= 0 || rect_.height <= 0)
    return;

  auto offset = rect_.Position() + parent_rect().GetRealOffset();

  auto& shader = renderer::GSM.shaders->base_alpha;
  shader.Bind();
  shader.SetProjectionMatrix(renderer::GSM.states.viewport.Current().Size());
  shader.SetTexture(base_tfb_.tex);
  shader.SetTextureSize(base::Vec2i(base_tfb_.width, base_tfb_.height));
  shader.SetTransOffset(offset);
  renderer::Texture::SetFilter(GL_LINEAR);

  base_quad_->Draw();
  renderer::Texture::SetFilter();
}

void Window::OnViewportRectChanged(const DrawableParent::ViewportInfo& rect) {}

void Window::InitWindow() {
  control_layer_.reset(new WindowControlLayer(weak_ptr_factory_.GetWeakPtr()));

  cursor_rect_ = new Rect();
  cursor_rect_observer_ = cursor_rect_->AddChangedObserver(base::BindRepeating(
      &Window::CursorRectChangedInternal, base::Unretained(this)));
}

void Window::ResetBaseTexQuadsInternal() {
  base::Rect bg_rect(2, 2, rect_.width - 4, rect_.height - 4);

  Sides<base::Rect> border_rects;
  border_rects.left = base::Rect(0, 8, 16, rect_.height - 16);
  border_rects.right = base::Rect(rect_.width - 16, 8, 16, rect_.height - 16);
  border_rects.top = base::Rect(8, 0, rect_.width - 16, 16);
  border_rects.bottom = base::Rect(8, rect_.height - 16, rect_.width - 16, 16);

  Corners<base::Rect> corner_rects;
  corner_rects.top_left = base::Rect(0, 0, 16, 16);
  corner_rects.top_right = base::Rect(rect_.width - 16, 0, 16, 16);
  corner_rects.bottom_left = base::Rect(0, rect_.height - 16, 16, 16);
  corner_rects.bottom_right =
      base::Rect(rect_.width - 16, rect_.height - 16, 16, 16);

  int count = 0;
  if (stretch_)
    count += 1;
  else
    count += QuadTileCount2D(128, 128, bg_rect.width, bg_rect.height);
  count += QuadTileCount(32, rect_.width - 16) * 2;
  count += QuadTileCount(32, rect_.height - 16) * 2;
  count += 4;

  base_tex_quad_array_->Resize(count);
  auto* vert = base_tex_quad_array_->vertices().data();

  int i = 0;
  if (stretch_) {
    renderer::QuadSetColor(&vert[i * 4], -1,
                           base::Vec4(0, 0, 0, back_opacity_ / 255.0f));
    i += renderer::QuadSetTexPosRect(&vert[i * 4], kBackgroundSrc, bg_rect);
    background_quads_count_ = 1;
  } else {
    int quads = BuildTiles(&vert[i * 4], kBackgroundSrc, bg_rect);
    for (int j = 0; j < quads; ++j)
      renderer::QuadSetColor(&vert[j * 4], -1,
                             base::Vec4(0, 0, 0, back_opacity_ / 255.0f));
    i += quads;
    background_quads_count_ = quads;
  }

  i += BuildTileH(&vert[i * 4], kBordersSrc.top, rect_.width - 16, 8, 0);
  i += BuildTileH(&vert[i * 4], kBordersSrc.bottom, rect_.width - 16, 8,
                  rect_.height - 16);
  i += BuildTileV(&vert[i * 4], kBordersSrc.left, rect_.height - 16, 0, 8);
  i += BuildTileV(&vert[i * 4], kBordersSrc.right, rect_.height - 16,
                  rect_.width - 16, 8);

  i += renderer::QuadSetTexPosRect(&vert[i * 4], kCornersSrc.top_left,
                                   corner_rects.top_left);
  i += renderer::QuadSetTexPosRect(&vert[i * 4], kCornersSrc.top_right,
                                   corner_rects.top_right);
  i += renderer::QuadSetTexPosRect(&vert[i * 4], kCornersSrc.bottom_left,
                                   corner_rects.bottom_left);
  i += renderer::QuadSetTexPosRect(&vert[i * 4], kCornersSrc.bottom_right,
                                   corner_rects.bottom_right);
  base_tex_quad_array_->Update();
}

void Window::UpdateBaseTexInternal() {
  if (rect_.width <= 0 || rect_.height <= 0)
    return;

  ResetBaseTexQuadsInternal();

  renderer::TextureFrameBuffer::Alloc(base_tfb_, rect_.width, rect_.height);

  renderer::FrameBuffer::Bind(base_tfb_.fbo);
  renderer::GSM.states.clear_color.Push(base::Vec4());
  renderer::FrameBuffer::Clear();
  renderer::GSM.states.clear_color.Pop();

  if (!windowskin_ || windowskin_->IsDisposed())
    return;

  renderer::GSM.states.viewport.Push(rect_.Size());
  renderer::GSM.states.blend.Push(false);

  auto& shader = renderer::GSM.shaders->base_alpha;
  shader.Bind();
  shader.SetProjectionMatrix(renderer::GSM.states.viewport.Current().Size());
  shader.SetTexture(windowskin_->AsGLType().tex);
  shader.SetTextureSize(windowskin_->GetSize());
  shader.SetTransOffset(base::Vec2());
  renderer::Texture::SetFilter(GL_LINEAR);

  /* Draw stretch layer */
  base_tex_quad_array_->Draw(0, background_quads_count_);

  renderer::GSM.states.blend.Set(true);
  renderer::GSM.states.blend_func.Push(renderer::GLBlendType::Normal);

  base_tex_quad_array_->Draw(
      background_quads_count_,
      base_tex_quad_array_->count() - background_quads_count_);

  renderer::GSM.states.blend_func.Pop();
  renderer::GSM.states.viewport.Pop();
  renderer::GSM.states.blend.Pop();
  renderer::Texture::SetFilter();
}

void Window::UpdateControlsQuadsInternal() {
  auto build_frame_internal = [](const base::Rect& rect,
                                 base::RectF quad_rects[9]) {
    int w = rect.width;
    int h = rect.height;
    int x1 = rect.x;
    int x2 = x1 + w;
    int y1 = rect.y;
    int y2 = y1 + h;

    int i = 0;
    quad_rects[i++] = base::Rect(x1, y1, 2, 2);
    quad_rects[i++] = base::Rect(x2 - 2, y1, 2, 2);
    quad_rects[i++] = base::Rect(x2 - 2, y2 - 2, 2, 2);
    quad_rects[i++] = base::Rect(x1, y2 - 2, 2, 2);

    quad_rects[i++] = base::Rect(x1, y1 + 2, 2, h - 4);
    quad_rects[i++] = base::Rect(x2 - 2, y1 + 2, 2, h - 4);
    quad_rects[i++] = base::Rect(x1 + 2, y1, w - 4, 2);
    quad_rects[i++] = base::Rect(x1 + 2, y2 - 2, w - 4, 2);

    quad_rects[i++] = base::Rect(x1 + 2, y1 + 2, w - 4, h - 4);
  };

  auto build_frame_quads_src = [&](const base::Rect& rect,
                                   renderer::CommonVertex vert[36]) {
    base::RectF quad_rects[9];
    build_frame_internal(rect, quad_rects);
    for (int i = 0; i < 9; ++i)
      renderer::QuadSetTexCoordRect(&vert[i * 4], quad_rects[i]);
    return 9;
  };

  auto build_frame_quads_pos = [&](const base::Rect& rect,
                                   renderer::CommonVertex vert[36]) {
    base::RectF quad_rects[9];
    build_frame_internal(rect, quad_rects);
    for (int i = 0; i < 9; ++i)
      renderer::QuadSetPositionRect(&vert[i * 4], quad_rects[i]);
    return 9;
  };

  int i = 0;
  auto* vert = controls_quads_->vertices().data();

  cursor_vertex_ = nullptr;
  auto cur_rect = cursor_rect_->AsBase();
  if (cur_rect.width > 0 && cur_rect.height > 0) {
    base::Rect effect_rect(cur_rect.x + 16, cur_rect.y + 16, cur_rect.width,
                           cur_rect.height);
    cursor_vertex_ = &vert[i * 4];
    build_frame_quads_src(kCursorSrc, cursor_vertex_);
    i += build_frame_quads_pos(effect_rect, cursor_vertex_);
  }

  const base::Vec2i scroll =
      (rect_.Size() - base::Vec2i(16, 16)) / base::Vec2i(2, 2);
  Sides<base::Rect> scroll_arrows;
  scroll_arrows.left = base::Rect(4, scroll.y, 8, 16);
  scroll_arrows.right = base::Rect(rect_.width - 12, scroll.y, 8, 16);
  scroll_arrows.top = base::Rect(scroll.x, 4, 16, 8);
  scroll_arrows.bottom = base::Rect(scroll.x, rect_.height - 12, 16, 8);

  if (contents_ && !contents_->IsDisposed()) {
    if (origin_.x > 0)
      i += renderer::QuadSetTexPosRect(&vert[i * 4], kScrollArrowSrc.left,
                                       scroll_arrows.left);

    if (origin_.y > 0)
      i += renderer::QuadSetTexPosRect(&vert[i * 4], kScrollArrowSrc.top,
                                       scroll_arrows.top);

    if ((rect_.width - 32) < (contents_->GetWidth() - origin_.x))
      i += renderer::QuadSetTexPosRect(&vert[i * 4], kScrollArrowSrc.right,
                                       scroll_arrows.right);

    if ((rect_.height - 32) < (contents_->GetHeight() - origin_.y))
      i += renderer::QuadSetTexPosRect(&vert[i * 4], kScrollArrowSrc.bottom,
                                       scroll_arrows.bottom);
  }

  pause_vertex_ = nullptr;
  if (pause_) {
    pause_vertex_ = &vert[i * 4];
    i += renderer::QuadSetTexPosRect(
        &vert[i * 4], kPauseAniSrc[kPauseAniQuad[pause_quad_index_]],
        base::RectF((rect_.width - 16) / 2, rect_.height - 16, 16, 16));
  }

  controls_quads_->Update();
  controls_quad_count = i;
}

void Window::UpdateControlsInternal() {
  bool update_buffer = false;
  if (active_ && cursor_vertex_) {
    float alpha = kCursorAniAlpha[cursor_alpha_index_] / 255.0f;
    for (int i = 0; i < 9; ++i)
      renderer::QuadSetColor(cursor_vertex_ + 4 * i, -1,
                             base::Vec4(0, 0, 0, alpha));
    update_buffer = true;
  }

  if (pause_ && pause_vertex_) {
    float alpha = kPauseAniAlpha[pause_alpha_index_] / 255.0f;
    base::RectF frameRect = kPauseAniSrc[kPauseAniQuad[pause_quad_index_]];
    renderer::QuadSetColor(pause_vertex_, -1, base::Vec4(0, 0, 0, alpha));
    renderer::QuadSetTexCoordRect(pause_vertex_, frameRect);
    update_buffer = true;
  }

  if (update_buffer)
    controls_quads_->Update();
}

void Window::CursorRectChangedInternal() {
  controls_quads_need_update_ = true;
}

void Window::CompositeControls() {
  if ((!windowskin_ || windowskin_->IsDisposed()) &&
      (!contents_ || contents_->IsDisposed()))
    return;

  if (rect_.width <= 0 || rect_.height <= 0)
    return;

  base::Vec2i offset = rect_.Position() + parent_rect().GetRealOffset();
  const base::Rect window_rect(offset, rect_.Size());
  const base::Rect contents_rect(offset + base::Vec2i(16, 16),
                                 rect_.Size() - base::Vec2i(32, 32));

  renderer::GSM.states.scissor.Push(true);
  renderer::GSM.states.scissor_rect.PushOnly();
  renderer::GSM.states.scissor_rect.SetIntersect(window_rect);

  auto& shader = renderer::GSM.shaders->base_alpha;
  shader.Bind();
  shader.SetProjectionMatrix(renderer::GSM.states.viewport.Current().Size());

  if (windowskin_ && !windowskin_->IsDisposed()) {
    shader.SetTransOffset(offset);
    shader.SetTexture(windowskin_->AsGLType().tex);
    shader.SetTextureSize(windowskin_->GetSize());
    renderer::Texture::SetFilter(GL_LINEAR);

    controls_quads_->Draw(0, controls_quad_count);
    renderer::Texture::SetFilter();
  }

  if (contents_ && !contents_->IsDisposed()) {
    renderer::GSM.states.scissor_rect.SetIntersect(contents_rect);

    shader.SetTransOffset(offset + (base::Vec2i(16, 16) - origin_));
    shader.SetTexture(contents_->AsGLType().tex);
    shader.SetTextureSize(contents_->GetSize());

    content_quad_->Draw();
  }

  renderer::GSM.states.scissor_rect.Pop();
  renderer::GSM.states.scissor.Pop();
}

}  // namespace content
