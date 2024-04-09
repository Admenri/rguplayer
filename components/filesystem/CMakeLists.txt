# Components - filesystem
cmake_minimum_required(VERSION 3.18.0)

#--------------------------------------------------------------------------------
# Source of filesystem
#--------------------------------------------------------------------------------

set(COMPONENTS_FILESYSTEM_SRC
  filesystem.cc
  filesystem.h
)

add_library(components_filesystem ${COMPONENTS_FILESYSTEM_SRC})
if (USE_PRIVATE_ADMENRI_ARCHIVER_CRYPT)
  set(CRYPTO_LIB admenri_crypto)
  target_compile_definitions(components_filesystem PRIVATE USE_ADMENRI_ARCHIVER=1)
else()
  set(CRYPTO_LIB)
endif()
target_link_libraries(components_filesystem physfs-static core_base ${CRYPTO_LIB})
