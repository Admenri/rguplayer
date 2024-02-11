# Soloud Audio library: https://github.com/jarikomppa/soloud
# RGU Wrapper embed
cmake_minimum_required(VERSION 3.18.0)

set (SOLOUD_HEADER_PATH include)
set (SOLOUD_SOURCE_PATH src)

# Headers
set (SOLOUD_TARGET_HEADERS
	${SOLOUD_HEADER_PATH}/soloud.h
	${SOLOUD_HEADER_PATH}/soloud_audiosource.h
	${SOLOUD_HEADER_PATH}/soloud_bassboostfilter.h
	${SOLOUD_HEADER_PATH}/soloud_biquadresonantfilter.h
	${SOLOUD_HEADER_PATH}/soloud_bus.h
	${SOLOUD_HEADER_PATH}/soloud_dcremovalfilter.h
	${SOLOUD_HEADER_PATH}/soloud_echofilter.h
	${SOLOUD_HEADER_PATH}/soloud_error.h
	${SOLOUD_HEADER_PATH}/soloud_fader.h
	${SOLOUD_HEADER_PATH}/soloud_fft.h
	${SOLOUD_HEADER_PATH}/soloud_fftfilter.h
	${SOLOUD_HEADER_PATH}/soloud_file.h
	${SOLOUD_HEADER_PATH}/soloud_file_hack_off.h
	${SOLOUD_HEADER_PATH}/soloud_file_hack_on.h
	${SOLOUD_HEADER_PATH}/soloud_filter.h
	${SOLOUD_HEADER_PATH}/soloud_flangerfilter.h
	${SOLOUD_HEADER_PATH}/soloud_internal.h
	${SOLOUD_HEADER_PATH}/soloud_lofifilter.h
	${SOLOUD_HEADER_PATH}/soloud_monotone.h
	${SOLOUD_HEADER_PATH}/soloud_openmpt.h
	${SOLOUD_HEADER_PATH}/soloud_queue.h
	${SOLOUD_HEADER_PATH}/soloud_robotizefilter.h
	${SOLOUD_HEADER_PATH}/soloud_sfxr.h
	${SOLOUD_HEADER_PATH}/soloud_speech.h
	${SOLOUD_HEADER_PATH}/soloud_tedsid.h
	${SOLOUD_HEADER_PATH}/soloud_thread.h
	${SOLOUD_HEADER_PATH}/soloud_vic.h
	${SOLOUD_HEADER_PATH}/soloud_vizsn.h
	${SOLOUD_HEADER_PATH}/soloud_wav.h
	${SOLOUD_HEADER_PATH}/soloud_waveshaperfilter.h
	${SOLOUD_HEADER_PATH}/soloud_wavstream.h
)


# Core
set (SOLOUD_CORE_PATH ${SOLOUD_SOURCE_PATH}/core)
set (SOLOUD_CORE_SOURCES
	${SOLOUD_CORE_PATH}/soloud.cpp
	${SOLOUD_CORE_PATH}/soloud_audiosource.cpp
	${SOLOUD_CORE_PATH}/soloud_bus.cpp
	${SOLOUD_CORE_PATH}/soloud_core_3d.cpp
	${SOLOUD_CORE_PATH}/soloud_core_basicops.cpp
	${SOLOUD_CORE_PATH}/soloud_core_faderops.cpp
	${SOLOUD_CORE_PATH}/soloud_core_filterops.cpp
	${SOLOUD_CORE_PATH}/soloud_core_getters.cpp
	${SOLOUD_CORE_PATH}/soloud_core_setters.cpp
	${SOLOUD_CORE_PATH}/soloud_core_voicegroup.cpp
	${SOLOUD_CORE_PATH}/soloud_core_voiceops.cpp
	${SOLOUD_CORE_PATH}/soloud_fader.cpp
	${SOLOUD_CORE_PATH}/soloud_fft.cpp
	${SOLOUD_CORE_PATH}/soloud_fft_lut.cpp
	${SOLOUD_CORE_PATH}/soloud_file.cpp
	${SOLOUD_CORE_PATH}/soloud_filter.cpp
	${SOLOUD_CORE_PATH}/soloud_queue.cpp
	${SOLOUD_CORE_PATH}/soloud_thread.cpp
)


# Audiosources
set (SOLOUD_AUDIOSOURCES_PATH ${SOLOUD_SOURCE_PATH}/audiosource)
set (SOLOUD_AUDIOSOURCES_SOURCES
	${SOLOUD_AUDIOSOURCES_PATH}/monotone/soloud_monotone.cpp
	${SOLOUD_AUDIOSOURCES_PATH}/openmpt/soloud_openmpt.cpp
	${SOLOUD_AUDIOSOURCES_PATH}/openmpt/soloud_openmpt_dll.c
	${SOLOUD_AUDIOSOURCES_PATH}/sfxr/soloud_sfxr.cpp
	${SOLOUD_AUDIOSOURCES_PATH}/speech/darray.cpp
	${SOLOUD_AUDIOSOURCES_PATH}/speech/klatt.cpp
	${SOLOUD_AUDIOSOURCES_PATH}/speech/resonator.cpp
	${SOLOUD_AUDIOSOURCES_PATH}/speech/soloud_speech.cpp
	${SOLOUD_AUDIOSOURCES_PATH}/speech/tts.cpp
	${SOLOUD_AUDIOSOURCES_PATH}/tedsid/sid.cpp
	${SOLOUD_AUDIOSOURCES_PATH}/tedsid/soloud_tedsid.cpp
	${SOLOUD_AUDIOSOURCES_PATH}/tedsid/ted.cpp
	${SOLOUD_AUDIOSOURCES_PATH}/vic/soloud_vic.cpp
	${SOLOUD_AUDIOSOURCES_PATH}/vizsn/soloud_vizsn.cpp
	${SOLOUD_AUDIOSOURCES_PATH}/wav/dr_impl.cpp
	${SOLOUD_AUDIOSOURCES_PATH}/wav/soloud_wav.cpp
	${SOLOUD_AUDIOSOURCES_PATH}/wav/soloud_wavstream.cpp
	${SOLOUD_AUDIOSOURCES_PATH}/wav/stb_vorbis.c
)

# Filters
set (SOLOUD_FILTERS_PATH ${SOLOUD_SOURCE_PATH}/filter)
set (SOLOUD_FILTERS_SOURCES
	${SOLOUD_FILTERS_PATH}/soloud_bassboostfilter.cpp
	${SOLOUD_FILTERS_PATH}/soloud_biquadresonantfilter.cpp
	${SOLOUD_FILTERS_PATH}/soloud_dcremovalfilter.cpp
	${SOLOUD_FILTERS_PATH}/soloud_echofilter.cpp
	${SOLOUD_FILTERS_PATH}/soloud_fftfilter.cpp
	${SOLOUD_FILTERS_PATH}/soloud_flangerfilter.cpp
	${SOLOUD_FILTERS_PATH}/soloud_lofifilter.cpp
	${SOLOUD_FILTERS_PATH}/soloud_robotizefilter.cpp
	${SOLOUD_FILTERS_PATH}/soloud_waveshaperfilter.cpp
)

# All together
set (SOLOUD_TARGET_SOURCES
	${SOLOUD_TARGET_HEADERS}
	${SOLOUD_CORE_SOURCES}
	${SOLOUD_AUDIOSOURCES_SOURCES}
	${SOLOUD_FILTERS_SOURCES}
)

add_library(soloud STATIC ${SOLOUD_TARGET_SOURCES})
target_include_directories(soloud PUBLIC include)
