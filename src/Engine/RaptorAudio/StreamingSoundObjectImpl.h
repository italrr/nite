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

#ifndef STREAMING_SOUND_OBJECT_IMPL_H
	#define STREAMING_SOUND_OBJECT_IMPL_H

	#include "WaveoutDevice.h"
	#include "SoundObject.h"

	#include <iostream>

	namespace Raptor
	{
		namespace Audio
		{
			class StreamingSoundObject;

			class StreamingSoundObjectImpl
			{
			protected:
				StreamingSoundObjectImpl( StreamingSoundObject* parent )
				{}

				virtual ~StreamingSoundObjectImpl( void ) {};
				friend class StreamingSoundObject;

			protected:
				virtual void Update( void ) = 0;
				virtual SoundObjectResults::SoundObjectResult AdvancePosition( void ) = 0;
				virtual short GetCurrentSample( unsigned int num ) = 0;
				virtual const short* GetChannelPtr( unsigned int num ) = 0;
				virtual bool UpdateBuffer( void ) = 0;

			protected:
				StreamingSoundObject* m_Parent;

				unsigned int m_NumChannels;
				unsigned int m_FileSize;
				const char* m_FilePath;

				double m_GlobalPosition;
			};
		};
	};
#endif
