#include "stdafx.h"
#include "Interfaces.h"
#include "Controller.h"

void IFractal::SetBehavior(Renderer* r, Signal* s)
	{
		pRend = r;
		pSig = s;
	}
void IFractal::SetBehavior(Controller* c)
	{
		pRend = c->Rend();
		pSig = c->Sig();
	}