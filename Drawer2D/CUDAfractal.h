#ifndef _CUDAFRACTAL_H_
#define _CUDAFRACTAL_H_

#include "Renderer.h"
#include "Interfaces.h"
#include <cuda_runtime_api.h>
#include <stdio.h>
#include <fstream>
#include <iostream>

extern void StartCUDAJulia(float* xpos, float* ypos, uchar4* destcol, 
						   int xPixels, int yPixels, int iter, float4 f);
extern void StartCUDAJuliaMusic(float* xpos, float* ypos, uchar4* destcol, 
						   int xPixels, int yPixels, int iter,
						   int* freq1, int* freq2, int anim);
extern void StartCUDAJuliaFFT(float* xpos, float* ypos, uchar4* destcol, 
						   int xPixels, int yPixels, int iter,
						   int* freq);
extern void StartCUDANewton(float4 posData, uchar4* destcol, 
						   int xPixels, int yPixels, int iter,
						   int* freq);
///<summary> 
/// ifct: Iter, Frac, Col, Type 
///</summary>
///<param name = "ifct">Iter, Frac, Col, Type
///</param>
extern void StartCUDAFractal(float4 posData, uchar4* destcol, 
							 int xPixels, int yPixels,
							 uint4 ifct, int* freq);

using namespace Const;

class CUDAfractal: public IFractal
{
	// Номер канала для отрисовки
	int channel;
	// Размер массива частот
	int n;
	// Массивы частот
	int* lw[4];
	int* drive;
	// Индексаторы частот
	//int f[2];
	// Тип Отрисовки
	int type;
	// Просто какая-то переменная
	int s, k;
	// Half screen size
	int X, Y;
	// Count pixel in screen
	int xPixels, yPixels;
	UINT Frac1, Frac2, Col;
	bool Type, flow;
	bool twoFrac;
	int I;
	int fps;

	float Width;
	float2 center; 

	float *xpos, *ypos;

	ID2D1Bitmap* btm;
	D2D1_BITMAP_PROPERTIES bbp;
	uchar4 *col;

public:
	CUDAfractal(Controller* c)
	{
		SetBehavior(c);
		lw[0]=lw[1]=lw[2]=lw[3]=0;
		center.x = 0;
		center.y = 0;
		channel = 0;
		Frac1=Frac2=Col=Type=flow=0;
		n = 1024;
		type = k = 0;
		this->s = twoFrac = 1;
		I = 0;
		k = 0;
		X = GetSystemMetrics(SM_CXSCREEN)/2,
		Y = GetSystemMetrics(SM_CYSCREEN)/2;
		Width = X/5.0f;
		xPixels = X*2;
		yPixels = Y*2;
		if (xPixels%16 != 0)
		xPixels += 16 - xPixels%16;
		if (yPixels%16 != 0)
		yPixels += 16 - yPixels%16;
		bbp = D2D1::BitmapProperties(pRend->pRT->GetPixelFormat());
		//pRend->pRT->CreateBitmap(CreateBitmap(D2D1::SizeU(1366, 768),bbp, &btm);
		//btm->
	}

	int Initialize()
	{
		k = 0;
		xpos = new float[xPixels],
		ypos = new float[yPixels];
		col = new uchar4[xPixels*yPixels];
		drive = new int[n/2];
		ZeroMemory(drive, sizeof(int)*n/2);
		//for (int i = 0, N=n>>2; i < N; i++)
		//	drive[i]=0;
		pRend->pRT->CreateBitmap(D2D1::SizeU(xPixels, yPixels), bbp, &btm);
		return 0;
	}
	int Destroy()
	{
		FreeMem(&xpos);
		FreeMem(&ypos);
		FreeMem(&col);
		FreeMem(&lw[0]);
		FreeMem(&lw[1]);
		FreeMem(&lw[2]);
		FreeMem(&lw[3]);
		return 0;
	}

	int Draw()
	{
		if(flow)k++;

		switch(type)
		{
		case 1:
			CUDAJuliaDraw();
			break;
		case 2:
			CUDAJuliaMusicDraw();
			break;
		case 3:
			CUDAJuliaFFTDraw();
			break;
		case 4:
			CUDANewtonDraw();
			break;
		case 5:
			CUDAFractalDraw();
			break;
		default:
			CUDAFractalDraw();
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
			center.y -= 10/Width;
			break;
		case VK_DOWN:
			center.y += 10/Width;
			break;
		case VK_LEFT:
			center.x -= 10/Width;
			break;
		case VK_RIGHT:
			center.x += 10/Width;
			break;

		case VK_SHIFT:
			Width *= 1.1;
			break;
		case VK_CONTROL:
			Width *= 0.9;
			break;

		case 'S':
			SaveImage(pRend->stop);
			break;

		case 'Q':
			s = 1 - s;
			break;
		case 'W':
			s = 1 - s;
			Sleep(100);
			s = 1 - s;
			break;
			
		case VK_NUMPAD0:
		case VK_NUMPAD1:
		case VK_NUMPAD2:
		case VK_NUMPAD3:
		case VK_NUMPAD4:
		case VK_NUMPAD5:
		case VK_NUMPAD6:
		case VK_NUMPAD7:
			Col = (key - VK_NUMPAD0);
			break;
		case VK_BACK:
			Type = 1 - Type;
			break;
		case VK_OEM_PLUS:
			Frac1++;
			break;
		case VK_OEM_MINUS:
			if (Frac1 > 0)
				Frac1--;
			break;
		case VK_OEM_6: // ]}
			Frac2++;
			break;
		case VK_OEM_4: // [{
			if (Frac2 > 0)
				Frac2--;
			break;
		case 'P':
			twoFrac = 1 - twoFrac;
			break;

		case VK_NUMPAD9:
			flow = 1 - flow;
			break;

		case VK_SPACE:
			{
			Frac1 = rand()%30;
			Frac2 = rand()%30;
			Type = rand()%2;
			Col = rand()%8;
		}break;
		case VK_OEM_5:
			{
			int t = Frac1;
			Frac1 = Frac2;
			Frac2 = t;
			break;
			}
			
		case 'F':
			Frac1 = 18;
			Frac2 = Type = 0;
			Col = 7;
			break;
		case 'G':
			Frac1 = 16;
			Frac2 = 9;
			Type = 0;
			Col = 6;
			break;
		case 'H':
			Frac1 = 24;
			Frac2 = 11;
			Type = 0;
			Col = 4;
			break;
		case 'J':
			Frac1 = 8;
			Frac2 = 1;
			Type = 1;
			Col = 6;
			break;
		case VK_MENU:
			center.x=center.y=0;
			Width = X/5.0f;
			I = 0;
			break;

		case VK_OEM_COMMA:
			I -= 5;
			break;
		case VK_OEM_PERIOD:
			I += 5;
			break;

		default:
			break;
		}
		return 0;
	}

	void SaveImage(bool stoped)
	{
		pRend->stop = stoped ? pRend->stop : true;
		wchar_t name[15] = L"Image\\0000.BMP";
		LPCWSTR namefile = (LPCWSTR)name;
		int i = 0;
		while (GetFileAttributes(namefile) != 0xffffffff)
		{
			i++;
			name[9] = (WCHAR)(L'0' + i%10);
			name[8] = (WCHAR)(L'0' + (i/10)%10);
			name[7] = (WCHAR)(L'0' + (i/100)%10);
			name[6] = (WCHAR)(L'0' + (i/1000)%10);
			namefile = (LPCWSTR)name;
		}

		int size = xPixels * yPixels * sizeof(uchar4);
		
		BITMAPFILEHEADER  hdr;
		ZeroMemory(&hdr, sizeof(BITMAPFILEHEADER));
		hdr.bfType    = 0x4D42;
		hdr.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		hdr.bfSize    = hdr.bfOffBits + size;

		// заголовок описателя растра
		BITMAPINFO dib;
		ZeroMemory(&dib, sizeof(BITMAPINFO));
		dib.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
		dib.bmiHeader.biBitCount  = sizeof(uchar4)*8;
		dib.bmiHeader.biCompression  = BI_RGB;
		dib.bmiHeader.biPlanes  = 1u;
		dib.bmiHeader.biWidth   = (long)xPixels;
		dib.bmiHeader.biHeight  = (long)yPixels;
		dib.bmiHeader.biSizeImage   = size;
		dib.bmiHeader.biXPelsPerMeter = 11811L;
		dib.bmiHeader.biYPelsPerMeter = 11811L;
		dib.bmiHeader.biClrImportant  = 0uL;
		dib.bmiHeader.biClrUsed  = 0uL;
		
		
		HANDLE fp = CreateFile((LPCWSTR)name, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, 
								CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		DWORD dwr = 0;
		WriteFile(fp, (LPCVOID)&hdr, sizeof(BITMAPFILEHEADER), &dwr, NULL);
		WriteFile(fp, (LPCVOID)&dib.bmiHeader, sizeof(BITMAPINFOHEADER), &dwr, NULL);
		uchar4 *bmp = new uchar4[xPixels*yPixels];
		for (int i = 0; i < yPixels; i++)
			for (int j = 0; j < xPixels; j++)
				bmp[j + i*xPixels] = col[j + (yPixels-i-1)*xPixels];

		WriteFile(fp, (LPCVOID)bmp, size, &dwr, NULL);
		pRend->stop = stoped ? pRend->stop : false;
		delete[] bmp;

		FlushFileBuffers(fp);
		CloseHandle(fp);
	}

	int Sum(int* f,int n){int s=0;for(int i=0;i<n;i++,s+=f[i]);return s;}
	float4 f;
	HRESULT CUDAJuliaDraw()
	{
		HRESULT hr = S_OK;

		int iter = (5)*10;
		float step;
		float width = Width;
		float	// Centre of fractal
				x = center.x, 
				y = center.y; 
		float	// Size of fractal window
				xsize = X / width, 
				ysize = Y / width;

		step = (1.0/width);

		xpos[0] = x - xsize;
		ypos[0] = y - ysize;
		for (int i = 1; i < xPixels; i++)
			xpos[i] = xpos[i-1] + step;
		for (int i = 1; i < yPixels; i++)
			ypos[i] = ypos[i-1] + step;

		if(s)
		{
		lw[0] = pSig->GetSignal(210, 1);
		lw[1] = pSig->GetSignal(210, 3);
		f.x = sinf(Sum(lw[0], 100) / 4700.0f)/5;
		f.y = cosf(Sum(lw[0]+100, 100) / 1700.0f)/5;
		f.w = sinf(Sum(lw[1], 100) / 3100.0f)/5;
		f.z = cosf(Sum(lw[1]+100, 100) / 1300.0f)/5;
		FreeMem(&lw[0]);
		FreeMem(&lw[1]);
		}

		StartCUDAJulia(xpos, ypos, col, xPixels, yPixels, iter, f);

		btm->CopyFromMemory(0, col, xPixels*sizeof(uchar4));

		pRend->pRT->Clear(D2D1::ColorF(D2D1::ColorF::White));

		pRend->pRT->DrawBitmap(btm);

		//pRend->SetColor(D2D1::ColorF(D2D1::ColorF::Red));
		//pRend->DrawNumber(pRend->k,10,10,20);

		return hr;
	}

	HRESULT CUDAJuliaMusicDraw()
	{
		HRESULT hr = S_OK;

		int iter = (6)*10;
		float step;
		float width = Width;
		float	// Centre of fractal
				x = center.x, 
				y = center.y; 
		float	// Size of fractal window
				xsize = X / width, 
				ysize = Y / width;

		step = (1.0/width);

		xpos[0] = x - xsize;
		ypos[0] = y - ysize;
		for (int i = 1; i < xPixels; i++)
			xpos[i] = xpos[i-1] + step;
		for (int i = 1; i < yPixels; i++)
			ypos[i] = ypos[i-1] + step;

		float4 f;
		lw[0] = pSig->GetSignal(iter, 1);
		lw[1] = pSig->GetSignal(iter, 3);

		StartCUDAJuliaMusic(xpos, ypos, col, xPixels, yPixels, iter, lw[0], lw[1], pRend->k);

		FreeMem(&lw[0]);
		FreeMem(&lw[1]);


		btm->CopyFromMemory(0, col, xPixels*sizeof(uchar4));
		
		//pRend->SetColor(D2D1::ColorF(0.f,0.f,0.f,0.5f));
		//pRend->pRT->FillRectangle(D2D1::RectF(0,0, X*2,Y*2), pRend->pBrush); 
		//pRend->pRT->Clear(D2D1::ColorF(D2D1::ColorF::White));

		pRend->pRT->DrawBitmap(btm);

		//pRend->SetColor(D2D1::ColorF(D2D1::ColorF::Red));
		//pRend->DrawNumber(pRend->k,10,10,20);

		return hr;
	}

	HRESULT CUDAJuliaFFTDraw()
	{
		HRESULT hr = S_OK;

		int iter = (6)*10;
		float step;
		float width = Width;
		float	// Centre of fractal
				x = center.x, 
				y = center.y; 
		float	// Size of fractal window
				xsize = X / width, 
				ysize = Y / width;

		step = (1.0/width);

		xpos[0] = x - xsize;
		ypos[0] = y - ysize;
		for (int i = 1; i < xPixels; i++)
			xpos[i] = xpos[i-1] + step;
		for (int i = 1; i < yPixels; i++)
			ypos[i] = ypos[i-1] + step;

		float4 f;
		lw[0] = pSig->GetSignal(n, 1);
		lw[1] = pSig->GetSignal(n, 3);
		lw[2] = new int[n];

		//#pragma omp parallel for shared(a, b, c) private(i)
		for (int i = 0; i < n; i++)
			lw[2][i] = (lw[0][i] + lw[1][i])/2;

		lw[3] = FFT::Return(lw[2], n);

		StartCUDAJuliaFFT(xpos, ypos, col, xPixels, yPixels, iter, lw[3]);

		FreeMem(&lw[0]);
		FreeMem(&lw[1]);
		FreeMem(&lw[2]);
		FreeMem(&lw[3]);


		btm->CopyFromMemory(0, col, xPixels*sizeof(uchar4));

		pRend->pRT->DrawBitmap(btm);

		pRend->SetColor(D2D1::ColorF(D2D1::ColorF::Red));
		pRend->DrawNumber(pRend->k++,10,10,20);

		return hr;
	}

	HRESULT CUDANewtonDraw()
	{
		HRESULT hr = S_OK;
		int S = s;
		int iter = (5)*20;
		float step;
		float width = Width;
		float	// Centre of fractal
				x = center.x, 
				y = center.y; 
		float	// Size of fractal window
				xsize = X / width, 
				ysize = Y / width;

		step = (1.0f/width);

		float4 posData;
		posData.x = x - xsize;
		posData.y = y - ysize;
		posData.z = 0;
		posData.w = step;
		
		if (S)
		{
			if (lw[0] != NULL)
			{
		free(lw[0]);
		free(lw[1]);
		free(lw[2]);
		free(lw[3]);
			}

		lw[0] = pSig->GetSignal(n, 1);
		lw[1] = pSig->GetSignal(n, 3);
		lw[2] = new int[n];

		for (int i = 0; i < n; i++)
			lw[2][i] = (lw[0][i] + lw[1][i])/2;

		lw[3] = FFT::Return(lw[2], n);
		}

		iter = pRend->k*256 + iter; 
		StartCUDANewton(posData, col, xPixels, yPixels, iter, lw[3]);

		btm->CopyFromMemory(0, col, xPixels*sizeof(uchar4));

		pRend->pRT->DrawBitmap(btm, D2D1::RectF(0.f, 0.f, pRend->Xscreen, pRend->Yscreen));

		pRend->SetColor(D2D1::ColorF(D2D1::ColorF::Red));
		pRend->DrawNumber(pRend->k++,10,10,20);

		return hr;
	}
	
	HRESULT CUDAFractalDraw()
	{
		int oldfps = fps;
		fps = clock();
		HRESULT hr = S_OK;
		int S = s;
		int iter = 60+I;
		float step;
		float width = Width;
		float	// Centre of fractal
				x = center.x, 
				y = center.y; 
		float	// Size of fractal window
				xsize = X / width, 
				ysize = Y / width;

		step = (1.0f/width);

		float4 posData;
		posData.x = x - xsize;
		posData.y = y - ysize;
		posData.z = 0;
		posData.w = step;
		
		if (S)
		{
			if (lw[0] != NULL)
			{
		free(lw[0]);
		free(lw[1]);
		free(lw[2]);
		free(lw[3]);
			}
			
		lw[0] = pSig->GetSignal(n, 1);
		lw[1] = pSig->GetSignal(n, 3);
		lw[2] = new int[n];

		for (int i = 0; i < n; i++)
			lw[2][i] = (lw[0][i] + lw[1][i])/2;

		lw[3] = FFT::Return(lw[2], n);
		}
		int max = 0;
		for (int i = 0, N = n/2; i < N; i++)
			if (lw[3][i] > max)
				max = lw[3][i];
		//int m = max / (oldfps);
		
		for (int i = 0, N = n/2; i < N; i++)
			if (drive[i] < lw[3][i])
				drive[i] = lw[3][i];
			else 
			{
				drive[i] *= 0.618;
				if (drive[i] < lw[3][i])
					drive[i] = lw[3][i];
				//drive[i]-=m;
				//if (drive[i] < 0)
				//	drive[i] = 0;
			}

		iter = pRend->k*256 + iter;

		uint4 ifct;
		ifct.x = k*512;
			iter = 60;
		ifct.x += iter;
		ifct.y = Frac1*1024 + Frac2*2 + twoFrac;
		ifct.z = Col;
		ifct.w = Type;
		StartCUDAFractal(posData, col, xPixels, yPixels, ifct, drive);

		btm->CopyFromMemory(0, col, xPixels*sizeof(uchar4));

		pRend->pRT->DrawBitmap(btm, D2D1::RectF(0.f, 0.f, pRend->Xscreen, pRend->Yscreen));

		pRend->SetColor(D2D1::ColorF(D2D1::ColorF::Red));
		pRend->DrawNumber(iter,10,10,4);
		pRend->DrawNumber(Frac1,10,60,4);
		pRend->DrawNumber(Frac2*(2*twoFrac-1),10,110,4);
		pRend->DrawNumber(ifct.w,10,160,4);
		fps = 1000/(clock()-fps);
		pRend->DrawNumber((fps+oldfps)/2,10,718,4);

		return hr;
	}

};
#endif