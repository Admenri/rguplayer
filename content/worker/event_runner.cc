// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/worker/event_runner.h"

#include "content/worker/binding_worker.h"

#include "SDL_timer.h"

namespace content {

EventRunner::EventRunner(WorkerShareData* share_data)
    : share_data_(share_data),
      fps_counter_{0, SDL_GetPerformanceCounter(),
                   SDL_GetPerformanceFrequency(), 0} {
  share_data_->user_event_id = SDL_RegisterEvents(EVENT_NUMS);
}

void EventRunner::InitDispatcher(base::WeakPtr<BindingRunner> binding_runner) {
  binding_runner_ = binding_runner;
  loop_runner_ =
      std::make_unique<base::RunLoop>(base::RunLoop::MessagePumpType::UI);
  share_data_->event_runner = loop_runner_->task_runner();

  quit_observer_ = base::RunLoop::BindEventDispatcher(
      base::BindRepeating(&EventRunner::EventDispatch, base::Unretained(this)));

  SDL_SetEventFilter(&EventRunner::EventFilter, this);
}

void EventRunner::EventMain() {
  loop_runner_->Run();
}

int EventRunner::EventFilter(void* userdata, SDL_Event* event) {
  EventRunner* self = static_cast<EventRunner*>(userdata);
  WorkerShareData* data = self->share_data_;

  switch (event->type) {
    case SDL_EVENT_WILL_ENTER_BACKGROUND:
      data->background_sync.require.store(true);
      return 0;
    case SDL_EVENT_DID_ENTER_FOREGROUND:
      data->background_sync.signal.store(true);
      return 0;
  }

  return 1;
}

void EventRunner::EventDispatch(const SDL_Event& event) {
  base::WeakPtr<ui::Widget> window = share_data_->window;
  int user_event = event.type - share_data_->user_event_id;

  /* Push queue for GUI event process */
  if (share_data_->enable_settings_menu)
    share_data_->event_queue.enqueue(event);

  /* Application quit flag */
  if (user_event == QUIT_SYSTEM_EVENT || event.type == SDL_EVENT_QUIT) {
    loop_runner_->QuitWhenIdle();
  }

  /* Display fps on window title */
  if (user_event == UPDATE_FPS_DISPLAY) {
    uint64_t now_ticks = SDL_GetPerformanceCounter();
    uint64_t delta_ticks = now_ticks - fps_counter_.last_counter;
    if (delta_ticks >= fps_counter_.counter_freq) {
      // Set display fps
      fps_counter_.average_fps = fps_counter_.frame_count;
      share_data_->display_fps = fps_counter_.average_fps;

      // Update fps
      fps_counter_.last_counter = SDL_GetPerformanceCounter();
      fps_counter_.frame_count = 0;
    }

    fps_counter_.frame_count++;
  }

  /* Reset content */
  if (event.type == SDL_EVENT_KEY_UP &&
      event.key.windowID == window->GetWindowID()) {
    if (event.key.scancode == SDL_SCANCODE_F1) {
      // Settings menu
      if (!share_data_->config->disable_menu())
        share_data_->enable_settings_menu = !share_data_->enable_settings_menu;
    } else if (event.key.scancode == SDL_SCANCODE_F12) {
      // Trigger reset process
      if (!share_data_->config->disable_reset())
        binding_runner_->RequestReset();
    }
  }

  // Window focus
  bool background_allow = share_data_->config->background_running();
  if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST && !background_allow) {
    if (event.window.windowID == window->GetWindowID()) {
      share_data_->background_sync.require.store(true);
      share_data_->background_sync.signal.store(false);
    }
  }

  if (event.type == SDL_EVENT_WINDOW_FOCUS_GAINED || background_allow) {
    if (event.window.windowID == window->GetWindowID() || background_allow) {
      share_data_->background_sync.require.store(false);
      share_data_->background_sync.signal.store(true);
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

}  // namespace content
