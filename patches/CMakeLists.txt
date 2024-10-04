# Apply patch for third-party
cmake_minimum_required(VERSION 3.20)

function(apply_git_patch patch_file work_dir)
  execute_process(
    COMMAND git apply ${patch_file}
    WORKING_DIRECTORY ${work_dir}
    RESULT_VARIABLE result
    ERROR_VARIABLE error_output
  )
endfunction()

# BGFX OpenGL Scissor Fix
apply_git_patch(${PROJECT_SOURCE_DIR}/patches/bgfx_scissor.patch ${PROJECT_SOURCE_DIR}/third_party/bgfx.cmake/bgfx)
