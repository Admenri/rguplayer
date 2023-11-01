# GPU defination
cmake_minimum_required(VERSION 3.18.0)

#--------------------------------------------------------------------------------
# Define gpu source
#--------------------------------------------------------------------------------

set(GPU_SRC
  gles2/context/gles_context.cc
  gles2/context/gles_context.h
  gles2/draw/quad_array.h
  gles2/draw/quad_drawable.cc
  gles2/draw/quad_drawable.h
  gles2/gsm/gles_gsm.cc
  gles2/gsm/gles_gsm.h
  gles2/gsm/state_stacks.cc
  gles2/gsm/state_stacks.h
  gles2/shader/shader_manager.cc
  gles2/shader/shader_manager.h
  gles2/meta/gles_meta.cc
  gles2/meta/gles_meta.h
  gles2/vertex/vertex_array.cc
  gles2/vertex/vertex_array.h
  gl_forward.h
)

#--------------------------------------------------------------------------------
# Autogen script
#--------------------------------------------------------------------------------

set(PYTHON_EXECUTABLE python)

# Export header file import
execute_process(
  COMMAND ${PYTHON_EXECUTABLE} ${SRC_ROOT}/gpu/gles2/context/build_gles2_command_buffer.py --file=gles2_commands.txt --header --output=gles2_command_buffer_header_autogen.h
  WORKING_DIRECTORY ${SRC_ROOT}/gpu/gles2/context
  RESULT_VARIABLE PYTHON_SCRIPT_RESULT
  OUTPUT_QUIET
)

if(NOT PYTHON_SCRIPT_RESULT EQUAL 0)
  message(FATAL_ERROR "Failed to generate gles2 header: ${PYTHON_SCRIPT_RESULT}")
endif()

# Export body file import
execute_process(
  COMMAND ${PYTHON_EXECUTABLE} ${SRC_ROOT}/gpu/gles2/context/build_gles2_command_buffer.py --file=gles2_commands.txt --body --output=gles2_command_buffer_autogen.cc
  WORKING_DIRECTORY ${SRC_ROOT}/gpu/gles2/context
  RESULT_VARIABLE PYTHON_SCRIPT_RESULT
  OUTPUT_QUIET
)

if(NOT PYTHON_SCRIPT_RESULT EQUAL 0)
  message(FATAL_ERROR "Failed to generate gles2 body file: ${PYTHON_SCRIPT_RESULT}")
endif()

#--------------------------------------------------------------------------------
# Generate shader embed
#--------------------------------------------------------------------------------

file(GLOB shader_files "${SRC_ROOT}/gpu/gles2/shader/shader_source/*")
foreach(shader_file ${shader_files})
  get_filename_component(file_extension ${shader_file} EXT)
  if (file_extension STREQUAL ".vert" OR file_extension STREQUAL ".frag")
    execute_process(
      COMMAND ${PYTHON_EXECUTABLE} ${SRC_ROOT}/buildtools/build_file_binary.py ${shader_file}
      WORKING_DIRECTORY ${SRC_ROOT}/gpu/gles2/shader/shader_source
      RESULT_VARIABLE SHADER_GENERATE_RESULT
      OUTPUT_QUIET
    )

    if(NOT SHADER_GENERATE_RESULT EQUAL 0)
      message(FATAL_ERROR "Failed to generate shader embed file: ${SHADER_GENERATE_RESULT}")
    endif()
  endif()
endforeach()

add_library(core_gpu ${GPU_SRC})
target_link_libraries(core_gpu core_base SDL2::Core)
