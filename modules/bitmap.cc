#include "modules/bitmap.h"

#include <SDL_image.h>

#include <algorithm>

namespace modules {

Bitmap::Bitmap(scoped_refptr<content::RendererThread> worker, int width,
               int height)
    : worker_(worker) {
  worker_->GetRenderThreadRunner()->PostTask(base::BindOnce(
      &Bitmap::InitTextureFrameBuffer, weak_ptr_factory_.GetWeakPtr(),
      base::Vec2i(width, height)));
}

Bitmap::Bitmap(scoped_refptr<content::RendererThread> worker,
               const std::string& filename)
    : worker_(worker) {
  SDL_Surface* surf = IMG_Load(filename.c_str());

  worker_->GetRenderThreadRunner()->PostTask(base::BindOnce(
      &Bitmap::InitTextureFrameBuffer, weak_ptr_factory_.GetWeakPtr(), surf));
}

Bitmap::~Bitmap() { Dispose(); }

base::Vec2i Bitmap::GetSize() { return texture_->GetSize(); }

base::Rect Bitmap::GetRect() {
  return base::Rect(base::Vec2i(), texture_->GetSize());
}

void Bitmap::Clear() {
  worker_->GetRenderThreadRunner()->PostTask(
      base::BindOnce(&Bitmap::ClearInternal, weak_ptr_factory_.GetWeakPtr()));
}

void Bitmap::OnObjectDisposed() {
  worker_->GetRenderThreadRunner()->ReleaseSoon(std::move(texture_));
  worker_->GetRenderThreadRunner()->DeleteSoon(std::move(frame_buffer_));
}

void Bitmap::EnsureSurfaceFormat(SDL_Surface*& surface) {
  if (surface->format->format == SDL_PIXELFORMAT_ABGR8888) return;

  SDL_Surface* conv_surf =
      SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ABGR8888, 0);
  SDL_FreeSurface(surface);

  surface = conv_surf;
}

void Bitmap::InitTextureFrameBuffer(
    std::variant<base::Vec2i, SDL_Surface*> init_data) {
  auto glctx = content::RendererThread::GetCCForRenderer()->GetContext();

  frame_buffer_.reset(new renderer::GLFrameBuffer(glctx));
  texture_ = new renderer::GLTexture(glctx);
  texture_->Bind();
  texture_->SetTextureFilter(GL_NEAREST);

  if (std::holds_alternative<base::Vec2i>(init_data)) {
    texture_->SetSize(std::get<base::Vec2i>(init_data));
    texture_->AllocEmpty();
  } else if (std::holds_alternative<SDL_Surface*>(init_data)) {
    auto surf = std::get<SDL_Surface*>(init_data);
    texture_->SetSize(base::Vec2i(surf->w, surf->h));
    EnsureSurfaceFormat(surf);
    texture_->BufferData(surf->pixels, GL_RGBA);

    SDL_FreeSurface(surf);
  } else {
    NOTREACHED();
  }

  frame_buffer_->Bind();
  frame_buffer_->SetRenderTarget(texture_);
  frame_buffer_->Unbind();
}

void Bitmap::ClearInternal() {
  frame_buffer_->Bind();
  frame_buffer_->Clear();
  frame_buffer_->Unbind();
}

}  // namespace modules
