#pragma once

#include "stdafx.h"
class ISignal;
#include "Interfaces.h"

#define ui unsigned int;

// Отвечает за перехват звука с микрофона
class CaptureMic: public ISignal
{
	int NUMPTS;
	int sampleRate;
	int N;
	short int *waveIn;
	int pos;
	bool play;

	WAVEFORMATEX pFormat;
	HWAVEIN hWaveIn;
    WAVEHDR waveInHdr;
	HWAVEOUT hWaveOut;
    WAVEHDR WaveOutHdr;

public:
	int Initialize()
	{
		return 0;
	}
	int Destroy()
	{
		return 0;
	}

	CaptureMic()
	{
		play = true;
		sampleRate = 44100;//* 0.05 / 4;
		NUMPTS = sampleRate * 30*60;
		waveIn = new short int[NUMPTS];
		//for (int i = 0; i < NUMPTS; i++)
			//waveIn[i] = 0;

		pFormat.wFormatTag = WAVE_FORMAT_PCM;     // simple, uncompressed format
		pFormat.nChannels = 2;                    //  1=mono, 2=stereo
		pFormat.wBitsPerSample = 16;              //  16 for high quality, 8 for telephone-grade
		pFormat.nSamplesPerSec = sampleRate;     
		pFormat.nAvgBytesPerSec = sampleRate * pFormat.nChannels * pFormat.wBitsPerSample / 8;
		pFormat.nBlockAlign = pFormat.nChannels * pFormat.wBitsPerSample / 8;                 
		pFormat.cbSize = 0;
	
		waveInHdr.lpData = (LPSTR)waveIn;
		waveInHdr.dwBufferLength = NUMPTS;
		N = NUMPTS / 4;
		waveInHdr.dwBytesRecorded = 0;
		waveInHdr.dwUser = 0L;
		waveInHdr.dwFlags = 0L;
		waveInHdr.dwLoops = 0L;

		WaveOutHdr.lpData = (LPSTR)waveIn;
		WaveOutHdr.dwBufferLength = NUMPTS * 2;
		WaveOutHdr.dwBytesRecorded = 0;
		WaveOutHdr.dwUser = 0L;
		WaveOutHdr.dwFlags = 0L;
		WaveOutHdr.dwLoops = 0L;

		std::function<void()> f = [&]
		{
			while (play)
			{
				waveInOpen(&hWaveIn, WAVE_MAPPER, &pFormat, 0L, 0L, WAVE_MAPPED_DEFAULT_COMMUNICATION_DEVICE);
				waveInPrepareHeader(hWaveIn, &waveInHdr, sizeof(WAVEHDR));
				waveInAddBuffer(hWaveIn, &waveInHdr, sizeof(WAVEHDR));
				waveInStart(hWaveIn);
				do{Sleep(10);} 
				while(waveInUnprepareHeader(hWaveIn, &waveInHdr, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING);
				//{Sleep(10);} 
			}
		};
		std::thread tr(f);
		tr.detach();
	}

	int* GetSignal(int num, int channel)
	{
		//if (waveInUnprepareHeader(hWaveIn, &waveInHdr, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING)
			//return GetFrequency(num, channel);
		return Get(num, channel == 1);
		//if (play)
		//{
		//	waveInOpen(&hWaveIn, WAVE_MAPPER, &pFormat, 0L, 0L, WAVE_MAPPED_DEFAULT_COMMUNICATION_DEVICE);
		//	waveInPrepareHeader(hWaveIn, &waveInHdr, sizeof(WAVEHDR));
		//	waveInAddBuffer(hWaveIn, &waveInHdr, sizeof(WAVEHDR));
		//	waveInStart(hWaveIn);
		//	return GetFrequency(num, channel);
		//}
	}

	//channel: 1 or 2 or 3 or 4
	int* GetFrequency(int n, int channel = 1)
	{
		int *k = new int[n];
		channel--;
		pos = waveInHdr.dwBytesRecorded/4;

		int i, j=0,
		l = waveInHdr.dwBufferLength/4;
		for (i = pos-2; j < n && i < l && i > 0; i--, j++)
		{
			k[j] = (waveInHdr.lpData[i*4 - channel + 3]);
			k[j] = k[j]<<8;
			k[j] += (waveInHdr.lpData[i*4 - channel + 2]);
			k[j] = f(k[j]);
		}
		if (i == 0 && j < n)
		for (int h = l; j < n; h--, j++)
		{
			k[j] = (waveInHdr.lpData[h*4 - channel + 3]);
			k[j] = k[j]<<8;
			k[j] += (waveInHdr.lpData[h*4 - channel + 2]);
			k[j] = f(k[j]);
		}
		return k;
	}
	
	int* Get(int n, bool left)
	{
		int* k = new int[n];
		pos = waveInHdr.dwBytesRecorded-(waveInHdr.dwBytesRecorded%4)-3;
		if (!left) pos+=2;
		int i=0;
		for (int j = pos; j > -1 && j < NUMPTS && i < n; j-=4, i++)
		{
			k[i] = waveInHdr.lpData[j];
		}
		if (i<n)
		{
			int j = NUMPTS-3;
			if (!left) j += 2;
			for (; i < n; i++, j-=4)
			{
				k[i] = waveInHdr.lpData[j];
			}
		}
		return k;
	}

	inline int f(int k)
	{
		return k/256;
		//return sqrtf(k+65536/2)-313+128;
	}

	void sound()
	{
		waveOutOpen(&hWaveOut, WAVE_MAPPER, &pFormat, 0L, 0L, WAVE_FORMAT_DIRECT);
		waveOutPrepareHeader(hWaveOut,&WaveOutHdr,sizeof(WAVEHDR));
		waveOutSetVolume(hWaveOut,0xFFFF);
		waveOutWrite(hWaveOut,&WaveOutHdr,sizeof(WAVEHDR));
		waveOutClose(hWaveOut);
	}

	~CaptureMic()
	{
		play = false;
	}
};