#ifndef _ZERO_H_
#define _ZERO_H_

#include "Interfaces.h"

class Zero: ISignal
{
public:
	Zero()
	{
		char* str = "Empty";
	}
	int Initialize() override
	{return 0;}
	int Destroy() override
	{return 0;}
	int* GetSignal(int num, int channel = 1) override
	{
		int* s = new int[num];
		for(int i = 0; i < num; i++)
			s[i] = 0;
		return s;
	}
};

#endif