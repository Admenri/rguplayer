# Base components
cmake_minimum_required(VERSION 3.18.0)

#--------------------------------------------------------------------------------
# Define base source
#--------------------------------------------------------------------------------

set(CORE_BASE_SRC
  bind/bind.h
  bind/bind_internal.h
  bind/callback.h
  bind/callback_forward.h
  bind/callback_helpers.cc
  bind/callback_helpers.h
  bind/callback_internal.cc
  bind/callback_internal.h
  bind/callback_list.cc
  bind/callback_list.h
  bind/cancelable_callback.h
  buildflags/build.h
  buildflags/compiler_specific.h
  containers/linked_list.h
  debug/logging.cc
  debug/logging.h
  exceptions/exception.h
  math/math.cc
  math/math.h
  math/transform.h
  memory/atomic_flag.cc
  memory/atomic_flag.h
  memory/atomic_ref_count.h
  memory/auto_reset.h
  memory/lock.h
  memory/lock_impl.cc
  memory/lock_impl.h
  memory/raw_scoped_refptr_mismatch_checker.h
  memory/ref_counted.cc
  memory/ref_counted.h
  memory/scoped_refptr.h
  memory/weak_ptr.cc
  memory/weak_ptr.h
  thread/thread_checker.cc
  thread/thread_checker.h
  worker/run_loop.cc
  worker/run_loop.h
  worker/thread_worker.cc
  worker/thread_worker.h
  template_util.h
)

add_library(core_base ${CORE_BASE_SRC} ${CORE_BASE_THIRD_PARTY})

if(CMAKE_SYSTEM_NAME STREQUAL "Android")
  set(LOG_LIBS
    android
    log
  )
endif()

if(SDL_STATIC)
target_link_libraries(core_base PUBLIC SDL3::SDL3-static ${LOG_LIBS})
else()
target_link_libraries(core_base PUBLIC SDL3-shared ${LOG_LIBS})
endif()

if (MSVC)
  target_compile_options(core_base PRIVATE /O2 /WX "$<$<CXX_COMPILER_ID:MSVC>:-utf-8>")
else()
  target_compile_options(core_base PRIVATE -O2 -Werror)
endif()
