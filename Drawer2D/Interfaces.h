#ifndef _INTERFACES_H_
#define _INTERFACES_H_

#include "stdafx.h"

class Renderer;
class Signal;
class Controller;

class IFractal
{
protected:
	Renderer* pRend;
	Signal* pSig;
	
public:
	virtual int Initialize() = 0;
	virtual int CatchMessage(WPARAM key) = 0;
	virtual int Draw() = 0;
	virtual int Destroy() = 0;

	void SetBehavior(Renderer* r, Signal* s);
	void SetBehavior(Controller* c);
};

// Класс от которого наследуются все классы, передающие сигналы.
class ISignal
{protected:
public:
	virtual int Initialize() = 0;
	virtual int Destroy() = 0;
	virtual int* GetSignal(int num, int channel=1) = 0;
};

#endif