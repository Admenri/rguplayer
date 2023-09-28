// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/render_thread.h"

#include <unordered_map>

namespace content {

namespace {

std::unordered_map<std::thread::id, renderer::CCLayer*> g_thread_context;

}  // namespace

RendererThread::RendererThread() : render_widget_(nullptr) {
  thread_.reset(new base::ThreadWorker());
  thread_->Start(base::RunLoop::MessagePumpType::IO);

  thread_->WaitUntilStart();
}

RendererThread::~RendererThread() {
  thread_->task_runner()->PostTask(base::BindOnce(
      &RendererThread::QuitThread, weak_ptr_factory_.GetWeakPtr()));
}

void RendererThread::InitContextAsync(SDL_Window* render_canvas) {
  render_widget_ = render_canvas;

  thread_->task_runner()->PostTask(base::BindOnce(
      &RendererThread::InitThread, weak_ptr_factory_.GetWeakPtr()));
}

scoped_refptr<base::SequencedTaskRunner>
RendererThread::GetRenderThreadRunner() {
  return thread_->task_runner();
}

renderer::CCLayer* RendererThread::GetCCForRenderer() {
  auto iter = g_thread_context.find(std::this_thread::get_id());
  if (iter != g_thread_context.end()) {
    return (*iter).second;
  }

  return nullptr;
}

void RendererThread::InitThread() {
  AddRef();

  gl_context_ = SDL_GL_CreateContext(render_widget_);

  try {
    renderer_cc_.reset(new renderer::CCLayer(render_widget_, gl_context_));
  } catch (const base::Exception& e) {
    base::Debug() << "[Core] Error: " << e.GetErrorMessage();
  }

  g_thread_context.insert(
      std::make_pair(std::this_thread::get_id(), renderer_cc_.get()));
}

void RendererThread::QuitThread() {
  SDL_GL_DeleteContext(gl_context_);

  Release();
}

}  // namespace content
