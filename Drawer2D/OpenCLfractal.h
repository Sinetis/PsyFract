#pragma once

#include "Renderer.h"
#include "Interfaces.h"
#include "resource.h"
#include <stdio.h>
#include <fstream>
#include <string>
#include <iostream>
#include <CL\cl.h>
#include <CL\cl_platform.h>
#include <CL\cl_ext.h>

using namespace Const;

#define MAX_SOURCE_SIZE 16777216
#define MAX_FRACS 32

#define float4 cl_float4
#define uchar4 cl_uchar4
#define uint4 cl_uint4

struct data
{
	int iter;
	float k;
	unsigned int frac;
	unsigned int col;
};

class OpenCLfractal: public IFractal
{
	cl_platform_id platform_id;
	cl_device_id device_id;
	cl_context context;
	cl_program program;
	cl_kernel kernel;
	cl_command_queue command_queue;
	LPSTR OpenCLString;

	// Номер канала для отрисовки
	int channel;
	// Размер массива частот
	int n;
	// Массивы частот
	int* lw;
	int ci[8];
	// Индексаторы частот
	//int f[2];

	// Тип Отрисовки
	int type;
	// Просто какая-то переменная
	int B;
	float speed;
	float power, colorPower;
	float k, lk;
	// Half screen size
	int X, Y;
	// Count pixel in screen
	int xPixels, yPixels;
	UINT Frac1, Frac2, Col;
	UINT prevFrac1, prevFrac2, prevCol;
	bool Type, prevType, flow;
	bool twoFrac;
	bool control;
	int numFrac, sqrnum;
	int I;
	float fps;
	int mouseHoverFrac;
	int mouseDownOn;
	bool info;

	float Width;
	float2 center; 

	float *xpos, *ypos;

	ID2D1Bitmap* btm;
	D2D1_BITMAP_PROPERTIES bbp;
	uchar4 *col;

public:
	OpenCLfractal(Controller* c)
	{
		SetBehavior(c);
		InitVariables();
		
		xPixels = X*2;
		yPixels = Y*2;
		if (xPixels%16 != 0)
		xPixels += 16 - xPixels%16;
		if (yPixels%16 != 0)
		yPixels += 16 - yPixels%16;

		bbp = D2D1::BitmapProperties(pRend->pRT->GetPixelFormat());
		//pRend->pRT->CreateBitmap(D2D1::SizeU(xPixels, yPixels),bbp, &btm);
		//ResizeBMP(xPixels/2, yPixels/2);

		OpenCLString = ("Нет такого.cl");
		
		//OpenCLString = ("OpenCLfunction.cl");
		LPSTR str = GetCommandLineA();
		//MessageBoxA(0,str,"-", MB_OK);
		if (str != NULL)
		{
			int i, l = strlen(str);
			//LPSTR cmp(".cl");
			//for (i = 0; i < 3 && i < l; i++)
			//	if (str[l-4 + i] != cmp[i])
			//		break;
			if (str[l-4] == '.' && str[l-3] == 'c' && str[l-2] == 'l')
			{
				for (i = l-3; str[i]!='\"'; i--);
				//delete OpenCLString;
				OpenCLString = new char[l-i-1];
				for(int j = i+1; j < l-1; j++)
					OpenCLString[j-i-1] = str[j];
				OpenCLString[l-i-2] = '\0';
			}
			//delete str;
		}
		//MessageBoxA(0,OpenCLString,"-", MB_OK);
	}

	void InitVariables()
	{
		for (int i = 0; i < 8; i++)
			ci[i] = i;
		//ci[0] = 5;ci[4] = 7;
		//ci[1] = 6;ci[5] = 4;
		//ci[2] = 1;ci[6] = 0;
		//ci[3] = 3;ci[7] = 2;
		c1.x = c2.x = c3.x = c4.x = 
		c1.y = c2.y = c3.y = c4.y = 0.f;
		control = false;
		mouseHoverFrac = mouseDownOn = 0;
		srand(time(0));
		lw = 0;
		center.x = 0;
		center.y = 0;
		channel = 0;
		Frac1=Frac2=Col=Type=flow=0;
		numFrac = 4;
		n = 1024;
		B = 1;
		type = k = 0;
		twoFrac = 1;
		speed = colorPower = 1;
		power = 2;
		I = 60;
		fps = 0;
		info = false;

		prevCol = prevFrac1 = prevFrac2 = prevType = 0;
		X = GetSystemMetrics(SM_CXSCREEN)/2,
		Y = GetSystemMetrics(SM_CYSCREEN)/2;
		Width = X/5.0f;
	}

	void ResizeBMP(int x, int y)
	{
		pRend->stop = true;
		pRend->WaitDrawing();
		btm->Release();
		X = x/2,
		Y = y/2;
		Width = X/5.0f;
		xPixels = x;
		yPixels = y;
		if (xPixels%16 != 0)
		xPixels += 16 - xPixels%16;
		if (yPixels%16 != 0)
		yPixels += 16 - yPixels%16;
		

		delete xpos;
		delete ypos;
		delete col;
		xpos = new float[xPixels];
		ypos = new float[yPixels];
		col = new uchar4[xPixels*yPixels];
		pRend->pRT->CreateBitmap(D2D1::SizeU(xPixels, yPixels), bbp, &btm);
		pRend->stop = false;
	}

	int Initialize()
	{
		pSig->stop = false;

		xpos = new float[xPixels],
		ypos = new float[yPixels];
		col = new uchar4[xPixels*yPixels];
		pRend->pRT->CreateBitmap(D2D1::SizeU(xPixels, yPixels), bbp, &btm);

		cl_int ret;
		cl_uint ret_num_platforms;
		cl_uint ret_num_devices;

		/* получить доступные платформы */
		ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
		if(ret)
			MessageBoxA(NULL, "clGetPlatformIDs(1, &platform_id, &ret_num_platforms);", "Ошибка в строке:", MB_OK);

		/* получить доступные устройства */
		ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, &ret_num_devices);
		if(ret)
			MessageBoxA(NULL, "clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, &ret_num_devices);", "Ошибка в строке:", MB_OK);

		/* создать контекст */
		context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
		if(ret)
			MessageBoxA(NULL, "clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);", "Ошибка в строке:", MB_OK);

		/* создаем команду */
		command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
		if(ret)
			MessageBoxA(NULL, "clCreateCommandQueue(context, device_id, 0, &ret);", "Ошибка в строке:", MB_OK);

		program = NULL;
		kernel = NULL;

		FILE *fp;
		size_t source_size;
		char *source_str;
		int i;

		fp = fopen(OpenCLString, "r");
		if (fp) 
		{
		source_str = (char *)malloc(MAX_SOURCE_SIZE);
		source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
		fclose(fp);
		}
		else
		{
			HRSRC src = FindResource(NULL, MAKEINTRESOURCE(IDR_CL1), L"CL");
			source_size = (size_t)SizeofResource(NULL, src);
			source_str = (char*)LockResource( LoadResource(NULL, src));		
		}

		/* создать бинарник из кода программы */
		program = clCreateProgramWithSource(
			context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);
		if(ret)
			MessageBoxA(NULL, "clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);", "Ошибка в строке:", MB_OK);
		
		if (fp){ 
		free(source_str);
		}

		/* скомпилировать программу */
		ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
		if(ret)
			MessageBoxA(NULL, "clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);", "Ошибка в строке:", MB_OK);
		
		cl_program_build_info info;
		clGetProgramBuildInfo(
			program, device_id, CL_PROGRAM_BUILD_LOG, NULL, NULL, &source_size);
		source_str = new char[source_size];
		clGetProgramBuildInfo(
			program, device_id, CL_PROGRAM_BUILD_LOG, source_size, (void*)source_str, NULL);
		if (source_size > 5)
		{
			MessageBoxA(NULL, source_str, "Error:", MB_OK);
			exit(0);
		}
		
		/* создать кернел */
		kernel = clCreateKernel(program, "OpenCLFractal", &ret);
		if(ret)
			MessageBoxA(NULL, "clCreateKernel(program, \"OpenCLFractal\", &ret);", "Ошибка в строке:", MB_OK);
		return 0;
	}

	int Destroy()
	{
		pSig->stop = true;
		if (xpos != NULL)
		FreeMem(&xpos);
		FreeMem(&ypos);
		if (col != NULL)
		FreeMem(&col);
		return 0;
	}

	int Draw()
	{
		//if (clock()-lk > 16)
			

		if (flow)
			k += speed*lk/1000.0f;
		
		lk = clock();
		clock_t t1 = clock();


		float oldfps = fps;
		fps = lk;		
		
		switch(type)
		{
		case 1:
			OpenCLFractalDraw();
			break;
		case 2:
			FewFractalDraw();
			break;
		case 3:
			PointsFractalDraw();
			break;
		default:
			OpenCLFractalDraw();
		}	

		if (clock() - t1 < 16)
			Sleep(16. - (clock() - t1));

		if (info)
		{
			fps = clock() - t1;
			fps = 1000.f / fps;
			fps = (fps + oldfps)/2;
			int j = 0;
			WCHAR *buffer = new WCHAR[1000];
			if (Type)
				j  = swprintf( buffer,     L"   Фрактал: \t%s\n", L"По Мандельброту" );
			else
				j  = swprintf( buffer,     L"   Фрактал: \t%s\n", L"По Жюлиа" );
			j += swprintf( buffer + j, L"   Время: \t%f\n", k );
			j += swprintf( buffer + j, L"   Первая формула: \t%d\n", Frac1 );
			j += swprintf( buffer + j, L"   Вторая формула: \t%d\n", Frac2 );
			j += swprintf( buffer + j, L"   Глубина: \t%d\n", I );
			j += swprintf( buffer + j, L"   X: \t%f\n", center.x);
			j += swprintf( buffer + j, L"   Y: \t%f\n", center.y);
			j += swprintf( buffer + j, L"   Масштаб по Y: \t%f\n", 2*Y/Width);
			j += swprintf( buffer + j, L"   Скорость времени: \t%f\n", speed);
			j += swprintf( buffer + j, L"   Сила взаимодействия: \t%f\n", 1/power);
			j += swprintf( buffer + j, L"   Сила цвета: \t%f\n", colorPower);
			j += swprintf( buffer + j, L"   Цветовая палитра #: \t%d\n", Col);
			j += swprintf( buffer + j, L"   Twist: \t%d%d%d%d%d%d%d%d\n", ci[0],ci[1],ci[2],ci[3],ci[4],ci[5],ci[6],ci[7]);
			j += swprintf( buffer + j, L"   FPS: \t%.3f\n", fps);
			j += swprintf( buffer + j, L"   c[0].x: \t%.3f\n", com[0].x);
			j += swprintf( buffer + j, L"   c[0].y: \t%.3f\n", com[0].y);
			j += swprintf( buffer + j, L"   c[1].x: \t%.3f\n", com[1].x);
			j += swprintf( buffer + j, L"   c[1].y: \t%.3f\n", com[1].y);
			j += swprintf( buffer + j, L"   c[2].x: \t%.3f\n", com[2].x);
			j += swprintf( buffer + j, L"   c[2].y: \t%.3f\n", com[2].y);
			j += swprintf( buffer + j, L"   c[3].x: \t%.3f\n", com[3].x);
			j += swprintf( buffer + j, L"   c[3].y: \t%.3f\n", com[3].y);
			POINT cur;
			GetCursorPos(&cur);
			j += swprintf( buffer + j, L"   CX: \t%d\n", (int)cur.x);
			j += swprintf( buffer + j, L"   CY: \t%d\n", (int)cur.y);
			j += swprintf( buffer + j, L"   c[4].x: \t%.3f\n", com[4].x);
			j += swprintf( buffer + j, L"   c[4].y: \t%.3f\n", com[4].y);


			pRend->DrawString(10,10, buffer);
			pRend->DrawLine(X-5, Y-5, X-5, Y+5,0.5f);
			pRend->DrawLine(X-10, Y, X+0, Y,0.5f);

			delete[] buffer;
		}
		//delete str;
		//pRend->pRT->DrawText(L"lkj",wcslen,NULL,D2D1::RectF(0,0,100,100), pRend->pBrush);
		
		lk = clock() - lk;


		return 0;
	}

	void SwitchType(int t)
	{
		static int prev;

		if (t != prev)
		{
			type = t;
			prev = t;
			switch (type)
			{
			case 2:
				{
				sqrnum = (sqrtf(numFrac)+0.99f);
				ResizeBMP(pRend->Xscreen/sqrnum, pRend->Yscreen/sqrnum);
				break;
				}
			case 3:
				ResizeBMP(pRend->Xscreen - (pRend->Yscreen>>2), pRend->Yscreen);
				break;
			default:
				ResizeBMP(pRend->Xscreen, pRend->Yscreen);
				break;
			}
		}
	}

	void SwitchFrac(int f1, int f2)
	{
		pRend->stop = true;
		pRend->WaitDrawing();
		prevFrac1 = Frac1;
		prevFrac2 = Frac2;
		Frac1 = f1;
		Frac2 = f2;
		pRend->stop = false;
	}
	void ModFrac(bool first, bool inc)
	{
		pRend->stop = true;
		pRend->WaitDrawing();	
		prevFrac1 = Frac1;
		prevFrac2 = Frac2;
		if (first)
		{
			if (inc)
				Frac1++;
			else
				if (Frac1 > 0)
					Frac1--;
		}
		else
			if (inc)
				Frac2++;
			else
				if (Frac2 > 0)
					Frac2--;
		pRend->stop = false;
	}

	int CatchMessage(WPARAM key)
	{

		switch(key)
		{
		case VK_LBUTTON:
			if (type == 3)
			{
				LPPOINT p = new POINT;
				GetCursorPos(p);
				mouseDownOn = p->y / (pRend->Yscreen>>2) + 1;
			}
			if (type != 2)
				break;
			pRend->stop = true;
			pRend->WaitDrawing();
			Frac1 = mouseHoverFrac;
			SwitchType(1);
			pRend->stop = false;
			break;
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
			if (GetKeyState(0xC0) & 0x8000)
				CatchMessage(VK_NUMPAD0 + key - '0');
			else
				SwitchType(key - '0');
			break;
		case VK_UP:
			center.y -= 5/Width;
			break;
		case VK_DOWN:
			center.y += 5/Width;
			break;
		case VK_LEFT:
			center.x -= 5/Width;
			break;
		case VK_RIGHT:
			center.x += 5/Width;
			break;

		case VK_SHIFT:
			//Width *= 1.1;
			break;
		case VK_CONTROL:
			if (GetKeyState(VK_SHIFT) & 0x8000)
				Width *= 1.05;
			else
				Width *= 0.95;
			break;

		case 'S':
			SaveImage(pRend->stop);
			break;
		case 'D':
			{
			//int x = xPixels, y = yPixels;
			if (abs(yPixels - pRend->Yscreen)<16)
				ResizeBMP(pRend->Xscreen*2,pRend->Yscreen*2);
			else
				ResizeBMP(pRend->Xscreen, pRend->Yscreen);
			//Sleep(100);
			//SaveImage(false);
			break;
			}
		case VK_NUMPAD0:
		case VK_NUMPAD1:
		case VK_NUMPAD2:
		case VK_NUMPAD3:
		case VK_NUMPAD4:
		case VK_NUMPAD5:
		case VK_NUMPAD6:
		case VK_NUMPAD7:
			prevCol = Col;
			Col = (key - VK_NUMPAD0);
			break;
		case VK_BACK:
			prevType = Type;
			Type = Type ? false : true;
			break;
		case 'N':
			if (GetKeyState(VK_SHIFT) & 0x8000)
			{
				power = 2;
				break;
			}
			power *= 1.1f;
			break;
		case 'M':
			power *= 0.9f;
			break;
		case 'L':
			colorPower *= 1.1f;
			break;
		case 'K':
			if (GetKeyState(VK_SHIFT) & 0x8000)
			{
				colorPower = 1;
				break;
			}
			colorPower *= 0.9f;
			break;
		case VK_OEM_PLUS:
			if (GetKeyState(0xC0) & 0x8000)
				{CatchMessage(VK_MULTIPLY);
				break;}
			ModFrac(1, 1);
			break;
		case VK_OEM_MINUS:			
			if (GetKeyState(0xC0) & 0x8000)
				{CatchMessage(VK_DIVIDE);
				break;}
			if (GetKeyState(VK_SHIFT) & 0x8000)
			{
				SwitchFrac(0,Frac2);
				break;
			}
			ModFrac(1, 0);
			break;
		case VK_OEM_6: // ]}
			ModFrac(0, 1);
			//Frac2++;
			break;
		case VK_OEM_4: // [{
			if (GetKeyState(VK_SHIFT) & 0x8000)
			{
				SwitchFrac(Frac1,0);
				break;
			}
			ModFrac(0, 0);
			//Frac2--;
			break;
		case 'P':
			twoFrac = twoFrac ? false : true;
			break;
		case 'O':
			control = control ? false : true;
			break;
		case 'R':
			{
			if (GetKeyState(VK_SHIFT) & 0x8000)
			{
				for (int i = 0; i < 8; i++)
					ci[i] = i;
				break;
			}
			int b = 0, k;
			for(int i = 0; i < 8; i++)
			{
				k = rand()%8;
				while (b & (1<<k))
					k = rand()%8;
				b |= (1<<k);
				ci[k] = i;
				//k = 8 - rand()%(8-i);
				//b = ci[i];
				//ci[i] = ci[k];
				//ci[k] = b;
			}
			break;
			}
		case 'I':
			info = info? false: true;
			break;

		case VK_NUMPAD9:
			if (GetKeyState(VK_SHIFT) & 0x8000)
			{
				k = 0;
				break;
			}
			flow = flow ? false: true;
			break;

		case VK_SPACE:
			{
			if (GetKeyState(VK_SHIFT) & 0x8000)
			{
				SwitchFrac(prevFrac1, prevFrac2);
				int t = Type, c = Col;
				Type = prevType;
				Col = prevCol;
				prevType = t;
				prevCol = c;
				break;
			}
			SwitchFrac(rand()%MAX_FRACS, rand()%MAX_FRACS);
			prevType = Type;
			Type = rand()%2;
			prevCol = Col;
			//Col = rand()%8;
		}break;
		case VK_OEM_5:
			{
			SwitchFrac(Frac2, Frac1);
			break;
			}

		case VK_MENU:
			srand(clock());
			//for (int i = 0; i < 8; i++)
			//	ci[i] = i;
			center.x=center.y=0;
			Width = X/5.0f;
			I = 60;
			break;

		case VK_OEM_2: // ?/
			InitVariables();
			break;

		case VK_OEM_COMMA: // <
			if (GetKeyState(VK_SHIFT) & 0x8000)
			{
				I = 60;
				break;
			}
			if (I > 11)
			I *= 0.9;
			break;
		case VK_OEM_PERIOD: // >
			I *= 1.1;
			break;

		case VK_OEM_1: //:;
			if (GetKeyState(VK_SHIFT) & 0x8000)
			{
				speed = 1;
				break;
			}
			speed *= 0.9;
			break;
		case VK_OEM_7: //'"
			speed *= 1.1;
			break;

		case VK_MULTIPLY:
			numFrac++;
			sqrnum = (sqrtf(numFrac)+0.99f);
			ResizeBMP(pRend->Xscreen/sqrnum, pRend->Yscreen/sqrnum);
			break;
		case VK_DIVIDE:
			numFrac--;
			sqrnum = (sqrtf(numFrac)+0.99f);
			ResizeBMP(pRend->Xscreen/sqrnum, pRend->Yscreen/sqrnum);
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
		CreateDirectory(L"Image", NULL);
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

data st;
float2 com[5];
	void StartOpenCLFractal(float4 posData, uchar4* destcol, 
							   int iter, int* freq)
	{
		float *c = new float[8];
		for (int i = 0; i < 8; i++)
			c[i] = 0;
		//int T[8] = {-1,129,236,300,349,422,464,499};
		//int T[8] = {0,5,10,15,20,25,30,35};
		//Frac1 = 1;
		int i;
		//for (i = 1; i < 8; i++, Frac1++)
		//	if (k < T[i] && k >= T[i-1])
		//		break;
		
		//st.col = Col + (T[i-1]<<3);
		st.col = Col;
		st.frac = Type + twoFrac*2 + (Frac1<<2) + (Frac2<<10) + (prevFrac1<<18) + (prevFrac2<<25);
		st.iter = iter;
		st.k = k;

		float2* comn = new float2[4];


	int xx = 0;

	int S = 40;
	float k = 3;
	int I[2][8] = { {0,	 64,  128, 192,	256, 320, 384, 448},
					{64, 128, 192, 256,	320, 384, 448, 512}};
	for (int j = 0; j < 8; j++)
		{I[0][j] /= k; I[1][j] /= k;}
	comn[0].x=comn[1].x=comn[2].x=comn[3].x=
	comn[0].y=comn[1].y=comn[2].y=comn[3].y=0;
	for (int j = 0; j < 8; j++)
	{
		if (j%2==0)
		for (i = I[0][j]; i < I[1][j]; i++)
			if (freq[i] > xx)
				comn[j/2].x  += freq[i];
		if (j%2==1)
		for (i = I[0][j]; i < I[1][j]; i++)
			if (freq[i] > xx)
				comn[j/2].y += freq[i];
	}

	float max = 1;
	int mid = 0;
	{
		if (comn[0].x > max)
			max = comn[0].x;
		if (comn[1].x > max)
			max = comn[1].x;
		if (comn[2].x > max)
			max = comn[2].x;
		if (comn[3].x > max)
			max = comn[3].x;
		if (comn[0].y > max)
			max = comn[0].y;
		if (comn[1].y > max)
			max = comn[1].y;
		if (comn[2].y > max)
			max = comn[2].y;
		if (comn[3].y > max)
			max = comn[3].y;
		
		mid += comn[0].x;
		mid += comn[1].x;
		mid += comn[2].x;
		mid += comn[3].x;
		mid += comn[0].y;
		mid += comn[1].y;
		mid += comn[2].y;
		mid += comn[3].y;
		mid >>= 3;
	}
	//float x, y;
	//max -= mid;
	max*=power;
	for (int i = 0; i < 4; i++)
	{
		com[i].x = (comn[i].x)/(mid*power);
		com[i].y = (comn[i].y)/(mid*power);
	}
	//com[0].x = mid;
		
	if (!control)
	{
	c1.x = com[0].x;
	c2.x = com[1].x;
	c3.x = com[2].x;
	c4.x = com[3].x;

	c1.y = com[0].y;
	c2.y = com[1].y;
	c3.y = com[2].y;
	c4.y = com[3].y;
	}

	POINT cur;
	GetCursorPos(&cur);
	com[4].x = (cur.x - X)/Width + center.x;
	com[4].y = (cur.y - Y)/Width + center.y;
	//center.x = (cur.x - X)/Width;
	//center.y = (cur.y - Y)/Width;
	OpenCL(posData, destcol);

	delete[] c;
	}


	void StartControlOpenCLFractal(float4 posData, uchar4* destcol, 
							   int iter)
	{
		data st;
		st.col = Col;
		st.frac = Type + twoFrac*2 + (Frac1<<2) + (Frac2<<10) + (prevFrac1<<18) + (prevFrac2<<25);
		st.iter = iter;
		st.k = k;

		com[0].x = c1.x;
		com[1].x = c2.x;
		com[2].x = c3.x;
		com[3].x = c4.x;

		com[0].y = c1.y;
		com[1].y = c2.y;
		com[2].y = c3.y;
		com[3].y = c4.y;

		OpenCL(posData, destcol);
	}

	void OpenCL(float4 posData, uchar4* destcol)						   
	{
		cl_mem	coldev = NULL,
				devCom = NULL;
		cl_int ret;

		/* создать буфер */
		coldev = clCreateBuffer(
			context, CL_MEM_WRITE_ONLY, xPixels*yPixels * sizeof(cl_uchar4), NULL, &ret);
		if(ret)
			MessageBoxA(NULL, "clCreateBuffer", "Ошибка в строке:", MB_OK);
		devCom = clCreateBuffer(
			context, CL_MEM_WRITE_ONLY, 5 * sizeof(cl_float2), NULL, &ret);
		if(ret)
			MessageBoxA(NULL, "clCreateBuffer", "Ошибка в строке:", MB_OK);
		
		
		
		/* записать данные в буфер */
		ret = clEnqueueWriteBuffer(
			command_queue, devCom, CL_TRUE, 0, 5 * sizeof(cl_float2), (void*)com, 0, NULL, NULL);
		if(ret)
			MessageBoxA(NULL, "clEnqueueWriteBuffer", "Ошибка в строке:", MB_OK);

		/* устанавливаем параметр */
		if( clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&coldev) )
			MessageBoxA(NULL, "clSetKernelArg", "Ошибка в строке:", MB_OK);

		if( clSetKernelArg(kernel, 1, sizeof(posData), (void *)&posData) )
			MessageBoxA(NULL, "clSetKernelArg", "Ошибка в строке:", MB_OK);

		if( clSetKernelArg(kernel, 2, sizeof(xPixels), (void *)&xPixels) )
			MessageBoxA(NULL, "clSetKernelArg", "Ошибка в строке:", MB_OK);

		if( clSetKernelArg(kernel, 3, sizeof(data), (void *)&st) )
			MessageBoxA(NULL, "clSetKernelArg", "Ошибка в строке:", MB_OK);

		if( clSetKernelArg(kernel, 4, sizeof(cl_mem), (void *)&devCom) )
			MessageBoxA(NULL, "clSetKernelArg", "Ошибка в строке:", MB_OK);
		//ret = clSetKernelArg(kernel, 5, sizeof(ifct.s[3]), (void *)&ifct.s[3]);
		//if(ret)
		//	MessageBoxA(NULL, "clSetKernelArg", "Ошибка в строке:", MB_OK);
		
		size_t global_work_size[2] = { xPixels, yPixels };
		size_t local_work_size[2] = { 16, 16 };

		/* выполнить кернел */
		if(
			clEnqueueNDRangeKernel(
				command_queue, kernel, 2, NULL, global_work_size, local_work_size, 0, NULL, NULL)
			)
			MessageBoxA(NULL, "clEnqueueNDRangeKernel", "Ошибка в строке:", MB_OK);

		if(
			clFinish(command_queue)
			)
			MessageBoxA(NULL, "clFinish(command_queue)", "Ошибка в строке:", MB_OK);

		/* считать данные из буфера */
		if(
			clEnqueueReadBuffer(
				command_queue, coldev, CL_TRUE, 0, 
				xPixels * yPixels * sizeof(cl_uchar4), (void*)destcol, 0, NULL, NULL)
			)
			MessageBoxA(NULL, "clEnqueueReadBuffer", "Ошибка в строке:", MB_OK);
	
		clReleaseMemObject(coldev);
		clReleaseMemObject(devCom);

	}

	HRESULT FewFractalDraw()
	{
		int oldfps = fps;
		fps = clock();
		HRESULT hr = S_OK;
		int iter = I - 5*sqrnum;
		float step;
		float width = Width;
		float	// Centre of fractal
				x = center.x, 
				y = center.y; 
		float	// Size of fractal window
				xsize = X / width, 
				ysize = Y / width;

		//if (k>15)
		//{
		//	int f = rand()%31;
		//	while (B & (1<<f))
		//		f = rand()%31;
		//	Col = rand()%8;
		//	Frac1 = f;
		//	k = 0;
		//	B = B | (1<<f);
		//}
		//if (B >= 2147483647) // 31 единичкa
		//	B = 0;

		step = (1.0f/width);

		float4 posData;
		posData.s[0] = x - xsize;
		posData.s[1] = y - ysize;
		posData.s[2] = colorPower;
		posData.s[3] = step;

		lw = pSig->GetFFT();


		pRend->pRT->Clear(D2D1::ColorF(D2D1::ColorF::Black));
		int z = sqrnum;
		int w = pRend->Xscreen/z,
			h = pRend->Yscreen/z;
		int f = Frac1;
		LPPOINT p = new POINT;
		GetCursorPos(p);
		int ii = p->x/w, jj = p->y/h;
		for (int i = 0; i < z && i*z < numFrac; i++)
			for (int j = 0; j < z && i*z + j < numFrac; j++)
		{
			StartOpenCLFractal(posData, col, iter, lw);
			btm->CopyFromMemory(0, col, xPixels*sizeof(uchar4));
			pRend->pRT->DrawBitmap(btm, D2D1::RectF(i*w, j*h, (i+1)*w, (j+1)*h));
			Frac1++;
		}
			Frac1 = f;
		
		pRend->pRT->DrawRectangle(D2D1::RectF(ii*w, jj*h, (ii+1)*w, (jj+1)*h), pRend->pBrush, 3);
		mouseHoverFrac = Frac1 + ii*z + jj;

		pRend->SetColor(D2D1::ColorF(D2D1::ColorF::White));
		//pRend->DrawNumber(iter,10,10,3);
		pRend->DrawNumber(Frac1 + jj + ii*sqrnum,10,60,7);
		pRend->DrawNumber(Frac2*(2*twoFrac-1),10,110,7);
		pRend->DrawNumber(Type,10,160,6);
		delete p;
		delete[] lw;
		//pRend->stop = true;
		return hr;
	}

HRESULT OpenCLFractalDraw()
	{
		
		int oldfps = fps;
		fps = clock();
		HRESULT hr = S_OK;
		int iter = I;
		float step;
		float width = Width;
		float	// Centre of fractal
				x = center.x, 
				y = center.y; 
		float	// Size of fractal window
				xsize = X / width, 
				ysize = Y / width;

		//if (k > 30)
		//{
		//	int f = rand()%31;
		//	while (B & (1<<f))
		//		f = rand()%31;
		//	Col = rand()%8;
		//	Frac1 = f;
		//	k = 0;
		//	B = B | (1<<f);
		//}
		//if (B >= 2147483647) // 31 единичкa
		//	B = 0;

		step = (1.0f/width);

		float4 posData;
		posData.s[0] = x - xsize;
		posData.s[1] = y - ysize;
		posData.s[2] = colorPower;
		posData.s[3] = step;

		lw = pSig->GetLogFFT();
		//lw = FFT::Return(pSig->GetSignal(1024,1),1024);
		StartOpenCLFractal(posData, col, iter, lw);
		delete[] lw;

		btm->CopyFromMemory(0, col, xPixels*sizeof(uchar4));

		pRend->pRT->DrawBitmap(btm, D2D1::RectF(0.f, 0.f, pRend->Xscreen, pRend->Yscreen));

		return hr;
	}

float2 c1,c2,c3,c4;
float2 cs1,cs2,cs3,cs4;
HRESULT PointsFractalDraw()
	{
		HRESULT hr = S_OK;
		int iter = I;
		float step;
		float width = Width;
		float	// Centre of fractal
				x = center.x, 
				y = center.y; 
		float	// Size of fractal window
				xsize = X / width, 
				ysize = Y / width;

		int yy = pRend->Yscreen>>2;
		int xx = pRend->Xscreen - yy;

		step = (1.0f/width);

		float4 posData;
		posData.s[0] = x - xsize;
		posData.s[1] = y - ysize;
		posData.s[2] = colorPower;
		posData.s[3] = step;

		if (GetKeyState(VK_LBUTTON) < 0)
			if (control)
			{
				LPPOINT p = new POINT;
				GetCursorPos(p);
				if (p->x > xx)
				{				
					float cx = ((p->x - xx - (yy>>1))<<1)/(yy+0.0f);
					switch (mouseDownOn)
					{
					case 1:
						c1.x = cx;
						c1.y = -((p->y - yy + (yy>>1))<<1)/(yy+0.0f);
						break;
					case 2:
						c2.x = cx;
						c2.y = -((p->y - (yy<<1) + (yy>>1))<<1)/(yy+0.0f);
						break;
					case 3:
						c3.x = cx;
						c3.y = -((p->y - yy*3 + (yy>>1))<<1)/(yy+0.0f);
						break;
					case 4:
						c4.x = cx;
						c4.y = -((p->y - (yy<<2) + (yy>>1))<<1)/(yy+0.0f);
						break;
					}
				}
				delete p;
			}


		lw = pSig->GetFFT();
		if (control)
			StartControlOpenCLFractal(posData, col, iter);
		else
			StartOpenCLFractal(posData, col, iter, lw);
		delete[] lw;

		btm->CopyFromMemory(0, col, xPixels*sizeof(uchar4));

		pRend->SetColor(D2D1::ColorF(D2D1::ColorF::White));
		pRend->pRT->FillRectangle(D2D1::RectF(xx, 0.f,
			pRend->Xscreen, pRend->Yscreen), pRend->pBrush);
		
		pRend->SetColor(D2D1::ColorF(D2D1::ColorF::Silver));
		for(int y = 0; y < pRend->Yscreen; y += (yy>>3))
			pRend->DrawLine(xx, y, pRend->Xscreen, y);
		for(int x = xx; x < pRend->Xscreen; x += (yy>>3))
			pRend->DrawLine(x, 0.0f, x, pRend->Yscreen);
		
		pRend->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
		for(int y = 0; y < pRend->Yscreen; y += (yy>>1))
			pRend->DrawLine(xx, y, pRend->Xscreen, y, y%yy == 0? 5: 1);
		for(int x = xx; x < pRend->Xscreen; x += (yy>>1))
			pRend->DrawLine(x, 0.0f, x, pRend->Yscreen);
		
		pRend->SetColor(D2D1::ColorF(D2D1::ColorF::Red));
		cs1.x = (c1.x + 1) * (yy>>1) + xx;
		cs2.x = (c2.x + 1) * (yy>>1) + xx;
		cs3.x = (c3.x + 1) * (yy>>1) + xx;
		cs4.x = (c4.x + 1) * (yy>>1) + xx;
		cs1.y = yy - (c1.y + 1)*(yy>>1);
		cs2.y = (yy<<1) - (c2.y + 1)*(yy>>1);
		cs3.y = yy*3 - (c3.y + 1)*(yy>>1);
		cs4.y = (yy<<2) - (c4.y + 1)*(yy>>1);
		pRend->pRT->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(cs1.x, cs1.y),5,5),pRend->pBrush);
		pRend->pRT->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(cs2.x, cs2.y),5,5),pRend->pBrush);
		pRend->pRT->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(cs3.x, cs3.y),5,5),pRend->pBrush);
		pRend->pRT->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(cs4.x, cs4.y),5,5),pRend->pBrush);
		pRend->DrawLine(xx + (yy>>1), (yy) - (yy>>1), cs1.x, cs1.y, 0.5f);
		pRend->DrawLine(xx + (yy>>1), (yy<<1) - (yy>>1), cs2.x, cs2.y, 0.5f);
		pRend->DrawLine(xx + (yy>>1), (yy+yy+yy) - (yy>>1), cs3.x, cs3.y, 0.5f);
		pRend->DrawLine(xx + (yy>>1), (yy<<2) - (yy>>1), cs4.x, cs4.y, 0.5f);

		pRend->pRT->DrawBitmap(btm, D2D1::RectF(0.f, 0.f, xx, pRend->Yscreen));

		return hr;
	}



};