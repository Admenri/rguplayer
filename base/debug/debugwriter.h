#ifndef BASE_DEBUG_DEBUGWRITER_H
#define BASE_DEBUG_DEBUGWRITER_H

#include <iostream>
#include <sstream>
#include <vector>

#ifdef __ANDROID__
#include <android/log.h>
#endif

namespace base {

class Debug {
 public:
  Debug() { buf << std::boolalpha; }

  template <typename T>
  Debug &operator<<(const T &t) {
    buf << t;
    buf << " ";

    return *this;
  }

  template <typename T>
  Debug &operator<<(const std::vector<T> &v) {
    for (size_t i = 0; i < v.size(); ++i) buf << v[i] << " ";

    return *this;
  }

  ~Debug() { std::cerr << buf.str() << std::endl; }

 private:
  std::stringstream buf;
};

}  // namespace base

#endif  // BASE_DEBUG_DEBUGWRITER_H
