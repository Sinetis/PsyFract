#ifndef _IFSYSTEM_H_
#define _IFSYSTEM_H_

#include "Renderer.h"
#include "Interfaces.h"

using namespace Const;

struct my
{
 float a,b,c,d,e,f, p;
 my(float a,float b,float c,float d,float e,float f,float p):
	 a(a),b(b),c(c),d(d),e(e),f(f),p(p){};
 my():p(0){};
};

class IFSystem: public IFractal
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
	int s, col;
	//
	int Xoffset, Yoffset;

	float scale, count;

public:
	IFSystem(Controller* c)
	{
		SetBehavior(c);

		channel = 0;
		n = 1024;
		type = 0;
		this->s = 0;
		col = 0;
		Xoffset = Yoffset = 0;
		scale = count = 1;
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
			RenderIFS2();
			break;
		case 3:
			LineIFS();
			break;
		default:
			RenderIFS();
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
			break;
		case 'Q':
			Xoffset = Yoffset = 0;
			scale = count = 1;
			if (++s > 4)
				s = 0;
			break;
		case VK_NUMPAD2:
			Yoffset -= 50;
			break;
		case VK_NUMPAD4:
			Xoffset += 50;
			break;
		case VK_NUMPAD8:
			Yoffset += 50;
			break;
		case VK_NUMPAD6:
			Xoffset -= 50;
			break;

		case VK_NUMPAD9:
			scale *= 1.1;
			break;
		case VK_NUMPAD3:
			scale *= 0.9;
			break;
		case VK_NUMPAD7:
			count *= 1.1;
			break;
		case VK_NUMPAD1:
			count *= 0.9;
			break;
		default:
			break;
		}
		return 0;
	}

D2D1_POINT_2F T(D2D1_POINT_2F P,float a,float b,float c,float d,float e,float f)
{
	float	x = a*P.x + b*P.y + e,
			y = c*P.x + d*P.y + f;
	D2D1_POINT_2F N;
	N.x = x;
	N.y = y;
	return N;
}

D2D1_POINT_2F T(D2D1_POINT_2F P, my* t)
{
	float	x = t->a*P.x + t->b*P.y + t->e,
			y = t->c*P.x + t->d*P.y + t->f;
	D2D1_POINT_2F N;
	N.x = x;
	N.y = y;
	return N;
}

D2D1_POINT_2F T(D2D1_POINT_2F P, my args, ...)
{
	my *t = &args;
	float v = GetRandomDouble(1);
	float pv = 0;
	while(t->p)
	{
		if (v >= pv && v < (pv + t->p) )
			return T(P, t);
		col++;
		pv += t->p;
		t++;
	}

	return D2D1::Point2F();
}

D2D1_POINT_2F GetTransform(D2D1_POINT_2F a,int s)
{
	switch (s)
			{
			case 0:
			return T(a,
					my( 0.024000,  0.000000,  0.000000, 0.432000, -0.036000, -0.748000, 0.011383),
					my( 0.767883,  0.014660, -0.013403, 0.839872, -0.058041,  1.703451, 0.708329),
					my(-0.058172,  0.359454,  0.329910, 0.063381,  0.178422,  2.002845, 0.134255),
					my( 0.078732, -0.370260,  0.341029, 0.085481, -0.077863,  2.091658, 0.146031),
					my());//*/
				break;
			case 1:
			return T(a,
					my(0.490909, -0.718182, -0.749495, -0.390657,  3.873884, 5.497090, 0.751886),
					my(0.395960,  0.166919,  0.327273, -0.470455, -4.724951, 5.882151, 0.248114),
					my());//*/
				break;
			case 2:
			return T(a,
				my(0, 0, 0, 0.5, 0, 0.00, 0.05),
				my(0.42, -0.42, 0.42, 0.42, 0, 4.00, 0.40),
				my(0.42, 0.42, -0.42, 0.42, 0, 4.00, 0.40),
				my(0.10, lw[0][5]/100.0, -lw[0][1]/100.0, 0.10, 0, 4.00, 0.15),
				my());//*/
				break;
			case 3:
				return T(a,
					my(0.49, -0.01,  0,   0.62, 0.25,  0.02, 0.316),
					my(0.27, -0.52,  0.4, 0.36, 0,    -0.56, 0.316),
					my(0.18,  0.73, -0.5, 0.26, 0.88, -0.08, 0.316),
					my(0.04,  0.01, -0.5, 0,    0.52, -0.32, 0.052),
					my());//*/
				break;
			case 4:
				return T(a,
					my(0.49, -0.01,  0,   0.62, 1.,  0.08, 0.316),
					my(0.27, -0.52,  0.4, 0.36, 0,    -2.24, 0.316),
					my(0.18,  0.73, -0.5, 0.26, 3.52, -0.32, 0.316),
					my(0.04,  0.01, -0.5, 0,    2.08, -1.28, 0.052),
					my());//*/
				break;
			default:
			return T(a,
				my(0.00, 0.00, 0.00, 0.50, 0, 0.00, 0.05),
				my(0.42, -0.42, 0.42, 0.42, 0, 0.20, 0.40),
				my(0.42, 0.42, -0.42, 0.42, 0, 0.20, 0.40),
				my(0.10, 0.00, 0.00, 0.10, 0, 0.20, 0.15),
				my());//*/
				break;
			}
}


HRESULT RenderIFS()
{
	HRESULT hr = S_OK;

	int iter = 100;//k*5;
	int X = GetSystemMetrics(SM_CXSCREEN)/2 + Xoffset,
		Y = GetSystemMetrics(SM_CYSCREEN)/2 + Yoffset;
	int size = 50*scale;
	int width = 50*count;
	D2D1_POINT_2F a, b, c;
	a = D2D1::Point2F( 0, 0 );
	b = a; b.y -= 300;
	lw[0] = pSig->GetSignal(50, 0);
	//pRend->pRT->Clear(D2D1::ColorF(D2D1::ColorF::Black));
	//pRend->SetColor(D2D1::ColorF(D2D1::ColorF::White));
	
		pRend->SetColor(D2D1::ColorF(1.f,1.f,1.f,1.2f));
		pRend->pRT->FillRectangle(D2D1::RectF(0,0, X*2,Y*2), pRend->pBrush); 

	//pRT->DrawRectangle(Point(a.x, a.y, 1), pBrush);
	int	x=0,y=0;
	for (int j = 0; j < 1e+5; j++)
	{
		int c;
		col = 0;
		a.x = x;
		a.y = y;
		for (int i = 0; i < iter; i++)
		{
			a = GetTransform(a, s);
			if (i==0) c = col;
		}

		//pRend->SetColor(D2D1::ColorF(	fabs(sin(c/1.)+.2),
		//								fabs(sin(c/2.)+.2),
		//								fabs(sin(c/3.)+.2)));

		switch (c)
		{
		case 0:
			pRend->SetColor(D2D1::ColorF(D2D1::ColorF::Aquamarine));
			break;
		case 1:
			pRend->SetColor(D2D1::ColorF(D2D1::ColorF::LimeGreen));
			break;
		case 2:
			pRend->SetColor(D2D1::ColorF(D2D1::ColorF::Green));
			break;
		case 3:
			pRend->SetColor(D2D1::ColorF(D2D1::ColorF::LawnGreen));
			break;
		case 4:
			pRend->SetColor(D2D1::ColorF(D2D1::ColorF::ForestGreen));
			break;
		}
		float r, x, y;

		for (int i = iter; i < -iter+5; i++)
		{
			a = GetTransform(a, s);
			r = a.x*a.x + a.y*a.y;
			if (rand()%2)
			{
				x = a.x*sin(r) - a.y*cos(r);
				y = a.x*cos(r) + a.y*sin(r);
			}
			else
			{
				x = 2*a.y / (sqrt(r)+1);
				y = 2*a.x / (sqrt(r)+1);
			}
			a.x = x;
			a.y = y;
			//float x = a.x*sin(r*r) - y*cos(r*r) 
			//a.x /= r;
			//a.y /= r;
		}
		col = 0;
		pRend->pRT->DrawRectangle(pRend->Point(a.x*size + X, -a.y*size + Y, 0.001), pRend->pBrush);
	}

	pRend->DrawNumber(pRend->k, 10, 10, 20);
	pRend->DrawNumber(width*width*4, 10, 50, 20);

	pRend->k++;
	Sleep(00);

	return hr;
}

HRESULT RenderIFS2()
{
	HRESULT hr = S_OK;

	int iter = 25;//k*5;
	int X = GetSystemMetrics(SM_CXSCREEN)/2 + Xoffset,
		Y = GetSystemMetrics(SM_CYSCREEN)/2 + Yoffset;
	int size = 50*scale;
	int width = 500*count;
	D2D1_POINT_2F a, b, c;
	a = D2D1::Point2F( 0, 0 );
	b = a; b.y -= 300;
	lw[0] = pSig->GetSignal(50, 0);
	//pRend->pRT->Clear(D2D1::ColorF(D2D1::ColorF::Black));
	
		pRend->SetColor(D2D1::ColorF(0.f,0.f,0.f,0.1f));
		pRend->pRT->FillRectangle(D2D1::RectF(0,0, X*2,Y*2), pRend->pBrush); 

	for (int i = 0; i < width; i++) 
	{
		int c;
		col = 0;
		a.x = 0;
		a.y = 0;
		for (int i = 0; i < iter; i++)
		{
			a = GetTransform(a, s);
			if (i==0) c = col;
		}

		switch (c)
		{
		case 0:
			pRend->SetColor(D2D1::ColorF(D2D1::ColorF::White));
			break;
		case 1:
			pRend->SetColor(D2D1::ColorF(D2D1::ColorF::Red));
			break;
		case 2:
			pRend->SetColor(D2D1::ColorF(D2D1::ColorF::Green));
			break;
		case 3:
			pRend->SetColor(D2D1::ColorF(D2D1::ColorF::Blue));
			break;
		case 4:
			pRend->SetColor(D2D1::ColorF(D2D1::ColorF::Yellow));
			break;
		}

		float r, x, y;

		for (int i = iter; i < -iter+5; i++)
		{
			a = GetTransform(a, s);
			r = a.x*a.x + a.y*a.y;
			if (rand()%2)
			{
				x = a.x*sin(r) - a.y*cos(r);
				y = a.x*cos(r) + a.y*sin(r);
			}
			else
			{
				x = 2*a.y / (sqrt(r)+1);
				y = 2*a.x / (sqrt(r)+1);
			}
			a.x = x;
			a.y = y;
			//float x = a.x*sin(r*r) - y*cos(r*r) 
			//a.x /= r;
			//a.y /= r;
		}
		pRend->DrawPoint(a.x*size + X, -a.y*size + Y, 0.9f);
	}

	pRend->DrawNumber(pRend->k, 10, 10, 20);
	pRend->DrawNumber(width, 10, 50, 20);

	pRend->k++;

	return hr;
}

HRESULT LineIFS()
{
	HRESULT hr = S_OK;

	int iter = 25;//k*5;
	int X = GetSystemMetrics(SM_CXSCREEN)/2 + Xoffset,
		Y = GetSystemMetrics(SM_CYSCREEN)/2 + Yoffset;
	int size = 50*scale;
	int width = 500*count;
	D2D1_POINT_2F a, b, c;
	a = D2D1::Point2F( 0, 0 );
	b = a;
	lw[0] = pSig->GetSignal(50, 0);
	pRend->pRT->Clear(D2D1::ColorF(D2D1::ColorF::Black));

	for (int i = 0; i < width; i++) 
	{
		int c;
		col = 0;
		a.x = 0;
		a.y = 0;
		for (int i = 0; i < iter; i++)
		{
			a = GetTransform(a, s);
			if (i==0) c = col;
		}

		switch (c)
		{
		case 0:
			pRend->SetColor(D2D1::ColorF(D2D1::ColorF::White));
			break;
		case 1:
			pRend->SetColor(D2D1::ColorF(D2D1::ColorF::Red));
			break;
		case 2:
			pRend->SetColor(D2D1::ColorF(D2D1::ColorF::Green));
			break;
		case 3:
			pRend->SetColor(D2D1::ColorF(D2D1::ColorF::Blue));
			break;
		case 4:
			pRend->SetColor(D2D1::ColorF(D2D1::ColorF::Yellow));
			break;
		}

		float r, x, y;

		for (int i = iter; i < -iter+5; i++)
		{
			a = GetTransform(a, s);
			r = a.x*a.x + a.y*a.y;
			if (rand()%2)
			{
				x = a.x*sin(r) - a.y*cos(r);
				y = a.x*cos(r) + a.y*sin(r);
			}
			else
			{
				x = 2*a.y / (sqrt(r)+1);
				y = 2*a.x / (sqrt(r)+1);
			}
			a.x = x;
			a.y = y;

		}
		//pRend->DrawPoint(a.x*size + X, -a.y*size + Y, 0.9f);
		a.x = a.x*size + X;
		a.y = -a.y*size + Y;
		if (i < 1)
		{
			b = a;
			continue;
		}

		pRend->DrawLine(a, b, 0.1f);
		b = a;
	}

	pRend->DrawNumber(pRend->k, 10, 10, 20);
	pRend->DrawNumber(width, 10, 50, 20);

	pRend->k++;

	return hr;
}

};
#endif