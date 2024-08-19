# Main build file of rgu project
cmake_minimum_required(VERSION 3.20.1)

#--------------------------------------------------------------------------------
# Sound decoder sources
#--------------------------------------------------------------------------------

# Almost everything is "compiled" here, but things that don't apply to the
#  build are #ifdef'd out. This is to make it easy to embed SDL_sound into
#  another project or bring up a new build system: just compile all the source
#  code and #define the things you want.
set(SDLSOUND_SRCS
    src/SDL_sound.c
    src/SDL_sound_flac.c
    src/SDL_sound_mp3.c
    src/SDL_sound_raw.c
    src/SDL_sound_vorbis.c
    src/SDL_sound_wav.c
)

set(SDLSOUND_HDRS
    src/dr_flac.h
    src/dr_mp3.h
    src/SDL_sound.h
    src/SDL_sound_internal.h
    src/stb_vorbis.h
)

set(SDLSOUND_ALL_SRCS
    ${SDLSOUND_SRCS}
    ${SDLSOUND_HDRS}
    ${TIMIDITY_SRCS}
    ${TIMIDITY_HDRS}
)

add_library(sound_decoder STATIC ${SDLSOUND_ALL_SRCS})
target_include_directories(sound_decoder PUBLIC src)
