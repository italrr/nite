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

#ifndef WAVEOUT_DEVICE_H
	#define WAVEOUT_DEVICE_H

	#include "RingBuffer.h"
	#include "BlockBuffer.h"

	#include <Windows.h>
	#include <MMSystem.h>

	namespace Raptor
	{
		namespace Audio
		{
			class WaveoutDevice
			{
			public:
				WaveoutDevice( unsigned int sampleRate, unsigned int sampleAmount );
				~WaveoutDevice( void );

			public:
				unsigned int GetSampleRate( void );
				void StopDevice( void );

			public:
				HANDLE GetWaveoutThreadHandle( void );

			public:
				HANDLE m_WaveoutBufferSwapEvent;
				HANDLE m_WaveoutReadyEvent;

			private:
				HANDLE m_WaveoutThreadHandle;
				HANDLE m_WaveoutStopEvent;
				HANDLE m_WaveoutWriteStopEvent;
				HWAVEOUT m_Waveout;

				BlockBuffer* m_BlockBuffer[2];
				WAVEHDR m_BlockBufferHeader[2];
				unsigned int m_CurrentBlock;

				unsigned int m_SampleRate;

				friend void CALLBACK WaveoutCallback( HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2 );
				friend DWORD WINAPI WaveoutBlockBufferThreadFunction( WaveoutDevice* wvOut );
			};
		};
	};
#endif
