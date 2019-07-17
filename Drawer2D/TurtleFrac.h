#ifndef _TURTLEFRAC_H_
#define _TURTLEFRAC_H_

#include "Renderer.h"
#include "Interfaces.h"

using namespace Const;

class TurtleFrac: public IFractal
{
	// Номер канала для отрисовки
	int channel;
	// Размер массива частот
	int n;
	// Массивы частот
	int* lw[4];
	// Индексаторы частот
	//int f[2];
	// Тип Отрисовки
	int type;
	// Просто какая-то переменная
	int s, k;

public:
	TurtleFrac(Controller* c)
	{
		SetBehavior(c);

		channel = 0;
		n = 1024;
		type = 0;
		this->s = 5;
		k = 0;
	}

	int Initialize()
	{
		return 0;
	}
	int Destroy()
	{
		return 0;
	}

	int Draw()
	{
		switch(type)
		{
		default:
			Trian();
			break;
		}
		return 0;
	}

	int CatchMessage(WPARAM key)
	{
		switch(key)
		{
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '0':
			type = key - '0';


		default:
			break;
		}
		return 0;
	}

	HRESULT Trian()
	{
		HRESULT hr = S_OK;

		int iter = 100;
		int X = GetSystemMetrics(SM_CXSCREEN)/2,
			Y = GetSystemMetrics(SM_CYSCREEN)/2;
		D2D1_POINT_2F a, b, c;
		a = D2D1::Point2F( X, Y );
		b = a; b.y -= 300;
		Turtle turtle(pRend);
		turtle.Go(a.x, a.y);
		int *f = pSig->GetSignal(200), s=0;
		for (int i = 0; i < 200; i++)
			if((f[i] < 2 && f[i] > -2) || (f[i] < -512 && f[i] > 512))
				f[i] = 0;
		turtle.Direction(*f);
		turtle.Color(D2D1::ColorF(.99,.99,.99));

		pRend->pRT->Clear(D2D1::ColorF(D2D1::ColorF::Black));

		//pRT->DrawLine(a, b, pBrush);
		srand(time(0));

		for (int i = 0; i < iter; i++)
		{
			turtle.Forward(30*(float(i)/iter));
			turtle.Rotate(f[(++s)]*4);
			turtle.Forward(-3*(float(i)/iter));
			turtle.Rotate(-f[(++s)]/20.0);
		}

		pRend->DrawNumber(k, 10, 10, 20);
		pRend->DrawNumber(*pSig->GetSignal(1), 10, 60, 20);
		delete []f;
		if(pRend->k++ > 360)
			pRend->k = 0;
		Sleep(00);

		return hr;
	}

	void Func(Turtle turtle, float c, int iter)
		{
			if (iter == 0 || c <= 1)
				return;
			turtle.LineWidth(c/10);
			turtle.Color(D2D1::ColorF(1.0-c/60.0,0.3,1.0-c/60.0));

			turtle.Forward(c);

			//turtle.Save();
			//turtle.Rotate(rand()%45);
			//turtle.Func(c/2, iter - 1);
			//turtle.Restore();

			//turtle.Forward(c);

			turtle.Save();
			turtle.Rotate(-rand()%45);
			turtle.Func(c/1.3, iter - 1);
			turtle.Restore();

			turtle.Rotate(rand()%45);
			turtle.Func(c/1.3, iter - 1);
		}
};
#endif