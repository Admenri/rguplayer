#ifndef CONTENT_SCRIPT_WINDOW_H_
#define CONTENT_SCRIPT_WINDOW_H_

#include "content/script/bitmap.h"
#include "content/script/disposable.h"
#include "content/script/utility.h"
#include "content/script/viewport.h"
#include "gpu/gles2/draw/quad_array.h"

namespace content {

/* Adaptor for RGSS 2/3 */
class Window2 : public base::RefCounted<Window2>,
                public Disposable,
                public ViewportChild {
 public:
  Window2(scoped_refptr<Viewport> viewport = nullptr);
  Window2(int x = 0, int y = 0, int width = 0, int height = 0);
  ~Window2() override;

  Window2(const Window2&) = delete;
  Window2& operator=(const Window2&) = delete;

  /* Method */
  void Update();
  void Move(int x, int y, int width, int height);

  bool IsOpened() const { return openness_ == 255; }
  bool IsClosed() const { return openness_ == 0; }

  /* Attribute */
  scoped_refptr<Bitmap> GetWindowskin() const { return windowskin_; }
  void SetWindowskin(scoped_refptr<Bitmap> windowskin);

  scoped_refptr<Bitmap> GetContents() const { return contents_; }
  void SetContents(scoped_refptr<Bitmap> contents);

  scoped_refptr<Rect> GetCursorRect() const { return cursor_rect_; }
  void SetCursorRect(scoped_refptr<Rect> cursor_rect);

  bool IsActive() const { return active_; }
  void SetActive(bool active);

  bool IsVisible() const { return visible_; }
  void SetVisible(bool visible);

  bool IsArrowsVisible() const { return arrows_visible_; }
  void SetArrowsVisible(bool arrows_visible);

  bool IsPause() const { return pause_; }
  void SetPause(bool pause);

  int GetX() const { return rect_.x; }
  void SetX(int x);

  int GetY() const { return rect_.y; }
  void SetY(int y);

  int GetWidth() const { return rect_.width; }
  void SetWidth(int width);

  int GetHeight() const { return rect_.height; }
  void SetHeight(int height);

  int GetOX() const { return ox_; }
  void SetOX(int ox);

  int GetOY() const { return oy_; }
  void SetOY(int oy);

  int GetPadding() const { return padding_; }
  void SetPadding(int padding);

  int GetPaddingBottom() const { return padding_bottom_; }
  void SetPaddingBottom(int padding_bottom);

  int GetOpacity() const { return opacity_; }
  void SetOpacity(int opacity);

  int GetBackOpacity() const { return back_opacity_; }
  void SetBackOpacity(int back_opacity);

  int GetContentsOpacity() const { return contents_opacity_; }
  void SetContentsOpacity(int contents_opacity);

  int GetOpenness() const { return openness_; }
  void SetOpenness(int openness);

  scoped_refptr<Tone> GetTone() const { return tone_; }
  void SetTone(scoped_refptr<Tone> tone);

 private:
  void OnObjectDisposed() override;
  std::string_view DisposedObjectName() const override { return "Window"; }

  void BeforeComposite() override;
  void Composite() override;
  void CheckDisposed() const override;
  void OnViewportRectChanged(const DrawableParent::ViewportInfo& rect) override;

  void InitWindow();
  void InitWindowInternal();
  void CalcBaseQuadArrayInternal();
  void UpdateBaseTextureInternal();
  void UpdateBaseQuadInternal();
  void ToneChangedInternal();

  scoped_refptr<Bitmap> windowskin_;
  scoped_refptr<Bitmap> contents_;
  scoped_refptr<Rect> cursor_rect_;
  bool active_ = true;
  bool visible_ = true;
  bool arrows_visible_ = true;
  bool pause_ = false;

  base::Rect rect_;
  int ox_ = 0, oy_ = 0;

  int padding_ = 12;
  int padding_bottom_ = padding_;

  int opacity_ = 255;
  int back_opacity_ = 255;
  int contents_opacity_ = 255;
  int openness_ = 255;

  scoped_refptr<Tone> tone_;
  base::CallbackListSubscription tone_observer_;

  struct {
    std::unique_ptr<gpu::QuadDrawableArray<gpu::CommonVertex>> quad_array_;

    size_t bg_tile_count_;
    size_t frame_tile_count_;
    gpu::TextureFrameBuffer tfb_;

    bool base_tex_updated_ = true;
  } base_layer_;

  std::unique_ptr<gpu::QuadDrawable> base_quad_;
  bool base_quad_updated_ = true;

  base::WeakPtrFactory<Window2> weak_ptr_factory_{this};
};

}  // namespace content

#endif  // CONTENT_SCRIPT_WINDOW_H_