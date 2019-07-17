#pragma once

#include "Renderer.h"
#include "Interfaces.h"

using namespace Const;

class ComplexFrac: public IFractal
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
	ComplexFrac(Controller* c)
	{
		SetBehavior(c);

		channel = 0;
		n = 1024;
		type = 1;
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
		case 2:
			Julia();
			break;
		default:
			Mandelbroth();
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

	int Julia()
	{
		HRESULT hr = S_OK;

		int iter = (2)*10;
		float step;
		float width = 400.0F;
		int X = GetSystemMetrics(SM_CXSCREEN)/2,
			Y = GetSystemMetrics(SM_CYSCREEN)/2;
		float	xmin = -1.5f,
				xmax =  1.5f,
				ymin = -0.8f,
				ymax =  0.8f;
		float m = xmax - xmin;
		if (m < ymax - ymin)
			m = ymax - ymin;
		//m/=2;

		step = (1.0/width);

		pRend->pRT->Clear(D2D1::ColorF(D2D1::ColorF::White));
		pRend->SetColor(D2D1::ColorF(D2D1::ColorF::Black));

		bool enter = false;
		// Начало лямбда функции по рисованию множества
		std::function<void(int)> f=[=, &enter](int c)
		{
			float
				Xmin = (c==1)||(c==4) ? 0 : xmin,
				Xmax = (c==2)||(c==3) ? 0 : xmax,
				Ymin = (c==1)||(c==2) ? 0 : ymin,
				Ymax = (c==3)||(c==4) ? 0 : ymax;
			int r,g,b;

			ID2D1SolidColorBrush* br;
			pRend->pD2DRenderTarget->CreateSolidColorBrush(
					D2D1::ColorF(D2D1::ColorF::Black),
					&br);

			for (float x = Xmin; x <= Xmax; x += step){
			for (float y = Ymin; y <= Ymax; y += step)
			{
				Complex c(x, y), z( 0,0 ), f( sin(k/6.0)/2.0, cos(k/6.0)/2.0);
				int it = 0;
				while (it < iter && z.d() < iter<<2)
				{
					//z =	((z*z).exp()) + c; 
					//c = f*c;
					z = z*z + c;
					it += 1;
				}
				//it = 255 - (it*255.0)/iter;
				b = int(127 * cos(it/266.0)) + 128;
				g = int(127 * cos(it/15.0)) + 128;
				r = int(127 * cos(it/4.0)) + 128;
				br->SetColor(D2D1::ColorF((r<<16) + ((g)<<8) + b));
				pRend->DrawPoint(	X + (x - (xmax+xmin)/2)*width, 
									Y + (y - (ymax+ymin)/2)*width, 1, br);
			}} 
		};

		//f(1);
		//f(2);
		//f(3);
		//f(4);
		std::thread t1(f,1);
		std::thread t2(f,2);
		std::thread t3(f,3);
		std::thread t4(f,4);
		t1.join();
		t2.join();
		t3.join();
		t4.join();
	
		pRend->DrawNumber(k,10,10,20);
		pRend->SetColor(D2D1::ColorF(D2D1::ColorF::Gold, 0.25F));
	
		pRend->LineT(0, Y, X*2);
		pRend->LineT(X - 5, Y - width, 10);
		pRend->LineT(X - 5, Y + width, 10);
		pRend->LineTL(X, 0, Y*2);
		pRend->LineTL(X + width, Y - 5, 10);
		pRend->LineTL(X - width, Y - 5, 10);

		pRend->k++;

		return hr;
	}


float prev;
HRESULT Mandelbroth()
{
	HRESULT hr = S_OK;

	int iter = 2*(pRend->k%50+1);
	float step;
	float width = 300.0F;// + k*300*atan(2*k)/(Const::PI/2);// * (k+1);
	int X = GetSystemMetrics(SM_CXSCREEN)/2,
		Y = GetSystemMetrics(SM_CYSCREEN)/2;
	//float	xmin = -1.0,
	//		xmax = -0.85,
	//		ymin = -0.35,
	//		ymax = -0.25;
	float	xmin = -1.6,//+1.5*atan(2*k)/(Const::PI/2),
			xmax =  1.6,//-1.5*atan(2*k)/(Const::PI/2),
			ymin = -1.0,//+1.0*atan(2*k)/(Const::PI/2),
			ymax =  1.0;//-1.0*atan(2*k)/(Const::PI/2);

	step = (1.0/width);

	pRend->pRT->Clear(D2D1::ColorF(D2D1::ColorF::White));
	pRend->SetColor(D2D1::ColorF(D2D1::ColorF::Blue));

	bool enter = false;
	// Начало лямбда функции по рисованию множества
	std::function<void(int)> f=[=, &enter](int c)
	{
		float
			Xmin = (c==1)||(c==4) ? 0 : xmin,
			Xmax = (c==2)||(c==3) ? 0 : xmax,
			Ymin = (c==1)||(c==2) ? 0 : ymin,
			Ymax = (c==3)||(c==4) ? 0 : ymax;

		ID2D1SolidColorBrush* br;
		pRend->pD2DRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::Black),
				&br);

		for (float x = Xmin; x <= Xmax; x += step){
		for (float y = Ymin; y <= Ymax; y += step)
		{
			Complex c (x, y);
			float m = Mand(c, Complex(- 0.80 , 0.159), iter).d();
			//float m = Mand(c, Complex( 0.3573586489535 , 0.068531456458), iter).d();
			if ( m < 1+ sqrt(1 + 4*c.d())/2 )
			//if (m > prev)
			{
				//while(enter){}
				enter = true;
				//WORD* w = aud->GetFrequency(3);
				br->SetColor(D2D1::ColorF(	0,
											(prev-m),
											(m-prev)));//*/
				pRend->DrawPoint(	X + (x - (xmax+xmin)/2)*width, 
									Y + (y - (ymax+ymin)/2)*width, 1);
				enter = false;
			}
		}} 
	};

	//std::thread t1(f,1);
	//std::thread t2(f,2);
	//std::thread t3(f,3);
	//std::thread t4(f,4);
	//t1.join();
	//t2.join();
	//t3.join();
	//t4.join();
	f(1);
	f(2);
	f(3);
	f(4);
	
	//iter = 5000;
	//Complex z(1,1);
	//Complex c( 0.1 , 0.1);
	//while (iter > 0)
	//{
	//	z.Rotate(0.1);
	//	z.Scale(0.9);
	//	z.Inverse();
	//	iter--;
	//	pRT->FillRectangle(&Point(	X + (z.x - (xmax+xmin)/2)*width, 
	//								Y + (z.y - (ymax+ymin)/2)*width, 2), pBrush);
	//}
	
	pRend->DrawNumber(k,10,10,20);
	pRend->pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Gold, 0.25F));
	
	pRend->LineT(0, Y, X*2);
	pRend->LineT(X - 5, Y - width, 10);
	pRend->LineT(X - 5, Y + width, 10);
	pRend->LineTL(X, 0, Y*2);
	pRend->LineTL(X + width, Y - 5, 10);
	pRend->LineTL(X - width, Y - 5, 10);
	
	
	//DrawNumber(100*(mouseX-X + ((xmax+xmin)/2)*width)/width, 10, 60, 20);
	//DrawNumber(100*(mouseY-Y - ((ymax+ymin)/2)*width)/width, 10, 110, 20);

	pRend->k++;

	return hr;
}

	Complex Mand(Complex z, Complex c, int iter)
	{
		while (iter > 1)
		{
			z = z*z + c;
			iter--;
		}
		if (iter == 1)
		{
			prev = z.d();
			z = z*z + c;
		}
		return z;
	}


};