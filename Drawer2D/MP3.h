#pragma once

#include "stdafx.h"
#include "Interfaces.h"

class MP3: ISignal
{
public:
	int Initialize() override
	{return 0;}
	int Destroy() override
	{return 0;}
	int* GetSignal(int num, int channel = 1) override
	{return NULL;}
};