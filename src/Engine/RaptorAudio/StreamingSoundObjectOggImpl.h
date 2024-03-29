/*
	Copyright (c) 2014 Sam Hardeman

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
*/

#ifndef STREAMING_SOUND_OBJECT_OGG_IMPL_H
	#define STREAMING_SOUND_OBJECT_OGG_IMPL_H

	#include "WaveoutDevice.h"
	#include "SoundObject.h"

	#include "StreamingSoundObjectImpl.h"

	#include <iostream>
	#include "stb_vorbis.h"

	namespace Raptor
	{
		namespace Audio
		{
			class StreamingSoundObject;

			struct OggBuffer
			{
				OggBuffer( void )
				{
					ob_BufferPtrs[0] = 0;
					ob_BufferPtrs[1] = 0;
				}

				~OggBuffer( void )
				{
					delete [] ob_BufferPtrs[0];
					delete [] ob_BufferPtrs[1];

					ob_BufferPtrs[0] = 0;
					ob_BufferPtrs[1] = 0;
				}

				short* ob_BufferPtrs[2];
				unsigned int ob_BufferSize;
			};

			class StreamingSoundObjectOggImpl : protected StreamingSoundObjectImpl
			{
			protected:
				StreamingSoundObjectOggImpl( StreamingSoundObject* parent );
				~StreamingSoundObjectOggImpl( void );
				friend class StreamingSoundObject;

			protected:
				void Update( void ) {};
				SoundObjectResults::SoundObjectResult AdvancePosition( void );
				short GetCurrentSample( unsigned int num );
				const short* GetChannelPtr( unsigned int num );
				bool UpdateBuffer( void );

			protected:
				OggBuffer m_OggBuffers[2];

				unsigned int m_BufferSize;
				unsigned int m_TotalSize;
				unsigned int m_CurrentBuffer;
				unsigned int m_BufferCountdown;

				bool m_LoopDone;
				stb_vorbis* m_OggHandle;
			};
		};
	};
#endif
