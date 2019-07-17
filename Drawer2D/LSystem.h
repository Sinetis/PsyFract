#ifndef _LSYSTEM_H_
#define _LSYSTEM_H_

#include "Renderer.h"
#include "Interfaces.h"

using namespace Const;

class LSystem: public IFractal
{
	Turtle* turtle;
	std::pair<WCHAR, LPCWSTR> *pair;
	int nump;
	LPCWSTR Axiom;
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
	int q, w;

public:
	LSystem(Controller* c)
	{
		SetBehavior(c);
		turtle = new Turtle(c->Rend());
		channel = 0;
		n = 1024;
		type = 0;
		q = 0;
		w = 0;
		Axiom = L"F-F-F-F--F+F--F-F";
		std::pair<WCHAR, LPCWSTR> tpair[] = {std::pair<WCHAR, LPCWSTR>('F', L"--FF-FF-FF-FF--FF+FF--FF-FF"),
											std::pair<WCHAR, LPCWSTR>('b', L"[F]")};
		pair = tpair;
		nump = 2;
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
			RenderTurtle();
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
		case 'Q':
			SetAxiom(q);
			if(++q > 2)
				q = 0;
			break;
		case 'W':
			SetPair(w);
			if(++w > 2)
				w = 0;
			break;

		default:
			break;
		}
		return 0;
	}

void SetAxiom(int t)
{
	//delete []Axiom;
	switch(t)
	{
	case 0:
		Axiom = L"+F[X]--F[X]--F[X]--F[X]";
		break;
	case 1:
		Axiom = L"F";
		break;
	case 2:
		Axiom = L"F-F-F-F--F+F--F-F";
		break;
	case 3:
		Axiom = L"F";
		break;
	}
}
void SetPair(int t)
{
	//delete []pair;
	switch(t)
	{
	case 0:
		{
		std::pair<WCHAR, LPCWSTR> tpair[] = {std::pair<WCHAR, LPCWSTR>('F', L"--FF-FF-FF-FF--FF+FF--FF-FF"),
											std::pair<WCHAR, LPCWSTR>('b', L"[F]")};
		pair = tpair;
		break;
		}
	case 1:
		{
		std::pair<WCHAR, LPCWSTR> tpair[] = {std::pair<WCHAR, LPCWSTR>('F', L"+FF-"),
								std::pair<WCHAR, LPCWSTR>('X', L"+F[X]--F[X]--F[X]--F[X]--F[X]--F[X]")};
		pair = tpair;
		break;
		}
	case 2:
		{
		std::pair<WCHAR, LPCWSTR> tpair[] = {std::pair<WCHAR, LPCWSTR>('F', L"--FF-FF-FF-FF--FF+FF--FF-FF"),
											std::pair<WCHAR, LPCWSTR>('b', L"[F]")};
		pair = tpair;
		break;
		}
	case 3:
		{
		std::pair<WCHAR, LPCWSTR> tpair[] = {std::pair<WCHAR, LPCWSTR>('F', L"[+F][-F]")};
		pair = tpair;
		break;
		}
	}
}

int RenderTurtle()
{
	HRESULT hr = S_OK;

	int iter = 5;
	int X = GetSystemMetrics(SM_CXSCREEN)/2,
		Y = GetSystemMetrics(SM_CYSCREEN)/2;
	D2D1_POINT_2F a, b, c;
	a = D2D1::Point2F( X, Y );
	b = a; b.y -= 300;
	turtle->Go(a.x, a.y);
	turtle->Direction(90);
	turtle->Color(D2D1::ColorF(.0,.0,.0));

	pRend->pRT->Clear(D2D1::ColorF(D2D1::ColorF::White));

	//pRT->DrawLine(a, b, pBrush);
	srand(time(0));
	//turtle.Func(150, iter);

	turtle->SetAxiom(Axiom);
	for (int i = 0; i < iter; i++)
	{
		turtle->New(pair, nump);
		/*c = AddPointFromVector(a, b, 0.99);
		RotationVector(&c, b, c, 195 + (k/10)%10);
		a = b, b = c;
		pRT->DrawLine(a, b, pBrush);*/
		//turtleForward(300*(float(i)/iter));
		//turtleRotate(aud->GetFrequency());
		//turtleForward(-30*(float(i)/iter));
		//turtleRotate(-aud->GetFrequency()/2);

	}
	//turtle.Interpretation(aud->GetFrequency256()/1.0, 5);
	turtle->Interpretation(pRend->k, 10);
	

	pRend->DrawNumber(lstrlenW(turtle->AXIOM), 10, 10, 20);
	pRend->DrawNumber(pRend->k, 10, 60, 20);

	if(pRend->k++ > 360)
		pRend->k = 0;
	Sleep(00);

	return hr;
}
};

#endif