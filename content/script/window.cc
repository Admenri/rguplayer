#include "content/script/window.h"

#include "content/binding/binding_runner.h"
#include "content/script/tilequad.h"

namespace content {

template <typename T>
struct Sides {
  T left, right, top, bottom;
};

template <typename T>
struct Corners {
  T top_left, top_right, bottom_left, bottom_right;
};

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

Window2::Window2(scoped_refptr<Viewport> viewport) : ViewportChild(viewport) {
  InitWindow();
}

Window2::Window2(int x, int y, int width, int height)
    : ViewportChild(nullptr), rect_(x, y, width, height) {
  InitWindow();
}

Window2::~Window2() { Dispose(); }

void Window2::Update() {}

void Window2::Move(int x, int y, int width, int height) {
  CheckIsDisposed();

  if (rect_ == base::Rect(x, y, width, height)) return;

  rect_ = base::Rect(x, y, width, height);
  base_layer_.base_tex_updated_ = true;
  base_quad_updated_ = true;
}

void Window2::SetWindowskin(scoped_refptr<Bitmap> windowskin) {
  CheckIsDisposed();

  if (windowskin_ == windowskin) return;

  windowskin_ = windowskin;
  base_layer_.base_tex_updated_ = true;
}

void Window2::SetContents(scoped_refptr<Bitmap> contents) {}

void Window2::SetCursorRect(scoped_refptr<Rect> cursor_rect) {}

void Window2::SetActive(bool active) {}

void Window2::SetVisible(bool visible) {}

void Window2::SetArrowsVisible(bool arrows_visible) {}

void Window2::SetPause(bool pause) {}

void Window2::SetX(int x) {}

void Window2::SetY(int y) {}

void Window2::SetWidth(int width) {
  CheckIsDisposed();

  if (rect_.width == width) return;

  rect_.width = width;
  base_layer_.base_tex_updated_ = true;
}

void Window2::SetHeight(int height) {
  CheckIsDisposed();

  if (rect_.height == height) return;

  rect_.height = height;
  base_layer_.base_tex_updated_ = true;
}

void Window2::SetOX(int ox) {}

void Window2::SetOY(int oy) {}

void Window2::SetPadding(int padding) {}

void Window2::SetPaddingBottom(int padding_bottom) {}

void Window2::SetOpacity(int opacity) {}

void Window2::SetBackOpacity(int back_opacity) {
  CheckIsDisposed();

  back_opacity = std::clamp(back_opacity, 0, 255);

  if (back_opacity_ == back_opacity) return;

  back_opacity_ = std::clamp(back_opacity, 0, 255);
  base_layer_.base_tex_updated_ = true;
}

void Window2::SetContentsOpacity(int contents_opacity) {}

void Window2::SetOpenness(int openness) {
  CheckIsDisposed();

  openness = std::clamp(openness, 0, 255);

  if (openness_ == openness) return;

  openness_ = std::clamp(openness, 0, 255);
  base_quad_updated_ = true;
}

void Window2::SetTone(scoped_refptr<Tone> tone) {
  CheckIsDisposed();

  if (tone_ == tone) return;

  tone_ = tone;
  base_layer_.base_tex_updated_ = true;
}

void Window2::OnObjectDisposed() {
  weak_ptr_factory_.InvalidateWeakPtrs();

  BindingRunner::Get()->GetRenderer()->DeleteSoon(
      std::move(base_layer_.quad_array_));

  BindingRunner::Get()->GetRenderer()->DeleteSoon(std::move(base_quad_));

  BindingRunner::Get()->GetRenderer()->PostTask(
      base::BindOnce(&gpu::TextureFrameBuffer::Del,
                     base::OwnedRef(std::move(base_layer_.tfb_))));
}

void Window2::BeforeComposite() {
  if (base_layer_.base_tex_updated_) {
    UpdateBaseTextureInternal();
    base_layer_.base_tex_updated_ = false;
  }

  if (base_quad_updated_) {
    UpdateBaseQuadInternal();
    base_quad_updated_ = false;
  }
}

void Window2::Composite() {
  bool windowskin_valid = !(!windowskin_ || windowskin_->IsDisposed());
  bool contents_valid = !(!contents_ || contents_->IsDisposed());

  base::Vec2i trans_offset = rect_.Position() + parent_rect().GetRealOffset();

  if (windowskin_valid) {
    auto& shader = gpu::GSM.shaders->base;

    shader.Bind();
    shader.SetProjectionMatrix(gpu::GSM.states.viewport.Current().Size());

    shader.SetTransOffset(trans_offset);
    shader.SetTexture(base_layer_.tfb_.tex);
    shader.SetTextureSize(rect_.Size());

    base_quad_->Draw();
  }
}

void Window2::CheckDisposed() const { CheckIsDisposed(); }

void Window2::OnViewportRectChanged(const DrawableParent::ViewportInfo& rect) {}

void Window2::InitWindow() {
  tone_ = new Tone();
  tone_observer_ = tone_->AddChangedObserver(base::BindRepeating(
      &Window2::ToneChangedInternal, weak_ptr_factory_.GetWeakPtr()));

  BindingRunner::Get()->GetRenderer()->PostTask(base::BindOnce(
      &Window2::InitWindowInternal, weak_ptr_factory_.GetWeakPtr()));
}

void Window2::InitWindowInternal() {
  base_layer_.quad_array_ =
      std::make_unique<gpu::QuadDrawableArray<gpu::CommonVertex>>();
  base_quad_ = std::make_unique<gpu::QuadDrawable>();

  base_layer_.tfb_ = gpu::TextureFrameBuffer::Gen();
  gpu::TextureFrameBuffer::Alloc(base_layer_.tfb_, rect_.width, rect_.height);
  gpu::TextureFrameBuffer::LinkFrameBuffer(base_layer_.tfb_);

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
  base_layer_.bg_tile_count_ =
      QuadTileCount2D(frame_tile_src.width, frame_tile_src.height,
                      background_rect.width, background_rect.height);
  quad_count += base_layer_.bg_tile_count_;

  /* Frame layer */
  base_layer_.frame_tile_count_ = 4;
  /* Frame sides */
  const base::Vec2i frame_size{rect_.width - 16 * 2, rect_.height - 16 * 2};
  if (frame_size.x > 0) {
    base_layer_.frame_tile_count_ += QuadTileCount(32, frame_size.x) * 2;
  }
  if (frame_size.y > 0) {
    base_layer_.frame_tile_count_ += QuadTileCount(32, frame_size.y) * 2;
  }
  quad_count += base_layer_.frame_tile_count_;

  /* Set vertex data */
  base_layer_.quad_array_->Resize(quad_count);

  /* Fill vertex data */
  gpu::CommonVertex* vertex = &base_layer_.quad_array_->vertices()[0];

  int i = 0;
  /* Stretch background */
  i += gpu::QuadSetTexPosRect(&vertex[i * 4], background_stretch_src,
                              background_rect);

  /* Tiled background */
  i += BuildTiles<gpu::CommonVertex>(&vertex[i * 4], background_tile_src,
                                     background_rect);

  /* Frame Corners */
  const Corners<base::Rect> corner_pos = {
      base::Rect(0, 0, 16, 16),                               /* Top left */
      base::Rect(rect_.width - 16, 0, 16, 16),                /* Top right */
      base::Rect(0, rect_.height - 16, 16, 16),               /* Bottom left */
      base::Rect(rect_.width - 16, rect_.height - 16, 16, 16) /* Bottom right */
  };

  i += gpu::QuadSetTexPosRect(&vertex[i * 4], corner_src.top_left,
                              corner_pos.top_left);
  i += gpu::QuadSetTexPosRect(&vertex[i * 4], corner_src.top_right,
                              corner_pos.top_right);
  i += gpu::QuadSetTexPosRect(&vertex[i * 4], corner_src.bottom_left,
                              corner_pos.bottom_left);
  i += gpu::QuadSetTexPosRect(&vertex[i * 4], corner_src.bottom_right,
                              corner_pos.bottom_right);

  /* Frame tile sides */
  if (frame_size.x > 0) {
    i += BuildTileH<gpu::CommonVertex>(&vertex[i * 4], border_src.top,
                                       frame_size.x, 16, 0);
    i += BuildTileH<gpu::CommonVertex>(&vertex[i * 4], border_src.bottom,
                                       frame_size.x, 16, rect_.height - 16);
  }

  if (frame_size.y > 0) {
    i += BuildTileV<gpu::CommonVertex>(&vertex[i * 4], border_src.left,
                                       frame_size.y, 0, 16);
    i += BuildTileV<gpu::CommonVertex>(&vertex[i * 4], border_src.right,
                                       frame_size.y, rect_.width - 16, 16);
  }

  /* Update vertex buffer data */
  base_layer_.quad_array_->Update();
}

void Window2::UpdateBaseTextureInternal() {
  if (!windowskin_ || windowskin_->IsDisposed()) return;

  CalcBaseQuadArrayInternal();

  gpu::TextureFrameBuffer::Alloc(base_layer_.tfb_, rect_.width, rect_.height);

  gpu::FrameBuffer::Bind(base_layer_.tfb_.fbo);
  gpu::GSM.states.clear_color.Push(base::Vec4());
  gpu::FrameBuffer::Clear();
  gpu::GSM.states.clear_color.Pop();

  gpu::GSM.states.viewport.Push(rect_.Size());
  gpu::GSM.states.blend.Push(false);

  auto& shader = gpu::GSM.shaders->plane;

  shader.Bind();
  shader.SetProjectionMatrix(gpu::GSM.states.viewport.Current().Size());
  shader.SetTone(tone_->AsBase());
  shader.SetColor(base::Vec4());
  shader.SetOpacity(back_opacity_ / 255.0f);

  shader.SetTexture(windowskin_->AsGLType().tex);
  shader.SetTextureSize(windowskin_->GetSize());

  shader.SetTransOffset(base::Vec2());

  /* Draw stretch layer */
  base_layer_.quad_array_->Draw(0, 1);

  gpu::GSM.states.blend.Set(true);
  gpu::GSM.states.blend_func.Push(gpu::GLBlendType::KeepDestAlpha);

  /* Background tiles */
  base_layer_.quad_array_->Draw(1, base_layer_.bg_tile_count_);

  gpu::GSM.states.blend_func.Set(gpu::GLBlendType::Normal);

  auto& frame_shader = gpu::GSM.shaders->base;
  frame_shader.Bind();
  frame_shader.SetProjectionMatrix(gpu::GSM.states.viewport.Current().Size());
  frame_shader.SetTexture(windowskin_->AsGLType().tex);
  frame_shader.SetTextureSize(windowskin_->GetSize());
  frame_shader.SetTransOffset(base::Vec2());

  /* Frame corner and sides */
  base_layer_.quad_array_->Draw(1 + base_layer_.bg_tile_count_,
                                base_layer_.frame_tile_count_);

  gpu::GSM.states.blend_func.Pop();
  gpu::GSM.states.viewport.Pop();
  gpu::GSM.states.blend.Pop();
}

void Window2::UpdateBaseQuadInternal() {
  const float openness = openness_ / 255.0f;
  const base::Rect tex(0, 0, rect_.width, rect_.height);
  const base::Rect pos(0, (rect_.height / 2.0f) * (1.0 - openness), rect_.width,
                       rect_.height * openness);

  base_quad_->SetTexCoordRect(tex);
  base_quad_->SetPositionRect(pos);
}

void Window2::ToneChangedInternal() {}

}  // namespace content
