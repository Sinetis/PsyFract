#ifndef _GEOMETRIC_H_
#define _GEOMETRIC_H_

#include "Controller.h"
#include "Interfaces.h"

using namespace Const;

class Geometric: public IFractal
{
	// Номер канала для отрисовки
	int channel;
	// Размер массива частот
	int n;
	// Массивы частот7
	int* freq[2];
	int* fur;
	// Индексаторы частот
	int f[2];
	// Тип фрактала
	int type;
	// Просто какая-то переменная
	int s, k, prev;
	// 
	int q, w, e;

	float time, fullTime;

public:
	Geometric(Controller* c)
	{
		SetBehavior(c);

		channel = 0;
		n = 512;
		freq[0] = 0;
		freq[1] = 0;
		fur = 0;
		f[0] = f[1] = 0;
		type = 0;
		this->s = 5;
		k = 0;
		w = q = 1;
		e = 1;
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
		case 1: 
			Koch();
			break;
		case 2:
			RenderFractalKoch();
			break;
		case 3:
			CircleBest();
			break;
		case 4:
			Light();
			break;
		case 5:
			Circle();
			break;
		default:
			CircleBest();
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
			q = 1 - q;
			break;
		case 'W':
			w = 1 - w;
			break;
		case 'E':
			e = 1 - e;
			w = q = 0;
			break;
		default:
			break;
		}
		return 0;
	}

	

float S3_2, C3_2;
HRESULT RenderFractalKoch()
{
	
	HRESULT hr = S_OK;
	int X = GetSystemMetrics(SM_CXSCREEN)/2,
		Y = GetSystemMetrics(SM_CYSCREEN)/2;

	//D2D_POINT_2F p1, p2, p3, p4;
	//p1.x = 181+68, p1.y = 65+77 + 256;
	//p2.x = 1185-68, p2.y = 65+77 + 256;
	//p3.x = 1185-68, p3.y = 703-77;
	//p4.x = 181+68, p4.y = 703-77;

	if (SUCCEEDED(hr))
	{
		pRend->pRT->Clear(D2D1::ColorF(D2D1::ColorF::Aqua));
		pRend->SetColor(D2D1::ColorF(D2D1::ColorF::SandyBrown));
		pRend->pRT->FillRectangle(D2D1::RectF(0, 500, 1366, 768), pRend->pBrush);
		pRend->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
		//pRT->SetAntialiasMode(D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		//pRT->DrawTextW(str, sizeof(str), NULL, D2D1::Rect(0,0, 50,50), pBrush);
		//LPWORD lw = aud->GetFrequency(2); 
		//p1.y += lw[0] >> 8;
		//p2.y += lw[1] >> 8;
		//FractalKoch2(p2, p1, s);
		//FractalKoch2(p4, p3, s);
		//text
		int k1 = 1, k2 = 3, k3 = 2;

		pRend->DrawLine(D2D1::Point2F(0,500), D2D1::Point2F(X*2,500), 0.25F);
		pRend->DrawLine(D2D1::Point2F(-100,Y*2), D2D1::Point2F(X,500), 0.25F);
		pRend->DrawLine(D2D1::Point2F(X-100,Y*2), D2D1::Point2F(X,500), 0.25F);
		pRend->DrawLine(D2D1::Point2F(X+100,Y*2), D2D1::Point2F(X,500), 0.25F);
		pRend->DrawLine(D2D1::Point2F(X*2+100,Y*2), D2D1::Point2F(X,500), 0.25F);

		pRend->SetColor(D2D1::ColorF(D2D1::ColorF::Blue));
		freq[0] = pSig->GetSignal(n,1);
		//	for (int i = 0; i < n; i++)
		//		if((freq[0][i] < 2 && freq[0][i] > -2) || (freq[0][i] < -512 && freq[0][i] > 512))
		//			freq[0][i] = 0;



		fur = ComplexArr(freq[0], n).Mod();
		freq[1] = pSig->GetSignal(n,3);
		//	for (int i = 0; i < n; i++)
		//		if((freq[1][i] < 2 && freq[1][i] > -2) || (freq[1][i] < -512 && freq[1][i] > 512))
		//			freq[1][i] = 0;

		pRend->SetColor(D2D1::ColorF(D2D1::ColorF(
				sin((fur[5]+fur[6]+fur[7]+fur[8]+fur[9])/200+0.7)+0.2,
				sin((fur[20]+fur[21]+fur[22]+fur[23]+fur[24])/200+0.7)+0.5,
				sin((fur[45]+fur[49]+fur[46]+fur[47]+fur[48])/200+0.7)+0.2)));

		FractalKoch(
			D2D1::Point2F(X + 300* cos(PI_180*k*k1), 
				Y + 300* sin(PI_180*k)),
			D2D1::Point2F(X + 300* cos(PI/3*2+PI_180*k*k2), 
				Y + 300* sin(PI/3*2+PI_180*k)), k<s*0? k/60: s);
	//	FractalKoch(
	//		D2D1::Point2F(X + 300* cos(PI_180*k), 
	//			Y + 300* sin(-PI_180*k)),
	///		D2D1::Point2F(X + 300* cos(PI/3*2-PI_180*k), 
	//			Y + 300* sin(PI/3*2-PI_180*k)), k<s*60? k/60: s);
		FractalKoch(
			D2D1::Point2F(X + 300* cos(PI/3*2+PI_180*k*k2), 
				Y + 300* sin(PI/3*2+PI_180*k)),
			D2D1::Point2F(X + 300* cos(PI/3*4+PI_180*k*k3), 
				Y + 300* sin(PI/3*4+PI_180*k)), k<s*0? k/60: s);
		FractalKoch(
			D2D1::Point2F(X + 300* cos(PI/3*4+PI_180*k*k3), 
				Y + 300* sin(PI/3*4+PI_180*k)),
			D2D1::Point2F(X + 300* cos(PI_180*k*k1), 
				Y + 300* sin(PI_180*k)), k<s*0? k/60: s);

				FractalKoch(
			D2D1::Point2F(X + 200* cos(PI_180*k*k1), 
				Y + 200* sin(PI_180*k)),
			D2D1::Point2F(X + 200* cos(PI/3*2+PI_180*k*k2), 
				Y + 200* sin(PI/3*2+PI_180*k)), k<s*0? k/60: s);
	//	FractalKoch(
	//		D2D1::Point2F(X + 300* cos(PI_180*k), 
	//			Y + 300* sin(-PI_180*k)),
	///		D2D1::Point2F(X + 300* cos(PI/3*2-PI_180*k), 
	//			Y + 300* sin(PI/3*2-PI_180*k)), k<s*60? k/60: s);
		FractalKoch(
			D2D1::Point2F(X + 200* cos(PI/3*2+PI_180*k*k2), 
				Y + 200* sin(PI/3*2+PI_180*k)),
			D2D1::Point2F(X + 200* cos(PI/3*4+PI_180*k*k3), 
				Y + 200* sin(PI/3*4+PI_180*k)), k<s*0? k/60: s);
		FractalKoch(
			D2D1::Point2F(X + 200* cos(PI/3*4+PI_180*k*k3), 
				Y + 200* sin(PI/3*4+PI_180*k)),
			D2D1::Point2F(X + 200* cos(PI_180*k*k1), 
				Y + 200* sin(PI_180*k)), k<s*0? k/60: s);


		FractalKoch(
			D2D1::Point2F(X + 100* cos(PI_180*k*k1), 
				Y + 100* sin(PI_180*k)),
			D2D1::Point2F(X + 100* cos(PI/3*2+PI_180*k*k2), 
				Y + 100* sin(PI/3*2+PI_180*k)), k<s*0? k/60: s);
	//	FractalKoch(
	//		D2D1::Point2F(X + 300* cos(PI_180*k), 
	//			Y + 300* sin(-PI_180*k)),
	///		D2D1::Point2F(X + 300* cos(PI/3*2-PI_180*k), 
	//			Y + 300* sin(PI/3*2-PI_180*k)), k<s*60? k/60: s);
		FractalKoch(
			D2D1::Point2F(X + 100* cos(PI/3*2+PI_180*k*k2), 
				Y + 100* sin(PI/3*2+PI_180*k)),
			D2D1::Point2F(X + 100* cos(PI/3*4+PI_180*k*k3), 
				Y + 100* sin(PI/3*4+PI_180*k)), k<s*0? k/60: s);

		FractalKoch(
			D2D1::Point2F(X + 100* cos(PI/3*4+PI_180*k*k3), 
				Y + 100* sin(PI/3*4+PI_180*k)),
			D2D1::Point2F(X + 100* cos(PI_180*k*k1), 
				Y + 100* sin(PI_180*k)), k<s*0? k/60: s);
		//FractalKoch(
		//	D2D1::Point2F(1, 1),
		//	D2D1::Point2F(1, 767), k<s*0? k/60: s);
		//FractalKoch(
		//	D2D1::Point2F(1365, 1),
		//	D2D1::Point2F(1365, 767), k<s*0? k/60: s);

		//FractalKoch(
		//	D2D1::Point2F(1366/4, Y),
		//	D2D1::Point2F(1366 - 1366/4, Y), s);

		pRend->DrawNumber(k, 10, 10, 20);
		//DrawNine(10, 10, 10);
		//Sleep(67);
	}
	if ((k++)>10800)
		k=0;

	FreeMem(&freq[0]);
	FreeMem(&freq[1]);
	FreeMem(&fur);
	return hr;
}

	int Koch()
	{
		channel = 0;
		int iter = 5;
		int k = pRend->k;
	
		HRESULT hr = S_OK;
		int X = GetSystemMetrics(SM_CXSCREEN)/2,
			Y = GetSystemMetrics(SM_CYSCREEN)/2;

		if (SUCCEEDED(hr))
		{
			pRend->pRT->Clear(D2D1::ColorF(D2D1::ColorF::Black));
			int k1 = 1, k2 = 3, k3 = 2;

			pRend->SetColor(D2D1::ColorF(D2D1::ColorF::Blue));

			f[0]=f[1]=0;
			n = 512;
		
			freq[0] = pSig->GetSignal(n,1);
			for (int i = 0; i < n; i++)
				if((freq[0][i] < 2 && freq[0][i] > -2) || (freq[0][i] < -512 && freq[0][i] > 512))
					freq[0][i] = 0;


			fur = ComplexArr(freq[0], n).Mod();
			freq[1] = pSig->GetSignal(n,3);
			for (int i = 0; i < n; i++)
				if((freq[1][i] < 2 && freq[1][i] > -2) || (freq[1][i] < -512 && freq[1][i] > 512))
					freq[1][i] = 0;

			pRend->SetColor(D2D1::ColorF(D2D1::ColorF(
				sin((fur[5]+fur[6]+fur[7]+fur[8]+fur[9])/20+0.7)+0.2,
				sin((fur[20]+fur[21]+fur[22]+fur[23]+fur[24])/20+0.7)+0.5,
				sin((fur[45]+fur[49]+fur[46]+fur[47]+fur[48])/20+0.7)+0.2)));

			k=30;
			FractalKoch(
				D2D1::Point2F(X + 300* cos(PI_180*k), 
					Y + 300* sin(-PI_180*k)),

				D2D1::Point2F(X + 300* cos(PI/3*2-PI_180*k), 
					Y + 300* sin(PI/3*2-PI_180*k)), iter);

			FractalKoch(
				D2D1::Point2F(X + 300* cos(PI/3*2-PI_180*k), 
					Y + 300* sin(PI/3*2-PI_180*k)),

				D2D1::Point2F(X + 300* cos(-PI/3*2-PI_180*k), 
					Y + 300* sin(-PI/3*2-PI_180*k)), iter);

			FractalKoch(
				D2D1::Point2F(X + 300* cos(-PI/3*2-PI_180*k), 
					Y + 300* sin(-PI/3*2-PI_180*k)),

				D2D1::Point2F(X + 300* cos(PI_180*k), 
					Y + 300* sin(-PI_180*k)), iter);

			channel = 1; k += 60;
			//pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Aqua));

			FractalKoch(
				D2D1::Point2F(X + 300* cos(PI_180*k), 
					Y + 300* sin(-PI_180*k)),

				D2D1::Point2F(X + 300* cos(PI/3*2-PI_180*k), 
					Y + 300* sin(PI/3*2-PI_180*k)), iter);

			FractalKoch(
				D2D1::Point2F(X + 300* cos(PI/3*2-PI_180*k), 
					Y + 300* sin(PI/3*2-PI_180*k)),

				D2D1::Point2F(X + 300* cos(-PI/3*2-PI_180*k), 
					Y + 300* sin(-PI/3*2-PI_180*k)), iter);

			FractalKoch(
				D2D1::Point2F(X + 300* cos(-PI/3*2-PI_180*k), 
					Y + 300* sin(-PI/3*2-PI_180*k)),

				D2D1::Point2F(X + 300* cos(PI_180*k), 
					Y + 300* sin(-PI_180*k)), iter);
			k -= 60;
			//Sleep(67);
		}
		if ((k)>10800)
			k=0;
		FreeMem(&freq[0]);
		FreeMem(&freq[1]);
		FreeMem(&fur);
		return hr;
	}

	void FractalKoch(D2D1_POINT_2F a, D2D1_POINT_2F e, int i)
	{
		D2D1_POINT_2F b, c, d, c2;
		if (((a.x - e.x)<4 && (a.x - e.x)>-4 && (a.y - e.y)<4 && (a.y - e.y)>-4) || i<=0)
		//if (i == 0)
		{
			pRend->DrawLine(a, e, 2.9F);
			//pRT->DrawRectangle(Point(a.x, a.y, 1.0f),pBrush);
			//pRT->DrawRectangle(Point(e.x, e.y, 1.0f),pBrush);
			return;
		}	

		b.x = (e.x - a.x) / 3 + a.x;// + (aud->GetFrequency() - 128) / 8;
		b.y = (e.y - a.y) / 3 + a.y;// + (aud->GetFrequency() - 128) / 8;
		d.x = (e.x - a.x) / 3 * 2 + a.x;
		d.y = (e.y - a.y) / 3 * 2 + a.y;

		if (q == 1 || f[0] > n-4) f[0] = 0;
		if (q == 1 || f[1] > n-4) f[1] = 0;
		if (channel)
		{
			pRend->RotationVector(&c2, d, b, i==s? freq[0][f[0]++]: -freq[0][f[0]++]);
			pRend->RotationVector(&c, b, d, i==s? -freq[0][f[0]++]: freq[0][f[0]++]);
		}
		else
		{
			pRend->RotationVector(&c2, d, b, i==s? -freq[1][f[1]++]: freq[1][f[1]++]);
			pRend->RotationVector(&c, b, d, i==s? freq[1][f[1]++]: -freq[1][f[1]++]);
		}


		FractalKoch(a, b, i-1);
		//if (i != s)
		FractalKoch(b, c, i-1);
		FractalKoch(c, c2	, i-1);
		//if (i != s)
		FractalKoch(c2, d, i-1);
		FractalKoch(d, e, i-1);


	}

	void Holms(D2D1_POINT_2F a, D2D1_POINT_2F c, int i)
	{
		if (i <= 0)
		{
			pRend->DrawLine(a, c, 0.5F);
			return;
		}
		float d = sqrt((a.x - c.x)*(a.x - c.x) + (a.y - c.y)*(a.y - c.y));
		D2D1_POINT_2F b, f;
		b.x = (c.x - a.x) / 3 + a.x -0.4* d*(rand()%500/500.0F - 0.5F);
		b.y = (c.y - a.y) / 3 + a.y -0.4* d*(rand()%500/500.0F - 0.5F);
		f.x = (c.x - a.x) / 3 + a.x -0.3* d*(rand()%500/500.0F - 0.5F);
		f.y = (c.y - a.y) / 3 + a.y -0.3* d*(rand()%500/500.0F - 0.5F);


		Holms(a, b, i-1);
		if(rand()%100 < 50)
		 Holms(a, f, i-rand()%3-1);
		Holms(b, c, i-1);
	}

	void Circle()
	{
		int X = GetSystemMetrics(SM_CXSCREEN)/2,
			Y = GetSystemMetrics(SM_CYSCREEN)/2;
		float r = 200.0f,R = 200.0f;

		//pRend->pRT->Clear(D2D1::ColorF(0.f, 0.f, 0.f, 1.5f));	
		pRend->SetColor(D2D1::ColorF(0.f,0.f,0.f,0.5f));
		pRend->pRT->FillRectangle(D2D1::RectF(0,0, X*2,Y*2), pRend->pBrush); 

		freq[0] = pSig->GetSignal(1024, 1);
		if (freq[0] == 0)
			return Circle();
		freq[1] = pSig->GetSignal(1024, 3);
		if (freq[1] == 0)
			return Circle();

		if (w == 0)
		{
			pRend->SetColor(D2D1::ColorF(
				fabs(cos(pRend->k/23.0f))/0.9f + 0.1f,
				fabs(cos(pRend->k/61.0f))/0.9f + 0.1f,
				fabs(cos(pRend->k/181.0f))/0.9f + 0.1f));
		}
		else
		{
			f[0] = 0;
			f[1] = 0;
			fur = ComplexArr(freq[0], 1024).Mod();

			float r=0,g=0,b=0;
			float s = 512.0f/22020.0f;
			int i;
			for(i = 40*s; i < 1440*s; i++) //100 1500
				//if(fur[i]>5)
					r += fur[i];
			for(i = 880*s; i < 2560*s; i++) // 1050 2450
				//if(fur[i]>5)
					g += fur[i];
			for(i = 2000*s; i < 3400*s; i++) // 2000 3400
				//if(fur[i]>5)
					b += fur[i];
			
			for(i = 3120*s; i < 3400*s; i++) // 2000 3400
					r += fur[i];

			for(i = 40*s; i < 320*s; i++) // 2000 3400
					b += fur[i];

			//r /= 1500;
			//g /= 1500;
			//b /= 1500;	

			float max = r;
			if (g > max)
				max = g;
			if (b > max)
				max = b;


			pRend->SetColor(D2D1::ColorF(D2D1::ColorF(r/max, g/max, b/max)));

			pRend->DrawNumber(r*255/max, 10,10,10);
			pRend->DrawNumber(g*255/max, 10,35,10);
			pRend->DrawNumber(b*255/max, 10,60,10);

			FreeMem(&fur);
		}
		//+++++++++++++++++++++++++++++++++++++++X += pRend->k%2 == 0? 100: -100;
		D2D1_POINT_2F p1, p2;
		p1.x = X + (R + freq[0][0])* cos(0);
		p1.y = Y + (R + freq[0][0])* sin(0);

		p2.x = X; p2.y = Y;

		//D2D1::Point2F(X + 300* cos(PI/3*2-PI_180*k), 
		//			Y + 300* sin(PI/3*2-PI_180*k)),
		if (q)
		{
			Y+=R/2;
			X+=R/2;
		}
		//pRend->pRT->FillEllipse(D2D1::Ellipse(D2D1::Point2F(X,Y), 300, 300),pRend->pBrush);

		if (e)
		{
			p1.x = X + (R + freq[0][0])* cos(0);
			p1.y = Y - (R + freq[0][0])* sin(0);
			for (int i = 1; i <= 720 ; i++)
			{
				p2.x = X + (R - freq[0][i]/2)* cos(PI_180*(i)/2)/2;
				p2.y = Y - (R - freq[0][i]/2)* sin(PI_180*(i)/2)/2;
				pRend->DrawLine(p1, p2, 0.9f);
				p1.x = X + (R + freq[0][i])* cos(PI_180*i/2);
				p1.y = Y - (R + freq[0][i])* sin(PI_180*i/2);
			}
		}
		else
		{
			R = 250;
			p1.x = X + (R + freq[0][0])* cos(0);
			p1.y = Y - (R + freq[0][0])* sin(0);
			for (int i = 1; i <= 720 ; i++)
			{
				p2.x = X + (R - freq[1][i])* cos(PI_180*(i)/2)/2;
				p2.y = Y - (R - freq[1][i])* sin(PI_180*(i)/2)/2;
				pRend->DrawLine(p1, p2, 0.9f);
				p1.x = X + (R + freq[0][i])* cos(PI_180*i/2);
				p1.y = Y - (R + freq[0][i])* sin(PI_180*i/2);
			}
			R = 200;
		}

		if (q && e)
		{
			Y-=R;
			p1.x = X + (R + freq[1][0])* cos(0);
			p1.y = Y + (R + freq[1][0])* sin(0);
			for (int i = 1; i <= 720 ; i++)
			{
				p2.x = X + (R - freq[1][i]/2)* cos(PI_180*(i)/2)/2;
				p2.y = Y + (R - freq[1][i]/2)* sin(PI_180*(i)/2)/2;
				pRend->DrawLine(p1, p2, 0.9f);
				p1.x = X + (R + freq[1][i])* cos(PI_180*i/2);
				p1.y = Y + (R + freq[1][i])* sin(PI_180*i/2);
			}

			Y+=R;
			X-=R;
			p1.x = X + (R + freq[0][0])* cos(PI);
			p1.y = Y - (R + freq[0][0])* sin(PI);
			for (int i = 1; i <= 720 ; i++)
			{
				p2.x = X + (R - freq[0][i]/2)* cos(PI_180*(360-i)/2)/2;
				p2.y = Y - (R - freq[0][i]/2)* sin(PI_180*(360-i)/2)/2;
				pRend->DrawLine(p1, p2, 0.9f);
				p1.x = X + (R + freq[0][i])* cos(PI_180*(360-i)/2);
				p1.y = Y - (R + freq[0][i])* sin(PI_180*(360-i)/2);
			}
			Y-=R;
			p1.x = X + (R + freq[1][0])* cos(PI);
			p1.y = Y + (R + freq[1][0])* sin(PI);
			for (int i = 1; i <= 720 ; i++)
			{
				p2.x = X + (R - freq[1][i]/2)* cos(PI_180*(360-i)/2)/2;
				p2.y = Y + (R - freq[1][i]/2)* sin(PI_180*(360-i)/2)/2;
				pRend->DrawLine(p1, p2, 0.9f);
				p1.x = X + (R + freq[1][i])* cos(PI_180*(360-i)/2);
				p1.y = Y + (R + freq[1][i])* sin(PI_180*(360-i)/2);
			}
		}

		pRend->k++;

		FreeMem(&freq[0]);
		FreeMem(&freq[0]);
		FreeMem(&freq[1]);
		FreeMem(&fur);
	}

	void Light()
	{
		int X = GetSystemMetrics(SM_CXSCREEN)/2,
			Y = GetSystemMetrics(SM_CYSCREEN)/2;

		pRend->pRT->Clear(D2D1::ColorF(D2D1::ColorF::Black));

		pRend->SetColor(D2D1::ColorF(
			fabs(cos(pRend->k/23.0f))/0.9f + 0.1f,
			fabs(cos(pRend->k/61.0f))/0.9f + 0.1f,
			fabs(cos(pRend->k/181.0f))/0.9f + 0.1f));
			

		freq[0] = pSig->GetSignal(721, 1);
		f[0] = 0;

		D2D1_POINT_2F p1, p2;
		p1.x = X + (200 + freq[0][0])* cos(0);
		p1.y = Y + (200 + freq[0][0])* sin(0);

		p2.x = X; p2.y = Y;

		//D2D1::Point2F(X + 300* cos(PI/3*2-PI_180*k), 
		//			Y + 300* sin(PI/3*2-PI_180*k)),

		for (int i = 1; i <= 720 ; i++)
		{
			p2.x = X + (200 - freq[0][i]/2)* cos(PI_180*(i/2))/2;
			p2.y = Y + (200 - freq[0][i]/2)* sin(PI_180*(i/2))/2;
			Holms(p2, p1, 4);
			p1.x = X + (200 + freq[0][i])* cos(PI_180*i/2);
			p1.y = Y + (200 + freq[0][i])* sin(PI_180*i/2);
		}

		pRend->k++;

		FreeMem(&freq[0]);
	}

	int CircleBest()
		{
		float t = clock();
		int X = GetSystemMetrics(SM_CXSCREEN)/2,
			Y = GetSystemMetrics(SM_CYSCREEN)/2;
		float r = 200.0f,R = 200.0f;

		//pRend->pRT->Clear(D2D1::ColorF(0.f, 0.f, 0.f, 1.5f));	
		pRend->SetColor(D2D1::ColorF(0.f,0.f,0.f,0.5f));
		pRend->pRT->FillRectangle(D2D1::RectF(0,0, X*2,Y*2), pRend->pBrush); 

		FreeMem(&freq[0]);
		freq[0] = pSig->GetSignal(1024, 1);
		if (freq[0] == 0)
			return CircleBest();
		smooth(freq[0], 721, 10);

		FreeMem(&freq[1]);
		freq[1] = pSig->GetSignal(1024, 3);
		if (freq[1] == 0)
			return CircleBest();
		smooth(freq[1], 721, 10);

		{
		f[0] = 0;
		f[1] = 0;
		FreeMem(&fur);
		int* fur2 = new int[1024];
		for (int i = 0; i < 1024; i++)
			fur2[i] = (freq[0][i] + freq[1][i]) / 2;
		//fur = ComplexArr(fur2, 1024).Mod(); 
		fur = FFT::Return(fur2, 1024);
		FreeMem(&fur2);

		float r=0,g=0,b=0;
		float s = 512.0f/22050.0f;
		int i;
		for(i = 40*s; i < 1440*s; i++)
			r += fur[i];
		for(i = 880*s; i < 2560*s; i++)
			g += fur[i];
		for(i = 2000*s; i < 3400*s; i++)
			b += fur[i];
			
		for(i = 3120*s; i < 3400*s; i++) // 2000 3400
			r += fur[i];

		for(i = 40*s; i < 320*s; i++) // 2000 3400
			b += fur[i];	

		float max = r;
		if (g > max)
			max = g;
		if (b > max)
			max = b;

		pRend->SetColor(D2D1::ColorF(D2D1::ColorF(b/max, g/max, r/max)));
			//pRend->DrawNumber(b*255/max, 10,10,10);
			//pRend->DrawNumber(g*255/max, 10,35,10);
			//pRend->DrawNumber(r*255/max, 10,60,10);
		pRend->DrawNumber(pRend->k++, 10,85,2);
		FreeMem(&fur);
		}
		
		//+++++++++++++++++++++++++++++++++++++++X += pRend->k%2 == 0? 100: -100;
		D2D1_POINT_2F p1, p2;
		p1.x = X + (R + freq[0][0])* cos(0);
		p1.y = Y + (R + freq[0][0])* sin(0);

		p2.x = X; p2.y = Y;
		if (q)
		{
			Y+=R/2;
			X+=R/2;
		}
		
		//pRend->pRT->FillEllipse(D2D1::Ellipse(D2D1::Point2F(X,Y), 300, 300),pRend->pBrush);


		p1.x = X + (R + freq[0][0])* cos(0);
		p1.y = Y - (R + freq[0][0])* sin(0);
		for (int i = 1; i <= 720 ; i++)
		{
			p2.x = X + (R - freq[0][i]/2)* cos(PI_180*(i)/2)/2;
			p2.y = Y - (R - freq[0][i]/2)* sin(PI_180*(i)/2)/2;
			pRend->DrawLine(p1, p2, 0.9f);
			p1.x = X + (R + freq[0][i])* cos(PI_180*i/2);
			p1.y = Y - (R + freq[0][i])* sin(PI_180*i/2);
		}

		Y-=R;
		p1.x = X + (R + freq[1][0])* cos(0);
		p1.y = Y + (R + freq[1][0])* sin(0);
		for (int i = 1; i <= 720 ; i++)
		{
			p2.x = X + (R - freq[1][i]/2)* cos(PI_180*(i)/2)/2;
			p2.y = Y + (R - freq[1][i]/2)* sin(PI_180*(i)/2)/2;
			pRend->DrawLine(p1, p2, 0.9f);
			p1.x = X + (R + freq[1][i])* cos(PI_180*i/2);
			p1.y = Y + (R + freq[1][i])* sin(PI_180*i/2);
		}

		Y+=R;
		X-=R;
		p1.x = X + (R + freq[0][0])* cos(PI);
		p1.y = Y - (R + freq[0][0])* sin(PI);
		for (int i = 1; i <= 720 ; i++)
		{
			p2.x = X + (R - freq[0][i]/2)* cos(PI_180*(360-i)/2)/2;
			p2.y = Y - (R - freq[0][i]/2)* sin(PI_180*(360-i)/2)/2;
			pRend->DrawLine(p1, p2, 0.9f);
			p1.x = X + (R + freq[0][i])* cos(PI_180*(360-i)/2);
			p1.y = Y - (R + freq[0][i])* sin(PI_180*(360-i)/2);
		}

		Y-=R;
		p1.x = X + (R + freq[1][0])* cos(PI);
		p1.y = Y + (R + freq[1][0])* sin(PI);
		for (int i = 1; i <= 720 ; i++)
		{
			p2.x = X + (R - freq[1][i]/2)* cos(PI_180*(360-i)/2)/2;
			p2.y = Y + (R - freq[1][i]/2)* sin(PI_180*(360-i)/2)/2;
			pRend->DrawLine(p1, p2, 0.9f);
			p1.x = X + (R + freq[1][i])* cos(PI_180*(360-i)/2);
			p1.y = Y + (R + freq[1][i])* sin(PI_180*(360-i)/2);
		}	

		pRend->DrawNumber((int)time/100, 10,110,2);
		pRend->DrawNumber((int)fullTime/100, 10,135,2);

		pRend->k++;
		t = clock() - t;
		if (prev != Sum(freq[0], 8))
			time += t;
		fullTime += t;
			
		prev = Sum(freq[0], 8);
			//InvalidateRect(pRend->hWnd, NULL, FALSE);
		//FreeMem(&freq[0]);
		//FreeMem(&freq[1]);
		//FreeMem(&fur);
		return 0;
	}

	int Sum(int* f,int n){int s=0;for(int i=0;i<n;i++,s+=f[i]);return s;}

	static void smooth(int* mas, int masn, int n)
{
	float skew;
	masn--;
	mas[0] = (mas[1] + mas[masn]) / 2.0f;
	//masn--;
	for (int i = 1; i < n; i++)
	{
		skew = mas[i] - (mas[i] + mas[masn - i]) / 2.f;
		skew *= 1 - (i+1.f)/(float)n;
		mas[i] -= (int)skew;
		mas[masn - i] += (int)skew;
	}
}

};
#endif