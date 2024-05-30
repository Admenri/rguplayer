#include "utils.hpp"

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <thread>

#include "player.hpp"

namespace uvpx {

DebugLogFuncPtr g_debugLogFuncPtr = nullptr;

/// Returns number of CPU cores.
int getSystemThreadsCount() {
  return std::thread::hardware_concurrency();
}

/// Sets debug log callback.
void setDebugLog(DebugLogFuncPtr func) {
  g_debugLogFuncPtr = func;
}

/// Internal debug log.
void debugLog(const char* format, ...) {
  char buffer[UVPX_THREAD_ERROR_DESC_SIZE];

  va_list arglist;
  va_start(arglist, format);
  vsprintf(buffer, format, arglist);
  va_end(arglist);

  if (g_debugLogFuncPtr != nullptr)
    g_debugLogFuncPtr(buffer);
}

}  // namespace uvpx
