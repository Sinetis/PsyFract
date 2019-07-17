#include "stdafx.h"
#include "Signal.h"
#include "Zero.h"
#include "Capture.h"
#include "WavPlayer.h"

Signal::Signal()
{
	signals.s1 = new CaptureMic();
	signals.s2 = new Audio();
	signal = signals.s1;
	signal->Initialize();
	zero = new Zero();
	F = true;
}

int* Signal::GetSignal(int num, int channel)
{
	int *t;
	try
	{
		IsRead = true;
		t = signal->GetSignal(num, channel);
		if (F)
			for (int i = 0; i < num; i++)
				t[i] *= filter[i];
		IsRead = false;
	}
	catch(...)
	{
		t = zero->GetSignal(num, channel);
	}
	return t;
}
int Signal::Initialize()
{
	return signal->Initialize();
	InProcess = true;
	return 0;
} 
int Signal::Destroy()
{
	InProcess = false;
	Sleep(1000);
	return signal->Destroy();
}

int Signal::SetSignal(ISignal *pS)
{
	pS->Initialize();
	ISignal* t = signal;
	signal = pS;
	while (IsRead)
		Sleep(3);
	t->Destroy();

	return 0;
}

int Signal::CatchMessage(WPARAM key)
{
	switch(key)
	{
	case 'Z':
		SetSignal(signals.s1);
		break;
	case 'X':
		SetSignal(signals.s2);
		break;
	case 'Q':
		stop = stop ? false : true;
		break;
	case 'W':
		stop = stop ? false : true;
		Sleep(100);
		stop = stop ? false : true;
		break;
	case 'F':
		F = F ? false : true;
		break;
	}
		return 0;	
}