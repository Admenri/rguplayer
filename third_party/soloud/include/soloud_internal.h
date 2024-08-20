/*
SoLoud audio engine
Copyright (c) 2013-2015 Jari Komppa

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.
*/

#ifndef SOLOUD_INTERNAL_H
#define SOLOUD_INTERNAL_H

#include "soloud.h"

namespace SoLoud
{
	// Interlace samples in a buffer. From 11112222 to 12121212
	void interlace_samples_float(const float *aSourceBuffer, float *aDestBuffer, unsigned int aSamples, unsigned int aChannels, unsigned int aStride);

	// Convert to 16-bit and interlace samples in a buffer. From 11112222 to 12121212
	void interlace_samples_s16(const float *aSourceBuffer, short *aDestBuffer, unsigned int aSamples, unsigned int aChannels, unsigned int aStride);
};

#define FOR_ALL_VOICES_PRE \
		handle *h_ = NULL; \
		handle th_[2] = { aVoiceHandle, 0 }; \
		lockAudioMutex_internal(); \
		h_ = voiceGroupHandleToArray_internal(aVoiceHandle); \
		if (h_ == NULL) h_ = th_; \
		while (*h_) \
		{ \
			int ch = getVoiceFromHandle_internal(*h_); \
			if (ch != -1)  \
			{

#define FOR_ALL_VOICES_POST \
			} \
			h_++; \
		} \
		unlockAudioMutex_internal();

#define FOR_ALL_VOICES_PRE_3D \
		handle *h_ = NULL; \
		handle th_[2] = { aVoiceHandle, 0 }; \
		h_ = voiceGroupHandleToArray_internal(aVoiceHandle); \
		if (h_ == NULL) h_ = th_; \
				while (*h_) \
						{ \
			int ch = (*h_ & 0xfff) - 1; \
			if (ch != -1 && m3dData[ch].mHandle == *h_)  \
						{

#define FOR_ALL_VOICES_POST_3D \
						} \
			h_++; \
						} 

#define FOR_ALL_VOICES_PRE_EXT \
		handle *h_ = NULL; \
		handle th_[2] = { aVoiceHandle, 0 }; \
		mSoloud->lockAudioMutex_internal(); \
		h_ = mSoloud->voiceGroupHandleToArray_internal(aVoiceHandle); \
		if (h_ == NULL) h_ = th_; \
		while (*h_) \
		{ \
			int ch = mSoloud->getVoiceFromHandle_internal(*h_); \
			if (ch != -1)  \
			{

#define FOR_ALL_VOICES_POST_EXT \
			} \
			h_++; \
		} \
		mSoloud->unlockAudioMutex_internal();

#define FOR_ALL_VOICES_PRE_3D_EXT \
		handle *h_ = NULL; \
		handle th_[2] = { aVoiceHandle, 0 }; \
		h_ = mSoloud->voiceGroupHandleToArray(aVoiceHandle); \
		if (h_ == NULL) h_ = th_; \
				while (*h_) \
						{ \
			int ch = (*h_ & 0xfff) - 1; \
			if (ch != -1 && mSoloud->m3dData[ch].mHandle == *h_)  \
						{

#define FOR_ALL_VOICES_POST_3D_EXT \
						} \
			h_++; \
						} 

#endif
