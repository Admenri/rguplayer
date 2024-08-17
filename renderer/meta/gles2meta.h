// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef RENDERER_META_GLES2META_H_
#define RENDERER_META_GLES2META_H_

#include "base/math/math.h"
#include "renderer/context/gles2_context.h"

namespace renderer {

template <class T>
struct GLID {
  using Target = T;

  GLuint gl;

  GLID(GLuint init = 0) : gl(init) {}
  bool operator==(const GLID& other) { return other.gl == gl; }
};

template <GLenum target>
struct Buffer {
  inline static GLID<Buffer<target>> Gen() {
    GLID<Buffer<target>> id;
    GL.GenBuffers(1, &id.gl);
    return id;
  }

  inline static void Del(const GLID<Buffer<target>>& id) {
    GL.DeleteBuffers(1, &id.gl);
  }

  inline static void Bind(const GLID<Buffer<target>>& id) {
    GL.BindBuffer(target, id.gl);
  }

  inline static void Unbind() { GL.BindBuffer(target, 0); }

  inline static void BufferData(GLsizeiptr size = 0,
                                const void* data = nullptr,
                                GLenum usage = GL_DYNAMIC_DRAW) {
    GL.BufferData(target, size, data, usage);
  }

  inline static void BufferSubData(GLintptr offset,
                                   GLsizeiptr size = 0,
                                   const void* data = nullptr) {
    GL.BufferSubData(target, offset, size, data);
  }
};

struct Texture {
  inline static GLID<Texture> Gen() {
    GLID<Texture> tex;
    GL.GenTextures(1, &tex.gl);
    return tex;
  }

  inline static void Del(const GLID<Texture>& id) {
    GL.DeleteTextures(1, &id.gl);
  }

  inline static void Bind(const GLID<Texture>& id) {
    GL.BindTexture(GL_TEXTURE_2D, id.gl);
  }
  inline static void Unbind() { GL.BindTexture(GL_TEXTURE_2D, 0); }

  inline static void TexImage2D(GLsizei width,
                                GLsizei height,
                                GLenum format = GL_RGBA,
                                const void* buffer = nullptr) {
    GL.TexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                  GL_UNSIGNED_BYTE, buffer);
  }

  inline static void TexSubImage2D(GLint x,
                                   GLint y,
                                   GLsizei width,
                                   GLsizei height,
                                   GLenum format = GL_RGBA,
                                   const void* data = nullptr) {
    GL.TexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, format,
                     GL_UNSIGNED_BYTE, data);
  }

  /*
   * Warning: Texture must set min filter when drawing, otherwise invalid
   * display.
   */
  inline static void SetFilter(GLint filter = GL_LINEAR) {
    GL.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    GL.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
  }

  inline static void SetWrap(GLint wrap = GL_CLAMP_TO_BORDER) {
    GL.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    GL.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
  }
};

struct FrameBuffer {
  inline static GLID<FrameBuffer> Gen() {
    GLID<FrameBuffer> id;
    GL.GenFramebuffers(1, &id.gl);
    return id;
  }

  inline static void Del(const GLID<FrameBuffer>& id) {
    GL.DeleteFramebuffers(1, &id.gl);
  }

  inline static void Bind(const GLID<FrameBuffer>& id) {
    GL.BindFramebuffer(GL_FRAMEBUFFER, id.gl);
  }

  inline static void Unbind() { GL.BindFramebuffer(GL_FRAMEBUFFER, 0); }

  inline static void SetTexture(const GLID<Texture>& tex,
                                uint16_t attachment = 0) {
    GL.FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment,
                            GL_TEXTURE_2D, tex.gl, 0);
  }

  inline static void ClearColor(GLclampf r = 0.0f,
                                GLclampf g = 0.0f,
                                GLclampf b = 0.0f,
                                GLclampf a = 0.0f) {
    GL.ClearColor(r, g, b, a);
  }

  inline static void Clear() { GL.Clear(GL_COLOR_BUFFER_BIT); }
};

using VertexBuffer = Buffer<GL_ARRAY_BUFFER>;
using IndexBuffer = Buffer<GL_ELEMENT_ARRAY_BUFFER>;
using UniformBuffer = Buffer<GL_UNIFORM_BUFFER>;

struct TextureFrameBuffer {
  GLID<Texture> tex;
  GLID<FrameBuffer> fbo;
  base::Vec2i size;

  inline static TextureFrameBuffer Gen() {
    TextureFrameBuffer tfb;

    tfb.tex = Texture::Gen();
    tfb.fbo = FrameBuffer::Gen();

    Texture::Bind(tfb.tex);
    Texture::SetFilter();
    Texture::SetWrap();

    return tfb;
  }

  inline static void Del(const TextureFrameBuffer& tfb) {
    Texture::Del(tfb.tex);
    FrameBuffer::Del(tfb.fbo);
  }

  inline static void Alloc(TextureFrameBuffer& tfb,
                           base::Vec2i size,
                           GLenum format = GL_RGBA) {
    Texture::Bind(tfb.tex);
    Texture::TexImage2D(size.x, size.y, format);
    tfb.size = size;
  }

  inline static void LinkFrameBuffer(const TextureFrameBuffer& tfb) {
    FrameBuffer::Bind(tfb.fbo);
    FrameBuffer::SetTexture(tfb.tex);
  }
};

struct VertexAttrib {};

}  // namespace renderer

#endif  // !RENDERER_META_GLES2META_H_
