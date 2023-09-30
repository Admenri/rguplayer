#include "modules/bitmap.h"

#include <SDL_image.h>

#include <algorithm>

namespace modules {

Bitmap::Bitmap(scoped_refptr<content::RendererThread> worker, int width,
               int height)
    : worker_(worker) {
  worker_->GetRenderThreadRunner()->PostTask(base::BindOnce(
      &Bitmap::InitBufferWH, weak_ptr_factory_.GetWeakPtr(), width, height));
}

Bitmap::Bitmap(scoped_refptr<content::RendererThread> worker,
               const std::string& filename)
    : worker_(worker) {
  worker_->GetRenderThreadRunner()->PostTask(base::BindOnce(
      &Bitmap::InitBufferPath, weak_ptr_factory_.GetWeakPtr(), filename));
}

Bitmap::~Bitmap() { Dispose(); }

base::Vec2i& Bitmap::GetSize() { return frame_canvas_buffer_->GetSize(); }

void Bitmap::Clear() {
  frame_canvas_buffer_->BindFrame();
  frame_canvas_buffer_->Clear();
  frame_canvas_buffer_->UnbindFrame();
}

void Bitmap::OnObjectDisposed() {
  worker_->GetRenderThreadRunner()->DeleteSoon(std::move(frame_canvas_buffer_));
}

void Bitmap::InitBufferWH(int width, int height) {
  renderer::CCLayer* cc = content::RendererThread::GetCCForRenderer();

  frame_canvas_buffer_ =
      std::make_unique<renderer::FrameBufferTexture>(cc->GetContext());

  frame_canvas_buffer_->BindTexture();
  frame_canvas_buffer_->Alloc(base::Vec2i(width, height));
  frame_canvas_buffer_->UnbindTexture();
}

void Bitmap::InitBufferPath(const std::string& filename) {
  frame_canvas_buffer_ = std::make_unique<renderer::FrameBufferTexture>(
      content::RendererThread::GetCCForRenderer()->GetContext());

  SDL_Surface* img_surface = IMG_Load(filename.c_str());
  EnsureSurfaceFormat(img_surface);

  frame_canvas_buffer_->BindTexture();
  frame_canvas_buffer_->BufferData(base::Vec2i(img_surface->w, img_surface->h),
                                   img_surface->pixels, GL_RGBA);
  frame_canvas_buffer_->UnbindTexture();

  SDL_FreeSurface(img_surface);
}

void Bitmap::EnsureSurfaceFormat(SDL_Surface*& surface) {
  if (surface->format->format == SDL_PIXELFORMAT_ABGR8888) return;

  SDL_Surface* conv_surf =
      SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ABGR8888, 0);
  SDL_FreeSurface(surface);

  surface = conv_surf;
}

}  // namespace modules
