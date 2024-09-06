cmake_minimum_required(VERSION 3.18.0)

set(SRC_AOM_PLAYER
  src/audio_decoder.cpp
  src/audio_decoder.hpp
  src/buffer.hpp
  src/dll_defines.hpp
  src/error_codes.hpp
  src/file_reader.cpp
  src/file_reader.hpp
  src/frame.cpp
  src/frame.hpp
  src/frame_buffer.cpp
  src/frame_buffer.hpp
  src/object_pool.hpp
  src/packet.cpp
  src/packet.hpp
  src/packet_queue.hpp
  src/player.cpp
  src/player.hpp
  src/thread_safe_queue.hpp
  src/timer.cpp
  src/timer.hpp
  src/utils.cpp
  src/utils.hpp
  src/video_player.cpp
  src/video_player.hpp
)

add_library(av1player ${SRC_AOM_PLAYER})
target_link_libraries(av1player dav1d core_base libwebm ogg vorbis)
