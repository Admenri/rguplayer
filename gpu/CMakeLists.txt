# GPU defination
cmake_minimum_required(VERSION 3.18.0)

#--------------------------------------------------------------------------------
# Autogen script
#--------------------------------------------------------------------------------

set(PYTHON_EXECUTABLE python)

# Export header file import
execute_process(
  COMMAND ${PYTHON_EXECUTABLE} ${SRC_ROOT}/gpu/gles2/build_gles2_command_buffer.py --header
  WORKING_DIRECTORY ${SRC_ROOT}/gpu/gles2
  RESULT_VARIABLE PYTHON_SCRIPT_RESULT
  OUTPUT_QUIET
)

if(NOT PYTHON_SCRIPT_RESULT EQUAL 0)
  message(FATAL_ERROR "Failed to generate gles2 header: ${PYTHON_SCRIPT_RESULT}")
endif()

# Export body file import
execute_process(
  COMMAND ${PYTHON_EXECUTABLE} ${SRC_ROOT}/gpu/gles2/build_gles2_command_buffer.py --body
  WORKING_DIRECTORY ${SRC_ROOT}/gpu/gles2
  RESULT_VARIABLE PYTHON_SCRIPT_RESULT
  OUTPUT_QUIET
)

if(NOT PYTHON_SCRIPT_RESULT EQUAL 0)
  message(FATAL_ERROR "Failed to generate gles2 body file: ${PYTHON_SCRIPT_RESULT}")
endif()

#--------------------------------------------------------------------------------
# Generate shader embed
#--------------------------------------------------------------------------------

file(GLOB shader_files "${SRC_ROOT}/gpu/shader/*")
foreach(shader_file ${shader_files})
  get_filename_component(file_extension ${shader_file} EXT)
  if (file_extension STREQUAL ".h" OR file_extension STREQUAL ".vert" OR file_extension STREQUAL ".frag")
    execute_process(
      COMMAND ${PYTHON_EXECUTABLE} ${SRC_ROOT}/buildtools/build_file_binary.py ${shader_file}
      WORKING_DIRECTORY ${SRC_ROOT}/gpu/shader
      RESULT_VARIABLE SHADER_GENERATE_RESULT
      OUTPUT_QUIET
    )

    if(NOT SHADER_GENERATE_RESULT EQUAL 0)
      message(FATAL_ERROR "Failed to generate shader embed file: ${SHADER_GENERATE_RESULT}")
    endif()
  endif()
endforeach()

#--------------------------------------------------------------------------------
# Define gpu source
#--------------------------------------------------------------------------------

set(GPU_SRC
  gles2/gles_context.cc
  gles2/gles_context.h
)

add_library(core_gpu ${GPU_SRC})
target_link_libraries(core_gpu core_base SDL2::Core)
