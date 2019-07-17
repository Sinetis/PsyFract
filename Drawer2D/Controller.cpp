#include "stdafx.h"
#include "Controller.h"

	Controller::Controller(HWND hwnd, int width, int height, bool fullscreen)
	{
		pSig =  new Signal();
		pRend = new Renderer(this, hwnd, width, height, fullscreen);
		pRend->CreateFractals();
		pSig->StartCalcFFT();
		pRend->StartRender();
	}

	Controller::~Controller()
	{
		pRend->DiscardResources();
		delete pRend;
		delete pSig;
	}

	void Controller::CatchMessage(WPARAM key)
	{
		pSig->CatchMessage(key);
		pRend->CatchMessage(key);
	}