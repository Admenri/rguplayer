// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/render_thread.h"

#include <unordered_map>

namespace content {

namespace {

std::unordered_map<std::thread::id, renderer::CCLayer*> g_thread_context;

}  // namespace

RendererThread::RendererThread() {
  thread_.reset(new base::ThreadWorker());
  thread_->Start(base::RunLoop::MessagePumpType::IO);

  thread_->WaitUntilStart();
}

RendererThread::~RendererThread() {
  thread_->task_runner()->PostTask(base::BindOnce(
      &RendererThread::QuitThread, weak_ptr_factory_.GetWeakPtr()));
}

void RendererThread::InitContextAsync(ui::Widget* render_canvas) {
  render_widget_ = render_canvas->AsWeakPtr();

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
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  gl_context_ = SDL_GL_CreateContext(render_widget_->AsSDLWindow());
  renderer_cc_.reset(new renderer::CCLayer(render_widget_, gl_context_));

  g_thread_context.insert(
      std::make_pair(std::this_thread::get_id(), renderer_cc_.get()));

  renderer_cc_->GetContext()->glClearColor(1, 1, 1, 1);
  renderer_cc_->GetContext()->glClear(GL_COLOR_BUFFER_BIT);
  SDL_GL_SwapWindow(render_widget_->AsSDLWindow());
}

void RendererThread::QuitThread() {
  renderer_cc_.reset();

  SDL_GL_DeleteContext(gl_context_);

  auto iter = g_thread_context.find(std::this_thread::get_id());
  if (iter != g_thread_context.end()) {
    g_thread_context.erase(iter);
  }
}

}  // namespace content
