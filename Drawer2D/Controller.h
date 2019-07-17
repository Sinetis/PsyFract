#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include "Renderer.h"
#include "Signal.h"
class Renderer;
class Signal;

class Controller
{
	Renderer* pRend;
	Signal* pSig;
public:
	Renderer* Rend(){return pRend;}
	Signal* Sig(){return pSig;}
	Controller(HWND hwnd, int width, int height, bool fullscreen);

	~Controller();

	void CatchMessage(WPARAM key);

private:

};


#endif
