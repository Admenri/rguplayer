#ifndef _UVPX_TIMER_H_
#define _UVPX_TIMER_H_

#include <atomic>
#include <chrono>

namespace uvpx {

class Timer {
 protected:
  std::atomic<bool> m_active;
  typedef std::chrono::steady_clock::time_point TimePoint;

  TimePoint m_start;
  TimePoint m_pauseStart;

  double m_pauseDuration;

  void now(TimePoint& tp);
  double secondsElapsed(const TimePoint& start, const TimePoint& end);

 public:
  Timer();
  ~Timer();

  void start();
  void stop();
  void pause();
  void resume();

  double elapsedSeconds();
};

}  // namespace uvpx

#endif  // _UVPX_TIMER_H_
