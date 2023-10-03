#include "modules/bitmap.h"

#include <SDL_image.h>

#include <algorithm>

namespace modules {

Bitmap::Bitmap(scoped_refptr<content::RendererThread> worker, int width,
               int height)
    : worker_(worker),
      pixel_format_(SDL_AllocFormat(SDL_PIXELFORMAT_ABGR8888), SDL_FreeFormat) {
  worker_->GetRenderThreadRunner()->PostTask(base::BindOnce(
      &Bitmap::InitTextureFrameBuffer, weak_ptr_factory_.GetWeakPtr(),
      base::Vec2i(width, height)));
}

Bitmap::Bitmap(scoped_refptr<content::RendererThread> worker,
               const std::string& filename)
    : worker_(worker),
      pixel_format_(SDL_AllocFormat(SDL_PIXELFORMAT_ABGR8888), SDL_FreeFormat) {
  SDL_Surface* surf = IMG_Load(filename.c_str());

  worker_->GetRenderThreadRunner()->PostTask(base::BindOnce(
      &Bitmap::InitTextureFrameBuffer, weak_ptr_factory_.GetWeakPtr(), surf));
}

Bitmap::~Bitmap() { Dispose(); }

base::Vec2i Bitmap::GetSize() { return texture_->GetSize(); }

base::Rect Bitmap::GetRect() {
  return base::Rect(base::Vec2i(), texture_->GetSize());
}

void Bitmap::Blt(int x, int y, Bitmap* src_bitmap, const base::Rect& src_rect,
                 int opacity) {}

void Bitmap::StretchBlt(const base::Rect& dst_rect, Bitmap* src_bitmap,
                        const base::Rect& src_rect, int opacity) {}

void Bitmap::FillRect(const base::Rect& rect, const base::Vec4i& color) {}

void Bitmap::GradientFillRect(const base::Rect& rect, const base::Vec4i& color1,
                              const base::Vec4i& color2, bool vertical) {}

void Bitmap::Clear() {
  worker_->GetRenderThreadRunner()->PostTask(base::BindOnce(
      &Bitmap::ClearInternal, weak_ptr_factory_.GetWeakPtr(), std::nullopt));
}

void Bitmap::ClearRect(const base::Rect& rect) {
  worker_->GetRenderThreadRunner()->PostTask(base::BindOnce(
      &Bitmap::ClearInternal, weak_ptr_factory_.GetWeakPtr(), rect));
}

base::Vec4i Bitmap::GetPixel(int x, int y) {
  GetSurface();

  size_t offset =
      x * pixel_format_->BytesPerPixel + y * read_pixel_buffer_->pitch;
  uint8_t* bytes = (uint8_t*)read_pixel_buffer_->pixels + offset;
  uint32_t pixel = *((uint32_t*)bytes);

  return base::Vec4i((pixel >> pixel_format_->Rshift) & 0xFF,
                     (pixel >> pixel_format_->Gshift) & 0xFF,
                     (pixel >> pixel_format_->Bshift) & 0xFF,
                     (pixel >> pixel_format_->Ashift) & 0xFF);
}

void Bitmap::SetPixel(int x, int y, const base::Vec4i& color) {
  base::Vec4i pixel(color);

  pixel.x = std::clamp(pixel.x, 0, 255);
  pixel.y = std::clamp(pixel.y, 0, 255);
  pixel.z = std::clamp(pixel.z, 0, 255);
  pixel.w = std::clamp(pixel.w, 0, 255);

  if (read_pixel_buffer_) {
    size_t offset =
        x * pixel_format_->BytesPerPixel + y * read_pixel_buffer_->pitch;
    uint8_t* bytes = (uint8_t*)read_pixel_buffer_->pixels + offset;
    *((uint32_t*)bytes) =
        SDL_MapRGBA(pixel_format_.get(), color.x, color.y, color.z, color.w);
  }

  worker_->GetRenderThreadRunner()->PostTask(
      base::BindOnce(&Bitmap::SetPixelInternal, weak_ptr_factory_.GetWeakPtr(),
                     base::Vec2i(x, y), color));
}

void Bitmap::HueChange(int hue) {}

void Bitmap::Blur() {}

void Bitmap::RadialBlur(int angle, int division) {}

SDL_Surface* Bitmap::GetSurface() {
  if (!read_pixel_buffer_) {
    base::RunLoop sync_loop;
    worker_->GetRenderThreadRunner()->PostTask(base::BindOnce(
        &Bitmap::GetSurfaceInternal, weak_ptr_factory_.GetWeakPtr(),
        sync_loop.QuitClosure()));
    sync_loop.Run();
  }

  return read_pixel_buffer_;
}

void Bitmap::OnObjectDisposed() {
  worker_->GetRenderThreadRunner()->ReleaseSoon(std::move(texture_));
  worker_->GetRenderThreadRunner()->DeleteSoon(std::move(frame_buffer_));

  if (read_pixel_buffer_) SDL_FreeSurface(read_pixel_buffer_);
}

void Bitmap::NeedUpdate() {
  if (read_pixel_buffer_) {
    SDL_FreeSurface(read_pixel_buffer_);
    read_pixel_buffer_ = nullptr;
  }
}

void Bitmap::EnsureSurfaceFormat(SDL_Surface*& surface) {
  if (surface->format->format == pixel_format_->format) return;

  SDL_Surface* conv_surf =
      SDL_ConvertSurfaceFormat(surface, pixel_format_->format, 0);
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

void Bitmap::ClearInternal(const std::optional<base::Rect>& rect) {
  auto* cc = content::RendererThread::GetCCForRenderer();

  frame_buffer_->Bind();

  if (rect.has_value()) {
    base::Rect calc_rect(*rect);

    if (calc_rect.width < 0) {
      calc_rect.width = -calc_rect.width;
      calc_rect.x -= calc_rect.width;
    }

    if (calc_rect.height < 0) {
      calc_rect.height = -calc_rect.height;
      calc_rect.y -= calc_rect.height;
    }

    cc->States()->scissor_test->Push(true);
    cc->States()->scissor_region->Push(calc_rect);
  }

  frame_buffer_->Clear();

  if (rect.has_value()) {
    cc->States()->scissor_test->Pop();
    cc->States()->scissor_region->Pop();
  }

  frame_buffer_->Unbind();

  NeedUpdate();
}

void Bitmap::GetSurfaceInternal(base::OnceClosure complete_closure) {
  auto* cc = content::RendererThread::GetCCForRenderer();

  if (read_pixel_buffer_) SDL_FreeSurface(read_pixel_buffer_);
  auto size = texture_->GetSize();
  read_pixel_buffer_ = SDL_CreateRGBSurface(
      0, size.x, size.y, pixel_format_->BitsPerPixel, pixel_format_->Rmask,
      pixel_format_->Gmask, pixel_format_->Bmask, pixel_format_->Amask);

  frame_buffer_->Bind();
  cc->States()->viewport->Push(base::Rect(base::Vec2i(), size));

  frame_buffer_->ReadPixels(base::Vec4i(0, 0, size.x, size.y), GL_RGBA,
                            GL_UNSIGNED_BYTE, read_pixel_buffer_->pixels);

  cc->States()->viewport->Pop();
  frame_buffer_->Unbind();

  std::move(complete_closure).Run();
}

void Bitmap::SetPixelInternal(const base::Vec2i& pos,
                              const base::Vec4i& color) {
  texture_->Bind();

  uint32_t pixel =
      SDL_MapRGBA(pixel_format_.get(), color.x, color.y, color.z, color.w);
  texture_->BufferData(base::Vec4i(pos.x, pos.y, 1, 1), &pixel, GL_RGBA);

  texture_->Unbind();

  NeedUpdate();
}

}  // namespace modules
