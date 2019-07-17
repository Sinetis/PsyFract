#ifndef _SIGNAL_H_
#define _SIGNAL_H_

#include "Interfaces.h"
#include "Complex.h"

class CaptureMic;
class Zero;
class Audio;

class Signal
{
	struct _str
	{
		CaptureMic*	s1;
		Audio*		s2;
	} signals;

	ISignal* signal;
	Zero* zero;

	float* filter;
	bool F;

public:
	Signal();

	int* GetSignal(int num, int channel=1);
	int Initialize();
	int Destroy();

	int SetSignal(ISignal *pS);
	int CatchMessage(WPARAM key);
	
	bool IsRead;
	bool InProcess;

	// функции и переменные для потока БПФ
private:
	float *driver, *prevDriver;
	float *current, *prevCurrent;
	int size, slow;
	bool calc, trans; 

	float coef;
	float *logFFT;

	void calcFFT()
	{
		ResizeDriver(size);
		while (InProcess)
		{
			if (!stop)
			{
				int *first, *second;
				float *F, *S;
				first = GetSignal(size, 1);
				second = GetSignal(size, 3);
				
				F = FFT::ReturnFloat(first, size);
				S = FFT::ReturnFloat(second, size);

				delete[] first;
				delete[] second;
			
				delete[] current;
				current = new float[size];
				for (int i = 0; i < size; i++)
					current[i] = (S[i] + F[i])/2; 
				delete[] F;
				delete[] S;
			}

			for (int i = 0, N = (size>>1); i < N; i++)
				if (driver[i] < current[i])
				{
					//driver[i] = current[i];
					driver[i] += (current[i] - driver[i])*.2;
					//if (driver[i] > current[i])
					//	driver[i] = current[i];
				}
				else 
				{
					driver[i] += (current[i] - driver[i])*0.1;
					//driver[i] *= 0.95;
					//if (driver[i] < current[i])
					//	driver[i] = current[i];
				}

			while(trans)
				Sleep(1);
			calc = true;
			memcpy(prevCurrent, current, sizeof(float)*size);
			memcpy(prevDriver, driver, sizeof(float)*(size>>1));
			calc = false;

			Sleep(slow);
		}
	}
public:
	bool stop;
	void StartCalcFFT()
	{
		driver = prevDriver =
		current = prevCurrent = NULL;
		size = 1024;
		coef = (size / log10f(size / 2)) / 2;
		slow = 1;
		calc = trans = false; 
		stop = 0;

		std::thread t([&]{this->calcFFT();});
		t.detach();
	}

	int* GetFFT()
	{
		int *fft = new int[size >> 1];

		while (calc)
			Sleep(1);

		trans = true;
		for (int i = 0, N = (size >> 1); i < N; i++)
			fft[i] = (int)(prevDriver[i]);
		trans = false;

		return fft;
	}

	int* GetLogFFT()
	{
		int *fft = new int[size >> 1];
		for (int i = 0, N = (size >> 1); i < N; i++)
		{
			fft[i] = 0;
		}

		while (calc)
			Sleep(1);

		trans = true;
		int x;
		for (int i = 0, N = (size >> 1); i < N; i++)
		{
			x = coef * log10f(i);
			fft[x] += (int)(prevDriver[i]);
		}
		trans = false;

		int j = 0;
		for (int i = 1, N = (size >> 1); i < N; i+=j)
		{
			j = 1;
			if (fft[i] == 0)
			{
				while (fft[i + j] == 0)
					j++;
				for (int k = 0; k < j; k++)
					fft[i + k] = fft[i - 1] / j;
				fft[i - 1] /= j;
			}
		}

		return fft;
	}

	void ResizeDriver(int n)
	{
		if (driver)
			delete[] driver;

		size = n;
		coef = (size / log10f(size / 2)) / 2;
		driver = new float[size>>1];
		prevDriver = new float[size>>1];
		prevCurrent = new float[size];
		filter = new float[size];
		for (int i = 0; i < size; i++)
			filter[i] = (1-cos(2*PI*i/(size-1)))/2;
	}
	void SetSlowdown(int s)
	{
		slow = s;
	}
	int GetDriverSize()
	{
		return size>>1;
	}
};

#endif