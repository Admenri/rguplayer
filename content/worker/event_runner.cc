// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/worker/event_runner.h"

#include "SDL_timer.h"

namespace content {

EventRunner::EventRunner()
    : fps_counter_{0, SDL_GetPerformanceCounter(),
                   SDL_GetPerformanceFrequency()} {
  user_event_id_ = SDL_RegisterEvents(EVENT_NUMS);
}

void EventRunner::InitEventDispatcher(scoped_refptr<CoreConfigure> config,
                                      base::WeakPtr<ui::Widget> window) {
  window_ = window;
  config_ = config;
  loop_runner_ =
      std::make_unique<base::RunLoop>(base::RunLoop::MessagePumpType::UI);

  quit_observer_ = base::RunLoop::BindEventDispatcher(
      base::BindRepeating(&EventRunner::EventFilter, base::Unretained(this)));
}

void EventRunner::EventMain() {
  loop_runner_->Run();
}

void EventRunner::EventFilter(const SDL_Event& event) {
  int user_event = event.type - user_event_id_;
  if (user_event == QUIT_SYSTEM_EVENT || event.type == SDL_EVENT_QUIT) {
    loop_runner_->QuitWhenIdle();
  }
  if (user_event == UPDATE_FPS_DISPLAY) {
    uint64_t now_ticks = SDL_GetPerformanceCounter();
    uint64_t delta_ticks = now_ticks - fps_counter_.last_counter;
    if (delta_ticks >= fps_counter_.counter_freq) {
      int32_t average_fps = fps_counter_.frame_count;
      window_->SetTitle(config_->game_title() +
                        " FPS: " + std::to_string(average_fps));

      fps_counter_.last_counter = SDL_GetPerformanceCounter();
      fps_counter_.frame_count = 0;
    }

    fps_counter_.frame_count++;
  }
}

}  // namespace content
