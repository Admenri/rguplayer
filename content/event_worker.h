#ifndef CONTENT_EVENT_WORKER_H_
#define CONTENT_EVENT_WORKER_H_

#include <SDL_keyboard.h>

#include <vector>

namespace content {

class EventWorker {
 public:
  EventWorker();
  virtual ~EventWorker();

  EventWorker(const EventWorker&) = delete;
  EventWorker& operator=(const EventWorker&) = delete;

  void CreateLoop();
  void Execute();

  std::vector<bool>& GetKeyStates() { return key_states_; }

 private:
  std::vector<bool> key_states_{SDL_NUM_SCANCODES};
};

}  // namespace content

#endif  // CONTENT_EVENT_WORKER_H_