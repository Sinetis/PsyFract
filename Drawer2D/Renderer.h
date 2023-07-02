
#ifndef _RENDERER_H_
#define _RENDERER_H_

#include "stdafx.h"

#include "WavPlayer.h"
#include "Capture.h"
#include "Complex.h"
#include "Constants.h"

//#include "Geometrics.h"

class Geometric;
class Frequency;
class IFSystem;
class LSystem;
class TurtleFrac;
class OpenCLfractal;
class HLSL;

using namespace Const;

#define GetRandomDouble [](float max){return max*((rand() % 10000) * 100 + (rand() % 10000)) / 1010000.0;}
#define double float

// Класс, отвечающий за всё рисование и связанное с ним.
class Renderer
{
	Controller *pCtrl;
	bool isDrawing, isExit, isFN;
	int FracNum;
public:
	struct
	{
		Frequency* frac1;
		Geometric* frac2;
		LSystem* frac4;
		IFSystem* frac5;
		TurtleFrac* frac6;
		OpenCLfractal* frac7;
		HLSL* frac8;

	} fracs;

	IFractal* frac;
	ID2D1Factory* pF;
	ID2D1HwndRenderTarget* pRT;
	int Xscreen, Yscreen;

	void StartRender();
	void DrawFN();
	void Draw() // Параллельная функция
	{
		while(!isExit)
		{
			if (!stop)
			{
				isDrawing = true;
				pRT->BeginDraw();
				frac->Draw();
				if (isFN)
					DrawFN();
				pRT->EndDraw();
				isDrawing = false;
			}
			else
				Sleep(50);
		}
	}
	void StopRender()
	{
		while (isDrawing)
			Sleep(100);
		isExit = true;
		frac->Destroy();
	}
	void WaitDrawing()
	{
		while (isDrawing)
			Sleep(50);
	}
	void CatchMessage(WPARAM key)
	{
		switch(key)
		{
		case 'F':
			if (GetKeyState(VK_SHIFT) & 0x8000)
				isFN = isFN ? false : true;
			break;
		case VK_F1:
		case VK_F2:
		case VK_F3:
		case VK_F4:
		case VK_F5:
		case VK_F6:
		case VK_F7:
		case VK_F8:
		case VK_F9:
		case VK_F10:
		case VK_F11:
		case VK_F12:
			ResetK();
			SwitchFractal(key - VK_F1 + 1);
			break;
		case VK_TAB:
			stop = 1 - stop;
			break;
		}

		frac->CatchMessage(key);
	}
	void CreateFractals();
	void SwitchFractal(int num = 0);

public:
	ID2D1Factory* pD2DFactory;
	IDWriteFactory* pDWriteFactory;
	ID2D1HwndRenderTarget* pD2DRenderTarget;
	IDWriteTextFormat* pDTextFormat;
	//IDWriteTextFormat* pDPrevTextFormat;
	ID2D1SolidColorBrush* pBrush;

	int k;
	bool bFullScreen;

	HWND hWnd;

public:
	Renderer(Controller *pCont, HWND hwnd, int width, int height, bool fullscreen);
	~Renderer();

	void ResetK(){k=0;}
	void IncK(){k++;}
	void DecK(){k--;}

	bool stop;
	int mouseX, mouseY;

	void DiscardResources();

	D2D1_RECT_F Point(float x, float y, float width);
	Complex Mand(Complex z, Complex c, int iter);


	void RotationVector(D2D1_POINT_2F* c, D2D1_POINT_2F a, D2D1_POINT_2F b, float angleDeg);
	D2D1_POINT_2F AddPointFromVector(D2D1_POINT_2F a, D2D1_POINT_2F b, float ratio);
	void Line(int x1, int y1, int x2, int y2);
	void LineT(int x, int y, int width);
	void LineTL(int x, int y, int width);


	void DrawNumber(int number, int x, int y, int width);
	void DrawString(int x, int y, WCHAR* string);
	int SetTextFont(WCHAR* textname);
	int SetTextSize(float size);
	int SetTextStyle(DWRITE_FONT_STYLE style);
	int SetTextWeight(DWRITE_FONT_WEIGHT size);
	int SetTextStretch(DWRITE_FONT_STRETCH stretch);
	int SetTextLocale(WCHAR* localename);


	void SetColor(D2D1_COLOR_F col)
	{
		pBrush->SetColor(col);
	}
	void DrawLine(D2D1_POINT_2F a, D2D1_POINT_2F b, float width = 1.f)
	{
		pRT->DrawLine(a, b, pBrush, width);
	}
	void DrawLine(float x1, float y1, float x2, float y2, float width = 1.f)
	{
		pRT->DrawLine(D2D1::Point2F(x1, y1), D2D1::Point2F(x2, y2), pBrush, width);
	}
	void DrawPoint(float x, float y, float width)
	{
		pRT->FillRectangle(Point(x, y, width), pBrush);
	}
	void DrawPoint(float x, float y, float width, ID2D1SolidColorBrush* br)
	{
		pRT->FillRectangle(Point(x, y, width), br);
	}

private:
	HRESULT CreateDeviceIndependentResources();
	HRESULT CreateDeviceResources();
};


struct ForSave
{
		D2D1_POINT_2F sa;
		float sangle;
		D2D1_COLOR_F scolor;
		float swidth;
};

class Turtle
	{
	private:
		D2D1_POINT_2F a, t;
		float angle;
		D2D1_COLOR_F color;
		float width;
		
		// For save
		std::vector<ForSave> save;

		Renderer* pRend;

	public:
		Turtle(Renderer* rend)
		{
			a = D2D1::Point2F( 0.0F, 0.0F );
			t = a;
			angle = 0.0F;
			width = 1.0F;
			pRend = rend;
		}
		void Forward (float len)
		{
			t.x = a.x + len * cos(PI_180*angle);
			t.y = a.y + len * sin(-angle*PI_180);
			pRend->DrawLine(a, t, width);
			a = t;
		}
		void Jump (float len)
		{
			t.x = a.x + len * cos(PI_180*angle);
			t.y = a.y + len * sin(-angle*PI_180);
			//pRend->DrawLine(a, t, width);
			a = t;
		}
		void Rotate (float ang)
		{
			angle += ang;
		}
		void LineWidth (float wid)
		{
			width = wid;
		}
		void Color (D2D1_COLOR_F col)
		{
			color = col;
			pRend->SetColor(color);
		}
		void Go (float x, float y)
		{
			a.x = x;
			a.y = y;
		}
		void Direction (float ang)
		{
			angle = ang;
		}
		void Save()
		{
			ForSave fs;
			fs.sa = a;
			fs.swidth = width;
			fs.scolor = color;
			fs.sangle = angle;

			save.push_back(fs);
		}
		void Restore()
		{
			ForSave fs = save.back();
			a = fs.sa;
			width = fs.swidth;
			color = fs.scolor;
			angle = fs.sangle; 

			save.pop_back();
		}
		void Func(float c, int iter); 

		LPCWSTR AXIOM;

		void SetAxiom(LPCWSTR A)
		{
			AXIOM = A;
		}
		void New(std::pair<WCHAR, LPCWSTR>* New, int num)
		{
			std::wstring T;
			int lengthA = lstrlenW(AXIOM);
			for (int j, i = 0; i < lengthA; i++)
			{
				for (j = 0; j < num; j++)
					if (New[j].first == AXIOM[i])
					{
						T += New[j].second;
						break;
					}
				if (j == num)
					T += AXIOM[i];
			}
			if (AXIOM != NULL)
				delete []AXIOM;
			WCHAR* temp = new WCHAR[T.length()];
			for (int i = 0; i < (T.length()); i++)
				temp[i] = T[i];
			AXIOM = temp;


		}

		void Interpretation(float angle, float step)
		{
			int lengthA = lstrlenW(AXIOM);
			for (int i = 0; i < lengthA; i++)
				switch (AXIOM[i])
				{
				case '+':
					this->Rotate(angle);
					break;
				case '-':
					this->Rotate(-angle);
					break;

				case '=':
					this->Rotate(GetRandomDouble(angle));
					break;
				case '_':
					this->Rotate(-GetRandomDouble(angle));
					break;

				case 'F':
					this->Forward(step);
					break;
				
				case 'f':
					this->Forward(GetRandomDouble(step));
					break;

				case 'b':
					this->Jump(step);
					break;

				case '[':
					this->Save();
					break;
				case ']':
					this->Restore();
					break;
				}
		}
		
	};


template<class Interface>
inline void SafeRelease(
    Interface **pPInterfaceToRelease
    )
{
    if (*pPInterfaceToRelease != NULL)
    {
        (*pPInterfaceToRelease)->Release();

        (*pPInterfaceToRelease) = NULL;
    }
}

//inline void FreeMem(int* mem)
//{
//	if (mem != NULL)
//		delete []mem;
//	mem = NULL;
//}



template <class T>
inline void FreeMem(T** pmem)
{
	if (*pmem != NULL)
		delete [](*pmem);
	*pmem = NULL;
}
//template <class T>
//inline void FreeMem(T* mem)
//{
//	if (mem != NULL)
//		delete []mem;
//	mem = NULL;
//}

#endif