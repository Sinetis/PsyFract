#ifndef _FREQUENCY_H_
#define _FREQUENCY_H_

#include "Controller.h"
#include "Interfaces.h"

class Frequency: public IFractal
{
	// Номер канала для отрисовки
	int channel;
	// Размер массива частот
	int n;
	// Массивы частот
	int* lw[4];
	int* M;
	long int *F;
	// Индексаторы частот
	//int f[2];
	// Тип Отрисовки
	int type;
	// Просто какая-то переменная
	int s, k;
	bool d;

	bool round;

public:
	Frequency(Controller* c)
	{
		SetBehavior(c);
		channel = 0;
		d = 0;
		n = 1024;
		type = 0;
		this->s = 16;
		k = 0;
		round  = false;
	}

	int Initialize()
	{
		lw[0]=lw[1]=lw[2]=lw[3]=0;
		M=NULL;
		F=NULL;
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
		case 0:
			FreqAnalyzer();
			break;
		case 2:
			Freq();
			break;
		case 3:
			Osc();
			break;
		default:
			Drive();
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
		case VK_UP:
			if (s < 512)
				s++;
			break;
		case VK_DOWN:
			if (s > 1)
				s--;
			break;
		case 'R':
			round = 1 - round;
			break;
		case 'Q':
			d = 1 - d;
			break;

		default:
			break;
		}
		return 0;
	}

HRESULT Freq()
{
	HRESULT hr = S_OK;

	int X = GetSystemMetrics(SM_CXSCREEN)/2,
		Y = GetSystemMetrics(SM_CYSCREEN)/5;

	//LPWORD lw = aud->GetFrequency256(n);
	//mic->start();
	//Sleep(000);

	lw[0] = pSig->GetSignal(n, 1);

	//for (int i = 0; i < n; i++)
	//			if((lw[0][i] < 4 && lw[0][i] > -4) || (lw[0][i] < -512 && lw[0][i] > 512))
	//				lw[0][i] = 0;
	//lw[1] = ComplexArr(lw[0], n).Mod();
	lw[1] = FFT::Return(lw[0], n);
	lw[2] = pSig->GetSignal(n, 3);
	lw[3] = pSig->GetSignal(n*64, 3);
	//mic->sound();
	
	
	pRend->pRT->Clear(D2D1::ColorF(D2D1::ColorF::White));
	pRend->SetColor(D2D1::ColorF(D2D1::ColorF::Black));

	D2D1_POINT_2F p1, p2;
	for (int j = 0; j < 3; j++)
	{
		p1.y = p2.y = Y*(j+1);
		p1.x = p2.x = 170;
		p1.y -= lw[j][0]/2 -0;
		for (int i = 0; i < n && p2.x < X*2; i++)
		{
			p2.x += 1.0;
			p2.y = Y*(j+1) - (lw[j][i])/2 + 0;
			pRend->DrawLine(p1, p2, 1.0f);
			p1 = p2;
		}
	}	
	p1.y = p2.y = Y*4;
	p1.x = p2.x = 170;
	p1.y -= lw[3][0]/2 -0;
	for (int i = 0; i < n*64 && p2.x < X*2; i+=32)
	{
		p2.x += 0.5f;
		p2.y = Y*4 - (lw[3][i])/2 + 0;
		pRend->DrawLine(p1, p2, 1.0f);
		p1 = p2;
	}
	

	pRend->DrawNumber(k++, 10, 10, 20);
	pRend->DrawNumber(1, 100, Y*1-20, 20);
	pRend->DrawNumber(2, 100, Y*2-20, 20);
	pRend->DrawNumber(3, 100, Y*3-20, 20);
	pRend->DrawNumber(4, 100, Y*4-20, 20);

	FreeMem(&lw[0]);
	FreeMem(&lw[1]);
	FreeMem(&lw[2]);
	FreeMem(&lw[3]);
	return hr;
}

void Filter(int* l, int n)
{
	int j;
	int dno = -1, t;
	n--;
	for(int i = 0; i < n; i++)
	{
		if (l[i] > l[i+1])
		{
			j = i-1;
			while (j > dno)
			{
				l[j] = 0;
				j--;
			}
			j = i+1;
			do 
			{
				t = l[j];
				l[j] = 0;
				j++;
			}
			while (t > l[j]);
			dno = j;
		}
	}
}

HRESULT FreqAnalyzer()
{
	HRESULT hr = S_OK;

	int X = GetSystemMetrics(SM_CXSCREEN)/2,
		Y = GetSystemMetrics(SM_CYSCREEN)/3;

	lw[0] = pSig->GetSignal(n, 1);
	if (round)
	{
		int* temp = new int[n];
		memcpy((void*)temp, (void*)lw[0], sizeof(int) * n);
		memcpy((void*)(lw[0]), (void*)(temp+n/2), sizeof(int) * n/2);
		memcpy((void*)(lw[0]+n/2), (void*)(temp), sizeof(int) * n/2);
	}
	//Filter(lw[0], 512);
	//lw[1] = FFT::Return(lw[0], n);
	lw[1] = pSig->GetLogFFT();
	if (round)
	Filter(lw[1], 512);
	//int l = 512;
	//if (round)
	//for (int i = 0; i < n/2; i++)
	//	if (lw[1][i] > l)
	//	{
	//		lw[1][i] = l - lw[1][i];
	//		//lw[1][i+16] += lw[1][i] - l;
	//		//lw[1][i] -= lw[1][i] - l;
	//	}

	pRend->pRT->Clear(D2D1::ColorF(D2D1::ColorF::White));
	pRend->SetColor(D2D1::ColorF(D2D1::ColorF::Black));

	D2D1_POINT_2F p1, p2;
	{
		p1.y = p2.y = Y;
		p1.x = p2.x = 170;
		p1.y -= lw[0][0]/2 -0;
		for (int i = 0; i < n && p2.x < X*2; i++)
		{
			p1.y = Y;
			p2.y = Y - (lw[0][i])/2 + 0;
			pRend->DrawLine(p1, p2, 1.0f);
			p2.x += 1.0f;
			p1.x += 1.0f;
		}
	}	
	{
		p1.y = p2.y = Y*3;
		p1.x = p2.x = 170;
		p1.y -= lw[1][0]/2 -0;
		for (int i = n/2; i < n && p2.x < X*2; i++)
		{
			p1.y = Y*2;
			p2.x += 1.0;
			p2.y = Y*2 - (lw[1][n-i])/4 + 0;
			pRend->DrawLine(p1, p2, 1.0f);
			p1 = p2;
		}
		for (int i = 0; i < n/2 && p2.x < X*2; i++)
		{
			p2.x += 1.0;
			p2.y = Y*2 - (lw[1][i])/4 + 0;
			pRend->DrawLine(p1, p2, 1.0f);
			p1 = p2;
		}
	}

	pRend->DrawNumber(k++, 10, 10, 20);
	pRend->DrawNumber(1, 100, Y*1-20, 20);
	pRend->DrawNumber(2, 100, Y*2-20, 20);
	
	pRend->SetColor(D2D1::ColorF(0.2f,0.4f,1.f));

	float o = 512000/22050; // Ширина 1000 герц

	for(int i = 0; i < 23; i++)
		pRend->DrawLine(682.f + i*o, 2.f * Y - 5, 682.f + i*o, 2.f * Y + 5);

	pRend->DrawNumber(0, 684, 2.f * Y + 10, 5);
	pRend->DrawLine(682.f, 2.f * Y - 50, 682.f, 2.f * Y + 20);

	pRend->DrawNumber(100, 128+684, 2.f * Y + 10, 5);
	pRend->DrawLine(128+682.f, 2.f * Y - 15, 128+682.f, 2.f * Y + 20);

	pRend->DrawNumber(1000, 256+684, 2.f * Y + 10, 5);
	pRend->DrawLine(256+682.f, 2.f * Y - 25, 256+682.f, 2.f * Y + 20);

	pRend->DrawNumber(10000, 384+684, 2.f * Y + 10, 5);
	pRend->DrawLine(384+682.f, 2.f * Y - 15, 384+682.f, 2.f * Y + 20);

	pRend->DrawNumber(22050, 512+684, 2.f * Y + 10, 5);
	pRend->DrawLine(512+682.f, 2.f * Y - 50, 512+682.f, 2.f * Y + 20);

	pRend->SetColor(D2D1::ColorF(1.f,0.4f,0.2f, 0.75f));

	int mid;

	int *freq = new int[8];
	//int S = 512/s;
	int I[2][8] = { {0,	6,	18,	36,	64,		110,	184,	306},
					{8,	20,	38,	66,	114,	190,	314,	512}};
	for (int j = 0; j < 8; j++)
		{I[0][j] /= 4; I[1][j] /= 4;}
	for (int j = 0; j < 8; j++)
	{
		freq[j] = 0;
		for (int i = I[0][j]; i < I[1][j]; i++)
			freq[j] += lw[1][i];
		if (freq[j]>0)
			pRend->DrawPoint(684 + I[0][j]*4, 2.f * Y + 25, sqrt(freq[j])/4);
		else
			pRend->DrawPoint(684 - I[0][j]*4 - sqrt(-freq[j])/4, 2.f * Y + 25, sqrt(-freq[j])/4);

	}
	//for(int j = 0; j < s; j++)
	//{
	//	freq[j] = 0;
	//	for(int i = 0; i < S; i++)
	//		freq[j] += lw[1][j*S + i];
	//	if (freq[j]>0)
	//		pRend->DrawPoint(684 + j*S, 2.f * Y + 25, sqrt(freq[j])/4);
	//	else
	//		pRend->DrawPoint(684 - j*S - sqrt(-freq[j])/4, 2.f * Y + 25, sqrt(-freq[j])/4);
	//}


	FreeMem(&freq);
	FreeMem(&lw[0]);
	FreeMem(&lw[1]);
	return hr;
}

HRESULT Drive()
{
	HRESULT hr = S_OK;
	int oldN = n;
	if (d) n=2048;
	else n = 1024;
	float dob = 4;
	if (d) dob = 1;

	if (oldN != n)
	{
		FreeMem(&lw[2]);
		FreeMem(&M);
		FreeMem(&F);
		M = new int[n/2];
		F = new long int[n/2];
		lw[2] = new int[n/2];
		for (int i = 0; i < n/2; i++)
			M[i] = F[i] = lw[2][i] = 0;
	}

	int X = GetSystemMetrics(SM_CXSCREEN)/2,
		Y = GetSystemMetrics(SM_CYSCREEN)/3 - 32;

	lw[0] = pSig->GetSignal(n, 1);
	lw[3] = pSig->GetSignal(n, 3);
	for (int i = 0; i < n; i++)
		lw[0][i] = (lw[0][i] + lw[3][i])/2;
	lw[1] = FFT::Return(lw[0], n);

	pRend->pRT->Clear(D2D1::ColorF(D2D1::ColorF::White));
	pRend->SetColor(D2D1::ColorF(D2D1::ColorF::Black));

	D2D1_POINT_2F p1, p2;
	{
		p1.y = p2.y = Y/2;
		p1.x = p2.x = 170;
		p1.y -= lw[0][0]/2 -0;
		for (int i = 0; i < n && p2.x < X*2; i++)
		{
			p1.y = Y/2;
			p2.y = Y - (lw[0][i])/2 - 64;
			pRend->DrawLine(p1, p2, 0.5f*dob);
			p2.x += 0.5f*dob;
			p1.x += 0.5f*dob;
		}
	}	
	{
		p1.y = p2.y = Y*2;
		p1.x = p2.x = 170;
		p1.y -= lw[1][0]/2 -0;
		for (int i = 0; i < n/2 && p2.x < X*2; i++)
		{
			p1.y = Y*2;
			p2.y = Y*2 + (lw[1][i])/16 + 0;
			pRend->DrawLine(p1, p2, 1.0f*dob);
			p2.x += 1.0*dob;
			p1 = p2;
		}
	}
	pRend->DrawNumber(1, 100, Y*1-84, 20);
	pRend->DrawNumber(2, 100, Y*2-20, 20);
	
	pRend->SetColor(D2D1::ColorF(0.2f,0.4f,1.f));

	float o = 1024000/22050; // Ширина 1000 герц

	for(int i = 0; i < 23; i++)
		pRend->DrawLine(170.f + i*o, 2.f * Y - 5, 170.f + i*o, 2.f * Y + 5);

	pRend->DrawNumber(0, 172, 2.f * Y + 10, 5);
	pRend->DrawLine(170.f, 2.f * Y - 50, 170.f, 2.f * Y + 20);

	pRend->DrawNumber(5513, 256+172, 2.f * Y + 10, 5);
	pRend->DrawLine(256+170.f, 2.f * Y - 15, 256+170.f, 2.f * Y + 20);

	pRend->DrawNumber(11025, 512+172, 2.f * Y + 10, 5);
	pRend->DrawLine(512+170.f, 2.f * Y - 25, 512+170.f, 2.f * Y + 20);

	pRend->DrawNumber(16538, 768+172, 2.f * Y + 10, 5);
	pRend->DrawLine(768+170.f, 2.f * Y - 15, 768+170.f, 2.f * Y + 20);

	pRend->DrawNumber(22050, 1024+172, 2.f * Y + 10, 5);
	pRend->DrawLine(1024+170.f, 2.f * Y - 50, 1024+170.f, 2.f * Y + 20);

	pRend->SetColor(D2D1::ColorF(1.f,0.4f,0.2f, 0.85f));


	if (lw[2] == NULL)
	{
		M = new int[n/2];
		F = new long int[n/2];
		lw[2] = new int[n/2];
		for (int i = 0; i < n/2; i++)
			M[i] = F[i] = lw[2][i] = 0;
	}

	float max = 0;
	int maxn = 0;
	for (int i = 0; i < n/2; i++)
	{
		if (lw[1][i] > max)
		{
			max = lw[1][i];
			maxn = i;
		}
	}
	pRend->DrawNumber((int)max, 10, 10, 20);
	max/=300;
	M[maxn] = 1;
	p1.y = p2.y = Y*2;
	p1.x = p2.x = 170;
	p1.y -= F[0]/2 -0;
	for (int i = 0; i < n/2 && p2.x < X*2; i++)
	{
		p1.y = Y*2;
		p2.y = Y*2 + M[i]*100;
		pRend->DrawLine(p1, p2, dob);
		p2.x += 1.0*dob;
		p1 = p2;
	}

	for (int i = 0; i < n/2; i++)//, lw[2][i-1]-=3)
		if (lw[2][i] < lw[1][i]/max)
		{
			lw[2][i] = (lw[1][i]/max);
			if (round)
				lw[2][i] *= expf(sqrtf(fabsf((7.f*(i-maxn))/n)));
		}
		else
		{
			lw[2][i] /= 1.0618;
			if (lw[2][i] < lw[1][i]/max)
				lw[2][i] = lw[1][i]/max;
		}


	if (round)
		for (int i = 0; i < n/2; i++)
			lw[1][i] *= expf(sqrtf(fabsf((7.f*(i-maxn))/n)));

	p1.y = p2.y = Y*2;
	p1.x = p2.x = 170;
	for (int i = 0; i < n/2 && p2.x < X*2; i++)
	{
		p1.y = Y*2;
		p2.y = Y*2 - (lw[2][i]);
		pRend->DrawLine(p1, p2, 1.0f*dob);
		p2.x += 1.0*dob;
		p1 = p2;
	}
	
	pRend->SetColor(D2D1::ColorF(0.25f,0.5f,1.0f, 0.75f));
	p1.y = p2.y = Y*2;
	p1.x = p2.x = 170;
	for (int i = 0; i < n/2 && p2.x < X*2; i++)
	{
		p1.y = Y*2;
		p2.y = Y*2 - lw[1][i]/max;
		pRend->DrawLine(p1, p2, 0.5f*dob);
		p2.x += 1.0*dob;
		p1 = p2;
	}
	
	for (int i = 0; i < n/2; i++)
		F[i] += lw[1][i];

	max = 0;
	for (int i = 0; i < n/2; i++)
		if (F[i] > max)
			max = F[i];
	max/=100;
	pRend->SetColor(D2D1::ColorF(1.0f,0.0f,0.0f, 1.0f));
	p1.y = p2.y = Y*2;
	p1.x = p2.x = 170;
	for (int i = 0; i < n/2 && p2.x < X*2; i++)
	{
		p1.y = Y*2;
		p2.y = Y*2 + 3*F[i]/max;
		pRend->DrawLine(p1, p2, dob);
		p2.x += 1.0*dob;
		p1 = p2;
	}


	FreeMem(&lw[0]);
	FreeMem(&lw[1]);
	FreeMem(&lw[3]);
	return hr;
}

float dist(D2D1_POINT_2F p1,D2D1_POINT_2F p2)
{
	D2D1_POINT_2F p;
	p.x = p1.x - p2.x;
	p.y = p1.y - p2.y;
	return sqrtf(p.x*p.x + p.y*p.y);
}

D2D1_POINT_2F mid(D2D1_POINT_2F p1,D2D1_POINT_2F p2)
{
	D2D1_POINT_2F p;
	p.x = (p1.x + p2.x)/2;
	p.y = (p1.y + p2.y)/2;
	return p;
}

HRESULT Osc()
{
	n = 512;
	lw[0] = pSig->GetSignal(n,3);
	lw[1] = pSig->GetSignal(n,1);

	for (int i = 0; i < n-2; i++)
	{
		lw[0][i] = lw[0][i] +lw[0][i+1];
		lw[1][i] = lw[1][i] +lw[1][i+1];
	}

	int X = GetSystemMetrics(SM_CXSCREEN)/2,
		Y = GetSystemMetrics(SM_CYSCREEN)/2;

	pRend->pRT->Clear(D2D1::ColorF(D2D1::ColorF::Black));
	pRend->SetColor(D2D1::ColorF(0.1, 1, 0.2, 0.1f));

	float f;
	D2D1_POINT_2F p1, p2;
	{
		p1.x = X - lw[0][1];
		p1.y = Y + lw[1][1];
		for (int i = 2; i < n; i++)
		{
			p2.x = X - lw[0][i];
			p2.y = Y + lw[1][i];
			//p2 = mid (p1,p2);
			f = 20.0f/dist(p1,p2);
			pRend->DrawLine(p1, p2, f > 5 ? 5 : f);
			
			p1 = p2;
		}
	}


	FreeMem(&lw[0]);
	FreeMem(&lw[1]);

	return 0;
}

};
#endif