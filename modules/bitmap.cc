#include "modules/bitmap.h"

#include <SDL_image.h>

#include <algorithm>

namespace modules {

Bitmap::Bitmap(scoped_refptr<content::RendererThread> worker, int width,
               int height)
    : worker_(worker), format_(SDL_AllocFormat(SDL_PIXELFORMAT_ABGR8888)) {
  worker_->GetRenderThreadRunner()->PostTask(base::BindOnce(
      &Bitmap::InitBufferWH, weak_ptr_factory_.GetWeakPtr(), width, height));
}

Bitmap::Bitmap(scoped_refptr<content::RendererThread> worker,
               const std::string& filename)
    : worker_(worker), format_(SDL_AllocFormat(SDL_PIXELFORMAT_ABGR8888)) {
  worker_->GetRenderThreadRunner()->PostTask(base::BindOnce(
      &Bitmap::InitBufferPath, weak_ptr_factory_.GetWeakPtr(), filename));
}

Bitmap::~Bitmap() { Dispose(); }

base::Vec2i Bitmap::GetSize() { return texture_->GetSize(); }

base::Vec4i Bitmap::GetRect() {
  auto size = texture_->GetSize();
  return base::Vec4i(0, 0, size.x, size.y);
}

void Bitmap::Clear() {}

SDL_Surface* Bitmap::SaveSurface() {
  SDL_Surface* tmp_surf = SDL_CreateRGBSurface(
      0, GetSize().x, GetSize().y, format_->BytesPerPixel, format_->Rmask,
      format_->Gmask, format_->Bmask, format_->Amask);

  base::RunLoop sync_loop;
  worker_->GetRenderThreadRunner()->PostTask(base::BindOnce(
      &Bitmap::SaveSurfaceInternal, weak_ptr_factory_.GetWeakPtr(), tmp_surf,
      std::move(sync_loop.QuitClosure())));
  sync_loop.Run();

  return tmp_surf;
}

void Bitmap::OnObjectDisposed() {
  if (format_) SDL_FreeFormat(format_);

  worker_->GetRenderThreadRunner()->ReleaseSoon(std::move(texture_));
}

void Bitmap::InitBufferWH(int width, int height) {
  renderer::CCLayer* cc = content::RendererThread::GetCCForRenderer();

  texture_ = new renderer::GLTexture(cc->GetContext());

  texture_->Bind();
  texture_->SetSize(base::Vec2i(width, height));
  texture_->AllocEmpty();
  texture_->Unbind();
}

void Bitmap::InitBufferPath(const std::string& filename) {
  renderer::CCLayer* cc = content::RendererThread::GetCCForRenderer();

  SDL_Surface* img_surface = IMG_Load(filename.c_str());
  EnsureSurfaceFormat(img_surface);

  auto size = base::Vec2i(img_surface->w, img_surface->h);

  texture_ = new renderer::GLTexture(cc->GetContext());
  texture_->Bind();
  texture_->SetSize(size);
  texture_->BufferData(img_surface->pixels, GL_RGBA);
  texture_->Unbind();

  SDL_FreeSurface(img_surface);
}

void Bitmap::SaveSurfaceInternal(SDL_Surface* tmp_surf,
                                 base::OnceClosure complete) {
  std::move(complete).Run();
}

void Bitmap::EnsureSurfaceFormat(SDL_Surface*& surface) {
  if (surface->format->format == SDL_PIXELFORMAT_ABGR8888) return;

  SDL_Surface* conv_surf =
      SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ABGR8888, 0);
  SDL_FreeSurface(surface);

  surface = conv_surf;
}

}  // namespace modules
