// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/worker/event_runner.h"

#include "content/worker/binding_worker.h"

#include "SDL_timer.h"

namespace content {

EventRunner::EventRunner(WorkerShareData* share_data)
    : share_data_(share_data),
      fps_counter_{false, 0, SDL_GetPerformanceCounter(),
                   SDL_GetPerformanceFrequency(), 0} {
  share_data_->user_event_id = SDL_RegisterEvents(EVENT_NUMS);
}

void EventRunner::InitDispatcher(base::WeakPtr<BindingRunner> binding_runner) {
  binding_runner_ = binding_runner;
  loop_runner_ =
      std::make_unique<base::RunLoop>(base::RunLoop::MessagePumpType::UI);

  quit_observer_ = base::RunLoop::BindEventDispatcher(
      base::BindRepeating(&EventRunner::EventFilter, base::Unretained(this)));
}

void EventRunner::EventMain() {
  loop_runner_->Run();
}

void EventRunner::EventFilter(const SDL_Event& event) {
  base::WeakPtr<ui::Widget> window = share_data_->window;
  int user_event = event.type - share_data_->user_event_id;

  /* Application quit flag */
  if (user_event == QUIT_SYSTEM_EVENT || event.type == SDL_EVENT_QUIT) {
    loop_runner_->QuitWhenIdle();
  }

  /* Display fps on window title */
  if (user_event == UPDATE_FPS_DISPLAY) {
    uint64_t now_ticks = SDL_GetPerformanceCounter();
    uint64_t delta_ticks = now_ticks - fps_counter_.last_counter;
    if (delta_ticks >= fps_counter_.counter_freq) {
      if (fps_counter_.enable_display) {
        fps_counter_.average_fps = fps_counter_.frame_count;
        UpdateFPSDisplay(std::make_optional<int32_t>(fps_counter_.average_fps));
      }

      fps_counter_.last_counter = SDL_GetPerformanceCounter();
      fps_counter_.frame_count = 0;
    }

    fps_counter_.frame_count++;
  }

  /* Reset content */
  if (event.type == SDL_EVENT_KEY_UP &&
      event.window.windowID == window->GetWindowID()) {
    if (event.key.keysym.scancode == SDL_SCANCODE_F2) {
      // Switch fps display mode
      fps_counter_.enable_display = !fps_counter_.enable_display;

      std::optional<int32_t> fps = std::nullopt;
      if (fps_counter_.enable_display)
        fps = std::make_optional<int32_t>(fps_counter_.average_fps);

      UpdateFPSDisplay(fps);
    } else if (event.key.keysym.scancode == SDL_SCANCODE_F12) {
      // Trigger reset process
      binding_runner_->RequestReset();
    }
  }

  // Audio device changed
  if (event.type == SDL_EVENT_AUDIO_DEVICE_ADDED) {
    SDL_AudioDeviceID which = event.adevice.which;
    LOG(INFO) << "[Event] Audio device added: "
              << SDL_GetAudioDeviceName(which);
  } else if (event.type == SDL_EVENT_AUDIO_DEVICE_REMOVED) {
    SDL_AudioDeviceID which = event.adevice.which;
    LOG(INFO) << "[Event] Audio device removed: " << which;
  }
}

void EventRunner::UpdateFPSDisplay(std::optional<int32_t> fps) {
  base::WeakPtr<ui::Widget> window = share_data_->window;
  scoped_refptr<CoreConfigure> config = share_data_->config;

  if (fps.has_value())
    return window->SetTitle(config->game_title() +
                            " - FPS: " + std::to_string(*fps));
  return window->SetTitle(config->game_title());
}

}  // namespace content
