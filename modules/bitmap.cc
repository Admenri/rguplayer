#include "modules/bitmap.h"

#include <SDL_image.h>

#include "content/render_thread.h"

namespace modules {

Bitmap::Bitmap(base::WeakPtr<renderer::CCLayer> cc, int width, int height)
    : cc_(cc) {
  InitCanvas();

  frame_canvas_->Bind();
  frame_canvas_->Alloc(base::Vec2i(width, height));
  frame_canvas_->Clear();
}

Bitmap::Bitmap(base::WeakPtr<renderer::CCLayer> cc, const std::string& filename)
    : cc_(cc) {
  InitCanvas();

  SDL_Surface* img_surface = IMG_Load(filename.c_str());
  EnsureSurfaceFormat(img_surface);

  frame_canvas_->Bind();
  frame_canvas_->BufferData(base::Vec2i(img_surface->w, img_surface->h),
                            img_surface->pixels, GL_RGBA);

  SDL_FreeSurface(img_surface);
}

void Bitmap::Clear() {
  CheckedForDispose();

  frame_canvas_->Bind();
  frame_canvas_->Clear();
}

void Bitmap::Bind() { frame_canvas_->BindTexture(); }

void Bitmap::Unbind() { frame_canvas_->UnbindTexture(); }

void Bitmap::InitCanvas() {
  frame_canvas_ =
      std::make_unique<renderer::FrameBufferTexture>(cc_->GetContext());
}

void Bitmap::OnObjectDisposed() {}

void Bitmap::EnsureSurfaceFormat(SDL_Surface*& surface) {
  if (surface->format->format == SDL_PIXELFORMAT_ABGR8888) return;

  SDL_Surface* conv_surf =
      SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ABGR8888, 0);
  SDL_FreeSurface(surface);

  surface = conv_surf;
}

}  // namespace modules
