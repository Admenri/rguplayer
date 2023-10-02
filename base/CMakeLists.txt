# base components
cmake_minimum_required(VERSION 3.18.0)

#--------------------------------------------------------------------------------
# Define base source
#--------------------------------------------------------------------------------

set(BASE_SRC
  bind/bind.h
  bind/bind_internal.h
  bind/callback.h
  bind/callback_forward.h
  bind/callback_helpers.cc
  bind/callback_helpers.h
  bind/callback_internal.cc
  bind/callback_internal.h
  bind/callback_list.h
  bind/cancelable_callback.h
  buildflags/build.h
  buildflags/compiler_specific.h
  debug/logging.cc
  debug/logging.h
  debug/debugwriter.h
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
  thread/thread_checker.h
  worker/run_loop.cc
  worker/run_loop.h
  worker/thread_worker.cc
  worker/thread_worker.h
  template_util.h
)

add_library(core_base ${BASE_SRC})
target_link_libraries(core_base SDL2::Core)
