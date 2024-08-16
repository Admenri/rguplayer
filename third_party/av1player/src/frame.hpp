#ifndef _UVPX_FRAME_H_
#define _UVPX_FRAME_H_

#include <cstdio>
#include <memory>

#include "dll_defines.hpp"

namespace uvpx {

class UVPX_EXPORT Frame {
 private:
  unsigned char* m_planes[3];
  size_t m_width[3];
  size_t m_height[3];

  double m_time;

 public:
  Frame();
  ~Frame();

  void resize(int id, size_t frame_width, size_t frame_height);
  void copyData(Frame* dst);

  unsigned char* plane(int id) const;
  size_t width(int id) const;
  size_t height(int id) const;

  void setTime(double time);
  double time() const;
};

}  // namespace uvpx

#endif  // _UVPX_FRAME_H_
