#include "event_worker.h"

#include <SDL_events.h>

#include <algorithm>

#include "base/debug/debugwriter.h"

namespace content {

EventWorker::EventWorker() {}

EventWorker::~EventWorker() {}

void EventWorker::CreateLoop() {
  
}

void EventWorker::Execute() {
  SDL_Event sdl_event;
  for (;;) {
    if (!SDL_WaitEvent(&sdl_event)) {
      base::Debug() << "[Content] Failed to get event from queue.";
      break;
    }

    switch (sdl_event.type) {
      case SDL_KEYDOWN: {
        uint16_t key_id = std::clamp<uint16_t>(sdl_event.key.keysym.scancode, 0,
                                               SDL_NUM_SCANCODES);
        key_states_[key_id] = true;
        break;
      }
      case SDL_KEYUP: {
        uint16_t key_id = std::clamp<uint16_t>(sdl_event.key.keysym.scancode, 0,
                                               SDL_NUM_SCANCODES);
        key_states_[key_id] = false;
        break;
      }
      default:
        break;
    }

    if (sdl_event.type == SDL_QUIT) {
      base::Debug() << "[Content] Event Worker Quit.";
      break;
    }
  }
}

}  // namespace content
