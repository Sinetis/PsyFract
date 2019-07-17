#include "stdafx.h"
#include "Renderer.h"

#include "Geometric.h"
#include "Frequency.h"
#include "IFSystem.h"
#include "LSystem.h"
#include "TurtleFrac.h"
#include "OpenCLfractal.h"
#include "HLSL.h"

#define double float

Renderer::Renderer(Controller* c,HWND hwnd, int width, int height, bool fullscreen)
{

	Xscreen = GetSystemMetrics(SM_CXSCREEN);
	Yscreen = GetSystemMetrics(SM_CYSCREEN);
	pCtrl = c;
	isExit = false;
	isDrawing = false;
	isFN = false;
	stop = TRUE;
	this->hWnd = hwnd;
	this->pD2DFactory = NULL;
	this->pD2DRenderTarget = NULL;
	this->CreateDeviceIndependentResources();
	ResetK();
	bFullScreen = fullscreen;
	mouseX = 0;
	mouseY = 0;
	
	stop = FALSE;
	pRT = pD2DRenderTarget;
	pF = pD2DFactory;
}

void Renderer::CreateFractals()
{
	fracs.frac1 = new Frequency(pCtrl);
	fracs.frac2 = new Geometric(pCtrl);
	fracs.frac4 = new LSystem(pCtrl);
	fracs.frac5 = new IFSystem(pCtrl);
	fracs.frac6 = new TurtleFrac(pCtrl);
	fracs.frac7 = new OpenCLfractal(pCtrl);
	fracs.frac8 = new HLSL(pCtrl);

	frac = fracs.frac7;
	FracNum = 7;
	frac->Initialize();
}

void Renderer::DrawFN()
{
	D2D1_ROUNDED_RECT rect;
	rect.rect = D2D1::RectF(0,0, 100,50);
	rect.radiusX = rect.radiusY = 10;
	int i;
	SetColor(D2D1::ColorF(1,1,1));
	for (i = 0; i < 7; i++)
	{
		rect.rect.left = i*100;
		rect.rect.right = 100 + rect.rect.left;
		pRT->FillRoundedRectangle(&rect, pBrush);
	}
	SetColor(D2D1::ColorF(0,0,0));
	for (i = 0; i < 7; i++)
	{
		rect.rect.left = i*100;
		rect.rect.right = 100 + rect.rect.left;
		pRT->DrawRoundedRectangle(&rect, pBrush);
	}
	
	this->DrawString(10, 8, L"F1");
	this->DrawString(10, 25, L"Frequency");
	this->DrawString(110, 8, L"F2");
	this->DrawString(110, 25, L"Geometric");

	this->DrawString(310, 8, L"F4");
	this->DrawString(310, 25, L"LSystem");
	this->DrawString(410, 8, L"F5");
	this->DrawString(410, 25, L"IFSystem");
	this->DrawString(510, 8, L"F6");
	this->DrawString(510, 25, L"TurtleFrac");
	this->DrawString(610, 8, L"F7");
	this->DrawString(610, 25, L"OpenCLfractal");
}

void Renderer::SwitchFractal(int num)
{
	if (FracNum == num)
		return;
	else
		FracNum = num;
	stop = true;
	WaitDrawing();
	IFractal* f = frac;
	switch(num)
		{
		case 1:
			frac = (IFractal*)fracs.frac1;
			break;
		case 2:           
			frac = (IFractal*)fracs.frac2;
			break;
		case 4:
			frac = (IFractal*)fracs.frac4;
			break;
		case 5:
			frac = (IFractal*)fracs.frac5;
			break;
		case 6:
			frac = (IFractal*)fracs.frac6;
			break;
		case 7:
			frac = (IFractal*)fracs.frac7;
			break;
		case 8:
			frac = (IFractal*)fracs.frac8;
			break;
		default:
			frac = (IFractal*)fracs.frac1;
			break;
		}
	if (FAILED(frac->Initialize()))
	{
		StopRender();
		MessageBox(hWnd, L"Что-то плохо", L"ERROR", MB_OK);
	}
	f->Destroy();
	stop = false;
}

void Renderer::StartRender()
{
	std::thread t([&]{this->Draw();});
	t.detach();
}

Renderer::~Renderer()
{
	this->DiscardResources();
}

inline HRESULT Renderer::CreateDeviceIndependentResources()
{
	HRESULT hr =
		D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &pD2DFactory);
	if (!SUCCEEDED(hr))
		throw L"Не удалось создать фабрику рисования";

	hr = 
		DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(&pDWriteFactory));
	if (!SUCCEEDED(hr))
		throw L"Не удалось создать фабрику текста";

	hr = CreateDeviceResources();
	if (!SUCCEEDED(hr))
		throw L"Не удалось создать ресурсы устройств";
	return hr;
}

inline HRESULT Renderer::CreateDeviceResources()
{
	HRESULT hr = S_OK;
	srand(time(0));
	if (!this->pD2DRenderTarget)
	{
		if (bFullScreen)
		{
			//SetWindowPos(hWnd, HWND_TOPMOST, 0, 0,
			//	GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 
			//	SWP_NOMOVE | SWP::NOACTIVATE | SWP_NOREPOSITION);
		}
		RECT rc;
		GetClientRect(hWnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
		hr = pD2DFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(hWnd, size),
			&pD2DRenderTarget);

		if (SUCCEEDED(hr))
		{
			hr = pD2DRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::Red),
				&pBrush);
		}
		if (SUCCEEDED(hr))
		{
			hr = pDWriteFactory->CreateTextFormat(
				L"Arial",
				NULL,
				DWRITE_FONT_WEIGHT_NORMAL,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				10.0f * 96.0f/72.0f, 
				L"en-US",
				&pDTextFormat
				);
		}

	}
	return hr;
}

void Renderer::DiscardResources()
{
	SafeRelease(&pBrush);
	SafeRelease(&pD2DRenderTarget);
	SafeRelease(&pD2DFactory);
	SafeRelease(&pDTextFormat);
	SafeRelease(&pDWriteFactory);
}

void Renderer::RotationVector(	D2D1_POINT_2F* c, 
								D2D1_POINT_2F a, 
								D2D1_POINT_2F b, 
								float angleDeg)
{
	float S, C;
	S = sin(PI_180 * angleDeg);
	C = cos(PI_180 * angleDeg);
	c->x = b.x - a.x;
	c->y = b.y - a.y;
	c->x = c->x*C + c->y*S;
	c->y = c->y*C - (b.x - a.x)*S;
	c->x += a.x;
	c->y += a.y;
}

void Renderer::Line(int x1, int y1, int x2, int y2)
{
	D2D1_POINT_2F a, b;
	a.x = x1;
	a.y = y1;
	b.x = x2;
	b.y = y2;
	pRT->DrawLine(a, b, pBrush);
}

void Renderer::DrawNumber(int number, int x, int y, int width)
{
	if (pDTextFormat == NULL)
		return;
	//float prev = pDTextFormat->GetFontSize();
	//if (SetTextSize(width*4)) MessageBox(NULL,L"Что-то пошло не так1.",L"ERROR",MB_OK);
	WCHAR* buf = new WCHAR[80];
	swprintf(buf, L"%d", number);
	DrawString(x,y, buf);
	//if (SetTextSize(prev)) MessageBox(NULL,L"Что-то пошло не так2.",L"ERROR",MB_OK);
	delete[] buf;
	return;
}

void Renderer::LineT(int x, int y, int width)
{Line(x, y, x + width, y);}
void Renderer::LineTL(int x, int y, int width)
{Line(x, y, x, y + width);}

D2D1_RECT_F Renderer::Point(float x, float y, float width)
{
	return D2D1::RectF(x, y, x+width, y+width);
}

D2D1_POINT_2F Renderer::AddPointFromVector(D2D1_POINT_2F a, D2D1_POINT_2F b, float ratio = 1.0F)
{
	D2D1_POINT_2F c;
	c.x = b.x + (b.x - a.x) * ratio;
	c.y = b.y + (b.y - a.y) * ratio;
	return c;
}

void Renderer::DrawString(int x, int y, WCHAR* string)
{
	D2D1_RECT_F layoutRect = D2D1::RectF(x, y, Xscreen/2 + x, Yscreen);

	pRT->DrawText(
            string,
			wcslen(string),
            pDTextFormat,
            layoutRect, 
            pBrush
        );
}

int Renderer::SetTextFont(WCHAR* fontname = NULL)
{
	if (fontname == NULL) fontname = L"Arial";
	
	WCHAR localename[LOCALE_NAME_MAX_LENGTH];
	pDTextFormat->GetLocaleName(&localename[0], pDTextFormat->GetLocaleNameLength()+1);

	IDWriteTextFormat *p = pDTextFormat;

	int r = pDWriteFactory->CreateTextFormat(
		fontname,
		NULL,
		pDTextFormat->GetFontWeight(),
		pDTextFormat->GetFontStyle(),
		pDTextFormat->GetFontStretch(),
		pDTextFormat->GetFontSize(),
		&localename[0],
		&pDTextFormat);
	SafeRelease(&p);
	return r;
	
}
int Renderer::SetTextSize(float size)
{
	if ( size == pDTextFormat->GetFontSize())
		return 0;

	WCHAR localename[LOCALE_NAME_MAX_LENGTH], fontname[40];
	pDTextFormat->GetLocaleName(&localename[0], pDTextFormat->GetLocaleNameLength()+1);
	pDTextFormat->GetFontFamilyName(&fontname[0], pDTextFormat->GetFontFamilyNameLength()+1);

	IDWriteTextFormat *p = pDTextFormat;

	int r = pDWriteFactory->CreateTextFormat(
		&fontname[0],
		NULL,
		pDTextFormat->GetFontWeight(),
		pDTextFormat->GetFontStyle(),
		pDTextFormat->GetFontStretch(),
		size,
		&localename[0],
		&pDTextFormat);
	SafeRelease(&p);
	return r;
}
int Renderer::SetTextStyle(DWRITE_FONT_STYLE style)
{
	WCHAR localename[LOCALE_NAME_MAX_LENGTH], fontname[40];
	pDTextFormat->GetLocaleName(&localename[0], pDTextFormat->GetLocaleNameLength()+1);
	pDTextFormat->GetFontFamilyName(&fontname[0], pDTextFormat->GetFontFamilyNameLength()+1);

	IDWriteTextFormat *p = pDTextFormat;

	int r = pDWriteFactory->CreateTextFormat(
		&fontname[0],
		NULL,
		pDTextFormat->GetFontWeight(),
		style,
		pDTextFormat->GetFontStretch(),
		pDTextFormat->GetFontSize(),
		&localename[0],
		&pDTextFormat);
	SafeRelease(&p);
	return r;
}
int Renderer::SetTextWeight(DWRITE_FONT_WEIGHT weight)
{
	WCHAR localename[LOCALE_NAME_MAX_LENGTH], fontname[40];
	pDTextFormat->GetLocaleName(&localename[0], pDTextFormat->GetLocaleNameLength()+1);
	pDTextFormat->GetFontFamilyName(&fontname[0], pDTextFormat->GetFontFamilyNameLength()+1);

	IDWriteTextFormat *p = pDTextFormat;

	int r = pDWriteFactory->CreateTextFormat(
		fontname,
		NULL,
		weight,
		pDTextFormat->GetFontStyle(),
		pDTextFormat->GetFontStretch(),
		pDTextFormat->GetFontSize(),
		localename,
		&pDTextFormat);
	SafeRelease(&p);
	return r;
}
int Renderer::SetTextStretch(DWRITE_FONT_STRETCH stretch)
{
	WCHAR localename[LOCALE_NAME_MAX_LENGTH], fontname[40];
	pDTextFormat->GetLocaleName(&localename[0], pDTextFormat->GetLocaleNameLength()+1);
	pDTextFormat->GetFontFamilyName(&fontname[0], pDTextFormat->GetFontFamilyNameLength()+1);

	IDWriteTextFormat *p = pDTextFormat;

	int r = pDWriteFactory->CreateTextFormat(
		fontname,
		NULL,
		pDTextFormat->GetFontWeight(),
		pDTextFormat->GetFontStyle(),
		stretch,
		pDTextFormat->GetFontSize(),
		localename,
		&pDTextFormat);
	SafeRelease(&p);
	return r;
}
int Renderer::SetTextLocale(WCHAR* localename = NULL)
{
	WCHAR *fontname = new WCHAR[40];
	pDTextFormat->GetFontFamilyName(fontname, pDTextFormat->GetFontFamilyNameLength()+1);
	if (localename == NULL) localename = L"en-us";

	IDWriteTextFormat *p = pDTextFormat;

	int r = pDWriteFactory->CreateTextFormat(
		fontname,
		NULL,
		pDTextFormat->GetFontWeight(),
		pDTextFormat->GetFontStyle(),
		pDTextFormat->GetFontStretch(),
		pDTextFormat->GetFontSize(),
		localename,
		&pDTextFormat);
	SafeRelease(&p);
	return r;
}