#pragma once

#include "Renderer.h"
#include "Interfaces.h"
#include "resource.h"
#include <stdio.h>
#include <fstream>
#include <string>
#include <iostream>

#include <d3d10.h>
#include <d3dx10.h>
#include "DXObjects.h"

using namespace Const;

#define MAX_SOURCE_SIZE 16777216
#define MAX_FRACS 32


class HLSL : public IFractal
{
	LPSTR HLSLString;

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

	D3D10_DRIVER_TYPE		g_driverType = D3D10_DRIVER_TYPE_NULL;
	ID3D10Device*			g_pd3dDevice = NULL;
	IDXGISwapChain*			g_pSwapChain = NULL;
	ID3D10RenderTargetView*	g_pRenderTargetView = NULL;

	ID3D10Effect*			g_pEffect = NULL;
	ID3D10EffectTechnique*	g_pTehnigue = NULL;
	ID3D10InputLayout*		g_pVertexLayout = NULL;
	ID3D10Buffer*			g_pVertexBuffer = NULL;

	ID3D10Buffer*			g_pIndexBuffer = NULL;

	D3DXMATRIX					g_World;
	D3DXMATRIX					g_View;
	D3DXMATRIX					g_Projection;
	ID3D10EffectMatrixVariable* g_pWorldVariable = NULL;
	ID3D10EffectMatrixVariable* g_pViewVariable = NULL;
	ID3D10EffectMatrixVariable* g_pProjectionVariable = NULL;

	ID3D10EffectVectorVariable* g_pLightColor = NULL;
	ID3D10EffectVectorVariable* g_pLightDirection = NULL;
	ID3D10EffectVectorVariable* g_pLightPosition = NULL;

	ID3D10EffectScalarVariable* g_pSpotLightPhi = NULL;
	ID3D10EffectScalarVariable* g_pSpotLightTheta = NULL;

	ID3D10Texture2D*		g_pDepthStencil = NULL;
	ID3D10DepthStencilView* g_pDepthStencilView = NULL;

public:
	HLSL(Controller* c)
	{
		SetBehavior(c);
		InitVariables();

		xPixels = X * 2;
		yPixels = Y * 2;
		if (xPixels % 16 != 0)
			xPixels += 16 - xPixels % 16;
		if (yPixels % 16 != 0)
			yPixels += 16 - yPixels % 16;

		HLSLString = ("shader.fx");

		//HLSLString = ("OpenCLfunction.cl");
		LPSTR str = GetCommandLineA();
		//MessageBoxA(0,str,"-", MB_OK);
		if (str != NULL)
		{
			int i, l = strlen(str);
			//LPSTR cmp(".cl");
			//for (i = 0; i < 3 && i < l; i++)
			//	if (str[l-4 + i] != cmp[i])
			//		break;
			if (str[l - 4] == '.' && str[l - 3] == 'c' && str[l - 2] == 'l')
			{
				for (i = l - 3; str[i] != '\"'; i--);
				//delete OpenCLString;
				HLSLString = new char[l - i - 1];
				for (int j = i + 1; j < l - 1; j++)
					HLSLString[j - i - 1] = str[j];
				HLSLString[l - i - 2] = '\0';
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
		control = false;
		mouseHoverFrac = mouseDownOn = 0;
		srand(time(0));
		lw = 0;
		center.x = 0;
		center.y = 0;
		channel = 0;
		Frac1 = Frac2 = Col = Type = flow = 0;
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
		X = GetSystemMetrics(SM_CXSCREEN) / 2,
			Y = GetSystemMetrics(SM_CYSCREEN) / 2;
		Width = X / 5.0f;
	}

	int Initialize()
	{
		HRESULT hr = S_OK;

		UINT width = pRend->Xscreen;
		UINT height = pRend->Yscreen;

		D3D10_DRIVER_TYPE driverTypes[] =
		{
			D3D10_DRIVER_TYPE_HARDWARE,
			D3D10_DRIVER_TYPE_REFERENCE,
		};
		UINT numDriverTypes =
			sizeof(driverTypes) / sizeof(driverTypes[0]);

		// Заполняем текстуру
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = pRend->hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = FALSE;

		// Пытаемся создать устройство, проходя по списку
		// Как только получилось - выходим из цикла
		for (UINT driverTypeIndex = 0;
			driverTypeIndex < numDriverTypes; driverTypeIndex++)
		{
			g_driverType = driverTypes[driverTypeIndex];
			hr = D3D10CreateDeviceAndSwapChain(
				NULL, g_driverType, NULL, 0, D3D10_SDK_VERSION,
				&sd, &g_pSwapChain, &g_pd3dDevice);
			if (SUCCEEDED(hr))
				break;
		}

		if (FAILED(hr))
			return hr;

		// Представление данных для буффера визуализации
		ID3D10Texture2D *pBackBuffer;
		hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (LPVOID*)&pBackBuffer);
		if (FAILED(hr))
			return hr;

		hr = g_pd3dDevice->CreateRenderTargetView(
			pBackBuffer, NULL, &g_pRenderTargetView);
		pBackBuffer->Release();
		if (FAILED(hr))
			return hr;

		// Настроим область отображения
		D3D10_VIEWPORT vp;
		vp.Width = width;
		vp.Height = height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		g_pd3dDevice->RSSetViewports(1, &vp);

		D3D10_TEXTURE2D_DESC descDepth;
		descDepth.Width = width;
		descDepth.Height = height;
		descDepth.ArraySize = 1;
		descDepth.MipLevels = 1;
		descDepth.Format = DXGI_FORMAT_D32_FLOAT;
		descDepth.SampleDesc.Count = 1;
		descDepth.SampleDesc.Quality = 0;
		descDepth.Usage = D3D10_USAGE_DEFAULT;
		descDepth.BindFlags = D3D10_BIND_DEPTH_STENCIL;
		descDepth.CPUAccessFlags = 0;
		descDepth.MiscFlags = 0;

		hr = g_pd3dDevice->CreateTexture2D(&descDepth, NULL, &g_pDepthStencil);
		if (FAILED(hr))
			return hr;

		D3D10_DEPTH_STENCIL_VIEW_DESC descDSV;
		descDSV.Format = descDepth.Format;
		descDSV.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;

		hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, &descDSV, &g_pDepthStencilView);
		if (FAILED(hr))
			return hr;

		// Свяжем буффер визуализации с графическим конвейером
		g_pd3dDevice->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);

		ID3D10Blob *g_pBlob;
		HRESULT r;
		// Создадим эффект
		hr = D3DX10CreateEffectFromFile(L"geometric.fx", NULL, NULL, "fx_4_0",
			D3D10_SHADER_ENABLE_STRICTNESS, 0, g_pd3dDevice, NULL, NULL,
			&g_pEffect, &g_pBlob, &r);
		if (FAILED(hr))
		{
			LPCSTR str = (LPCSTR)g_pBlob->GetBufferPointer();
			Destroy();
			MessageBoxA(pRend->hWnd, str, "ERROR", MB_OK);
			exit(0);
			return hr;
		}

		// Извлекаем технику отображения
		g_pTehnigue = g_pEffect->GetTechniqueByName("RenderGeometric");

		g_pWorldVariable = g_pEffect->GetVariableByName("World")->AsMatrix();
		g_pViewVariable = g_pEffect->GetVariableByName("View")->AsMatrix();
		g_pProjectionVariable = g_pEffect->GetVariableByName("Projection")->AsMatrix();

		g_pLightColor = g_pEffect->GetVariableByName("LightColor")->AsVector();
		g_pLightDirection = g_pEffect->GetVariableByName("LightDirection")->AsVector();
		g_pLightPosition = g_pEffect->GetVariableByName("LightPosition")->AsVector();

		g_pSpotLightPhi = g_pEffect->GetVariableByName("Phi")->AsScalar();
		g_pSpotLightTheta = g_pEffect->GetVariableByName("Theta")->AsScalar();

		// Описываем формат входных данных
		D3D10_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
			D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
		D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24,
		D3D10_INPUT_PER_VERTEX_DATA, 0 }
		};
		UINT numElements = sizeof(layout) / sizeof(layout[0]);

		// Создаём объект входных данных
		D3D10_PASS_DESC passDesc;
		g_pTehnigue->GetPassByIndex(0)->GetDesc(&passDesc);

		hr = g_pd3dDevice->CreateInputLayout(layout, numElements,
			passDesc.pIAInputSignature, passDesc.IAInputSignatureSize,
			&g_pVertexLayout);
		if (FAILED(hr))
			return hr;

		// Связываем объект входных данных с графическим конвейером
		g_pd3dDevice->IASetInputLayout(g_pVertexLayout);

		// Создаём буффер вершин
		D3DXVECTOR3 vertices[] =
		{
			D3DXVECTOR3(-1, 1, -1), D3DXVECTOR3(0, 1, 0), D3DXVECTOR4(1,0,0,1),
			D3DXVECTOR3(1, 1, -1), D3DXVECTOR3(0, 1, 0), D3DXVECTOR4(1,0,0,1),
			D3DXVECTOR3(1, 1,  1), D3DXVECTOR3(0, 1, 0), D3DXVECTOR4(1,0,0,1),
			D3DXVECTOR3(-1, 1,  1), D3DXVECTOR3(0, 1, 0), D3DXVECTOR4(1,0,0,1),

			D3DXVECTOR3(-1, -1, -1), D3DXVECTOR3(0, -1, 0), D3DXVECTOR4(0,1,0,1),
			D3DXVECTOR3(1, -1, -1), D3DXVECTOR3(0, -1, 0), D3DXVECTOR4(0,1,0,1),
			D3DXVECTOR3(1, -1,  1), D3DXVECTOR3(0, -1, 0), D3DXVECTOR4(0,1,0,1),
			D3DXVECTOR3(-1, -1,  1), D3DXVECTOR3(0, -1, 0), D3DXVECTOR4(0,1,0,1),

			D3DXVECTOR3(-1, -1,  1), D3DXVECTOR3(-1, 0, 0), D3DXVECTOR4(0,0,1,1),
			D3DXVECTOR3(-1, -1, -1), D3DXVECTOR3(-1, 0, 0), D3DXVECTOR4(0,0,1,1),
			D3DXVECTOR3(-1,  1, -1), D3DXVECTOR3(-1, 0, 0), D3DXVECTOR4(0,0,1,1),
			D3DXVECTOR3(-1,  1,  1), D3DXVECTOR3(-1, 0, 0), D3DXVECTOR4(0,0,1,1),

			D3DXVECTOR3(1, -1,  1), D3DXVECTOR3(1, 0, 0), D3DXVECTOR4(1,1,0,1),
			D3DXVECTOR3(1, -1, -1), D3DXVECTOR3(1, 0, 0), D3DXVECTOR4(1,1,0,1),
			D3DXVECTOR3(1,  1, -1), D3DXVECTOR3(1, 0, 0), D3DXVECTOR4(1,1,0,1),
			D3DXVECTOR3(1,  1,  1), D3DXVECTOR3(1, 0, 0), D3DXVECTOR4(1,1,0,1),

			D3DXVECTOR3(-1, -1, -1), D3DXVECTOR3(0, 0, -1), D3DXVECTOR4(1,0,1,1),
			D3DXVECTOR3(1, -1, -1), D3DXVECTOR3(0, 0, -1), D3DXVECTOR4(1,0,1,1),
			D3DXVECTOR3(1,  1, -1), D3DXVECTOR3(0, 0, -1), D3DXVECTOR4(1,0,1,1),
			D3DXVECTOR3(-1,  1, -1), D3DXVECTOR3(0, 0, -1), D3DXVECTOR4(1,0,1,1),

			D3DXVECTOR3(-1, -1,  1), D3DXVECTOR3(0, 0, 1), D3DXVECTOR4(0,1,1,1),
			D3DXVECTOR3(1, -1,  1), D3DXVECTOR3(0, 0, 1), D3DXVECTOR4(0,1,1,1),
			D3DXVECTOR3(1,  1,  1), D3DXVECTOR3(0, 0, 1), D3DXVECTOR4(0,1,1,1),
			D3DXVECTOR3(-1,  1,  1), D3DXVECTOR3(0, 0, 1), D3DXVECTOR4(0,1,1,1),

		};
		D3D10_BUFFER_DESC bd;
		bd.Usage = D3D10_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(vertices);
		bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		D3D10_SUBRESOURCE_DATA initData;
		initData.pSysMem = vertices;
		hr = g_pd3dDevice->CreateBuffer(&bd, &initData, &g_pVertexBuffer);
		if (FAILED(hr))
			return hr;

		UINT stride = sizeof(SimpleVertex);
		UINT offset = 0;
		// Связыываем буфер вершин с графическим конвейером
		g_pd3dDevice->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

		DWORD indices[] =
		{
			3,1,0, 2,1,3,
			6,4,5, 7,4,6,
			11,9,8, 10,9,11,
			14,12,13, 15,12,14,
			19,17,16, 18,17,19,
			22,20,21, 23,20,22
		};
		bd.ByteWidth = sizeof(indices);
		bd.BindFlags = D3D10_BIND_INDEX_BUFFER;
		initData.pSysMem = indices;
		hr = g_pd3dDevice->CreateBuffer(&bd, &initData, &g_pIndexBuffer);
		if (FAILED(hr))
			return hr;

		g_pd3dDevice->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		g_pd3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


		CreateMatrix();

		return S_OK;
	}

	void CreateMatrix()
	{
		D3DXMatrixIdentity(&g_World);

		D3DXVECTOR3 Eye(0, 2, -5);
		D3DXVECTOR3 At(0, 0, 0);
		D3DXVECTOR3 Up(0, 1, 0);
		D3DXMatrixLookAtLH(&g_View, &Eye, &At, &Up);

		D3DXMatrixPerspectiveFovLH(
			&g_Projection, (float)D3DX_PI / 4,
			pRend->Xscreen / (float)pRend->Yscreen, 0.1f, 100.0f);
	}

	void RenderScene()
	{
		static POINT cursor = { -666,0 };
		static POINT cursorPrev;
		cursorPrev = cursor;
		GetCursorPos((LPPOINT)&cursor);
		static float shiftX, shiftY;
		if (cursorPrev.x == -666)
		{
			cursorPrev = cursor;
		}
		shiftX = (cursor.x - cursorPrev.x) / 100.0f;
		shiftY = (cursor.y - cursorPrev.y) / 100.0f;

		static float x = 0, y = 0;
		if (GetKeyState(VK_LBUTTON) < 0)
		{
			x += shiftX;
			y -= shiftY;
		}

		static float t = 0;
		if (g_driverType == D3D10_DRIVER_TYPE_REFERENCE)
		{
			t += (float)D3DX_PI * 0.0125;
		}
		else
		{
			static DWORD dwTimeStart = 0;
			DWORD dwTimeCur = GetTickCount();
			if (dwTimeStart == 0)
				dwTimeStart = dwTimeCur;
			t = (dwTimeCur - dwTimeStart) / 1000.0f;
		}
		D3DXMatrixRotationY(&g_World, t);

		g_pWorldVariable->SetMatrix((float*)&g_World);
		g_pViewVariable->SetMatrix((float*)&g_View);
		g_pProjectionVariable->SetMatrix((float*)&g_Projection);

		float lightColor[4] = { 1, 0.3f, 0.3f, 1 };
		g_pLightColor->SetFloatVector((float*)&lightColor);

		float sx = sinf(y), sy = sinf(x), cx = cos(y), cy = cos(x);
		float lightDir[4] = { 0, 0, 0.0f, 1 };
		g_pLightDirection->SetFloatVector((float*)&lightDir);

		//float lightPos[4] = { x, y, -1.5f, 1 };
		float lightPos[4] = { 2 * sx*cy, 2 * cx, 2 * sx*sy, 1 };
		g_pLightPosition->SetFloatVector((float*)&lightPos);

		float clearColor[4] = { 0.1f, 0.5f, (sinf(k * 10) + 1.0f) / 2, 1.0f };
		g_pd3dDevice->ClearRenderTargetView(g_pRenderTargetView, clearColor);
		g_pd3dDevice->ClearDepthStencilView(g_pDepthStencilView,
			D3D10_CLEAR_DEPTH | D3D10_CLEAR_STENCIL, 1, 0);

		g_pSpotLightPhi->SetFloat(D3DXToRadian(50));
		g_pSpotLightTheta->SetFloat(D3DXToRadian(25));

		//Рисуем наш треугольник
		D3D10_TECHNIQUE_DESC techDesc;
		g_pTehnigue->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			g_pTehnigue->GetPassByIndex(p)->Apply(0);
			g_pd3dDevice->DrawIndexed(36, 0, 0);
		}
		// Переключаем буферы
		g_pSwapChain->Present(0, 0);
	}

	int Initialize1()
	{
		HRESULT hr = S_OK;

		UINT width = pRend->Xscreen;
		UINT height = pRend->Yscreen;

		D3D10_DRIVER_TYPE driverTypes[] =
		{
			D3D10_DRIVER_TYPE_HARDWARE,
			D3D10_DRIVER_TYPE_REFERENCE,
		};
		UINT numDriverTypes =
			sizeof(driverTypes) / sizeof(driverTypes[0]);

		// Заполняем текстуру
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = pRend->hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = FALSE;

		// Пытаемся создать устройство, проходя по списку
		// Как только получилось - выходим из цикла
		for (UINT driverTypeIndex = 0;
			driverTypeIndex < numDriverTypes; driverTypeIndex++)
		{
			g_driverType = driverTypes[driverTypeIndex];
			hr = D3D10CreateDeviceAndSwapChain(
				NULL, g_driverType, NULL, 0, D3D10_SDK_VERSION,
				&sd, &g_pSwapChain, &g_pd3dDevice);
			if (SUCCEEDED(hr))
				break;
		}

		if (FAILED(hr))
			return hr;

		// Представление данных для буффера визуализации
		ID3D10Texture2D *pBackBuffer;
		hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (LPVOID*)&pBackBuffer);
		if (FAILED(hr))
			return hr;

		hr = g_pd3dDevice->CreateRenderTargetView(
			pBackBuffer, NULL, &g_pRenderTargetView);
		pBackBuffer->Release();
		if (FAILED(hr))
			return hr;

		// Свяжем буффер визуализации с графическим конвейером
		g_pd3dDevice->OMSetRenderTargets(1, &g_pRenderTargetView, NULL);
		// Настроим область отображения
		D3D10_VIEWPORT vp;
		vp.Width = width;
		vp.Height = height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		g_pd3dDevice->RSSetViewports(1, &vp);

		ID3D10Blob *g_pBlob;
		HRESULT r;

		// Создадим эффект
		hr = D3DX10CreateEffectFromFile(L"PointLight.fx", NULL, NULL, "fx_4_0",
			D3D10_SHADER_ENABLE_STRICTNESS, 0, g_pd3dDevice, NULL, NULL,
			&g_pEffect, &g_pBlob, &r);
		if (FAILED(hr))
		{
			LPCSTR str = (LPCSTR)g_pBlob->GetBufferPointer();
			Destroy();
			MessageBoxA(pRend->hWnd, str, "ERROR", MB_OK);
			exit(0);
			return hr;
		}

		// Извлекаем технику отображения
		g_pTehnigue = g_pEffect->GetTechniqueByName("RenderSpotLight");

		g_pWorldVariable = g_pEffect->GetVariableByName("World")->AsMatrix();
		g_pViewVariable = g_pEffect->GetVariableByName("View")->AsMatrix();
		g_pProjectionVariable = g_pEffect->GetVariableByName("Projection")->AsMatrix();

		g_pLightColor = g_pEffect->GetVariableByName("LightColor")->AsVector();
		g_pLightDirection = g_pEffect->GetVariableByName("LightDirection")->AsVector();
		g_pLightPosition = g_pEffect->GetVariableByName("LightPosition")->AsVector();

		g_pSpotLightPhi = g_pEffect->GetVariableByName("Phi")->AsScalar();
		g_pSpotLightTheta = g_pEffect->GetVariableByName("Theta")->AsScalar();

		// Описываем формат входных данных
		D3D10_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
			D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
		D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24,
		D3D10_INPUT_PER_VERTEX_DATA, 0 }
		};
		UINT numElements = sizeof(layout) / sizeof(layout[0]);

		// Создаём объект входных данных
		D3D10_PASS_DESC passDesc;
		g_pTehnigue->GetPassByIndex(0)->GetDesc(&passDesc);

		hr = g_pd3dDevice->CreateInputLayout(layout, numElements,
			passDesc.pIAInputSignature, passDesc.IAInputSignatureSize,
			&g_pVertexLayout);
		if (FAILED(hr))
			return hr;

		// Связываем объект входных данных с графическим конвейером
		g_pd3dDevice->IASetInputLayout(g_pVertexLayout);

		// Создаём буффер вершин
		D3DXVECTOR3 vertices[] =
		{
			D3DXVECTOR3(-1, 1, -1), D3DXVECTOR3(0, 1, 0), D3DXVECTOR4(1,0,0,1),
			D3DXVECTOR3(1, 1, -1), D3DXVECTOR3(0, 1, 0), D3DXVECTOR4(1,0,0,1),
			D3DXVECTOR3(1, 1,  1), D3DXVECTOR3(0, 1, 0), D3DXVECTOR4(1,0,0,1),
			D3DXVECTOR3(-1, 1,  1), D3DXVECTOR3(0, 1, 0), D3DXVECTOR4(1,0,0,1),

			D3DXVECTOR3(-1, -1, -1), D3DXVECTOR3(0, -1, 0), D3DXVECTOR4(0,1,0,1),
			D3DXVECTOR3(1, -1, -1), D3DXVECTOR3(0, -1, 0), D3DXVECTOR4(0,1,0,1),
			D3DXVECTOR3(1, -1,  1), D3DXVECTOR3(0, -1, 0), D3DXVECTOR4(0,1,0,1),
			D3DXVECTOR3(-1, -1,  1), D3DXVECTOR3(0, -1, 0), D3DXVECTOR4(0,1,0,1),

			D3DXVECTOR3(-1, -1,  1), D3DXVECTOR3(-1, 0, 0), D3DXVECTOR4(0,0,1,1),
			D3DXVECTOR3(-1, -1, -1), D3DXVECTOR3(-1, 0, 0), D3DXVECTOR4(0,0,1,1),
			D3DXVECTOR3(-1,  1, -1), D3DXVECTOR3(-1, 0, 0), D3DXVECTOR4(0,0,1,1),
			D3DXVECTOR3(-1,  1,  1), D3DXVECTOR3(-1, 0, 0), D3DXVECTOR4(0,0,1,1),

			D3DXVECTOR3(1, -1,  1), D3DXVECTOR3(1, 0, 0), D3DXVECTOR4(1,1,0,1),
			D3DXVECTOR3(1, -1, -1), D3DXVECTOR3(1, 0, 0), D3DXVECTOR4(1,1,0,1),
			D3DXVECTOR3(1,  1, -1), D3DXVECTOR3(1, 0, 0), D3DXVECTOR4(1,1,0,1),
			D3DXVECTOR3(1,  1,  1), D3DXVECTOR3(1, 0, 0), D3DXVECTOR4(1,1,0,1),

			D3DXVECTOR3(-1, -1, -1), D3DXVECTOR3(0, 0, -1), D3DXVECTOR4(1,0,1,1),
			D3DXVECTOR3(1, -1, -1), D3DXVECTOR3(0, 0, -1), D3DXVECTOR4(1,0,1,1),
			D3DXVECTOR3(1,  1, -1), D3DXVECTOR3(0, 0, -1), D3DXVECTOR4(1,0,1,1),
			D3DXVECTOR3(-1,  1, -1), D3DXVECTOR3(0, 0, -1), D3DXVECTOR4(1,0,1,1),

			D3DXVECTOR3(-1, -1,  1), D3DXVECTOR3(0, 0, 1), D3DXVECTOR4(0,1,1,1),
			D3DXVECTOR3(1, -1,  1), D3DXVECTOR3(0, 0, 1), D3DXVECTOR4(0,1,1,1),
			D3DXVECTOR3(1,  1,  1), D3DXVECTOR3(0, 0, 1), D3DXVECTOR4(0,1,1,1),
			D3DXVECTOR3(-1,  1,  1), D3DXVECTOR3(0, 0, 1), D3DXVECTOR4(0,1,1,1),

		};
		D3D10_BUFFER_DESC bd;
		bd.Usage = D3D10_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(vertices);
		bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		D3D10_SUBRESOURCE_DATA initData;
		initData.pSysMem = vertices;
		hr = g_pd3dDevice->CreateBuffer(&bd, &initData, &g_pVertexBuffer);
		if (FAILED(hr))
			return hr;

		UINT stride = sizeof(SimpleVertex);
		UINT offset = 0;
		// Связыываем буфер вершин с графическим конвейером
		g_pd3dDevice->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

		DWORD indices[] =
		{
			3,1,0, 2,1,3,
			6,4,5, 7,4,6,
			11,9,8, 10,9,11,
			14,12,13, 15,12,14,
			19,17,16, 18,17,19,
			22,20,21, 23,20,22
		};
		bd.ByteWidth = sizeof(indices);
		bd.BindFlags = D3D10_BIND_INDEX_BUFFER;
		initData.pSysMem = indices;
		hr = g_pd3dDevice->CreateBuffer(&bd, &initData, &g_pIndexBuffer);
		if (FAILED(hr))
			return hr;

		g_pd3dDevice->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		g_pd3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


		CreateMatrix();

		return S_OK;
	}

	void CreateMatrix1()
	{
		D3DXMatrixIdentity(&g_World);

		D3DXVECTOR3 Eye(0, 2, -5);
		D3DXVECTOR3 At(0, 0, 0);
		D3DXVECTOR3 Up(0, 1, 0);
		D3DXMatrixLookAtLH(&g_View, &Eye, &At, &Up);

		D3DXMatrixPerspectiveFovLH(
			&g_Projection, (float)D3DX_PI / 4,
			pRend->Xscreen / (float)pRend->Yscreen, 0.1f, 100.0f);
	}

	void RenderScene1()
	{
		static POINT cursor = { -666,0 };
		static POINT cursorPrev;
		cursorPrev = cursor;
		GetCursorPos((LPPOINT)&cursor);
		static float shiftX, shiftY;
		if (cursorPrev.x == -666)
		{
			cursorPrev = cursor;
		}
		shiftX = (cursor.x - cursorPrev.x) / 100.0f;
		shiftY = (cursor.y - cursorPrev.y) / 100.0f;

		static float x = 0, y = 0;
		if (GetKeyState(VK_LBUTTON) < 0)
		{
			x += shiftX;
			y -= shiftY;
		}

		static float t = 0;
		if (g_driverType == D3D10_DRIVER_TYPE_REFERENCE)
		{
			t += (float)D3DX_PI * 0.0125;
		}
		else
		{
			static DWORD dwTimeStart = 0;
			DWORD dwTimeCur = GetTickCount();
			if (dwTimeStart == 0)
				dwTimeStart = dwTimeCur;
			t = (dwTimeCur - dwTimeStart) / 1000.0f;
		}
		D3DXMatrixRotationY(&g_World, t);

		g_pWorldVariable->SetMatrix((float*)&g_World);
		g_pViewVariable->SetMatrix((float*)&g_View);
		g_pProjectionVariable->SetMatrix((float*)&g_Projection);

		float lightColor[4] = { 1, 0.3f, 0.3f, 1 };
		g_pLightColor->SetFloatVector((float*)&lightColor);

		float sx = sinf(y), sy = sinf(x), cx = cos(y), cy = cos(x);
		float lightDir[4] = { 0, 0, 0.0f, 1 };
		g_pLightDirection->SetFloatVector((float*)&lightDir);

		//float lightPos[4] = { x, y, -1.5f, 1 };
		float lightPos[4] = { 2 * sx*cy, 2 * cx, 2 * sx*sy, 1 };
		g_pLightPosition->SetFloatVector((float*)&lightPos);

		float clearColor[4] = { 0.1f, 0.5f, (sinf(k * 10) + 1.0f) / 2, 1.0f };
		g_pd3dDevice->ClearRenderTargetView(g_pRenderTargetView, clearColor);

		g_pSpotLightPhi->SetFloat(D3DXToRadian(50));
		g_pSpotLightTheta->SetFloat(D3DXToRadian(25));

		//Рисуем наш треугольник
		D3D10_TECHNIQUE_DESC techDesc;
		g_pTehnigue->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			g_pTehnigue->GetPassByIndex(p)->Apply(0);
			g_pd3dDevice->DrawIndexed(36, 0, 0);
		}
		// Переключаем буферы
		g_pSwapChain->Present(0, 0);
	}

	int Initialize2()
	{
		HRESULT hr = S_OK;

		UINT width = pRend->Xscreen;
		UINT height = pRend->Yscreen;

		D3D10_DRIVER_TYPE driverTypes[] =
		{
			D3D10_DRIVER_TYPE_HARDWARE,
			D3D10_DRIVER_TYPE_REFERENCE,
		};
		UINT numDriverTypes =
			sizeof(driverTypes) / sizeof(driverTypes[0]);

		// Заполняем текстуру
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = pRend->hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = FALSE;

		// Пытаемся создать устройство, проходя по списку
		// Как только получилось - выходим из цикла
		for (UINT driverTypeIndex = 0;
			driverTypeIndex < numDriverTypes; driverTypeIndex++)
		{
			g_driverType = driverTypes[driverTypeIndex];
			hr = D3D10CreateDeviceAndSwapChain(
				NULL, g_driverType, NULL, 0, D3D10_SDK_VERSION,
				&sd, &g_pSwapChain, &g_pd3dDevice);
			if (SUCCEEDED(hr))
				break;
		}

		if (FAILED(hr))
			return hr;

		// Представление данных для буффера визуализации
		ID3D10Texture2D *pBackBuffer;
		hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (LPVOID*)&pBackBuffer);
		if (FAILED(hr))
			return hr;

		hr = g_pd3dDevice->CreateRenderTargetView(
			pBackBuffer, NULL, &g_pRenderTargetView);
		pBackBuffer->Release();
		if (FAILED(hr))
			return hr;

		// Свяжем буффер визуализации с графическим конвейером
		g_pd3dDevice->OMSetRenderTargets(1, &g_pRenderTargetView, NULL);
		// Настроим область отображения
		D3D10_VIEWPORT vp;
		vp.Width = width;
		vp.Height = height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		g_pd3dDevice->RSSetViewports(1, &vp);

		// Создадим эффект
		hr = D3DX10CreateEffectFromFile(L"first.fx", NULL, NULL, "fx_4_0",
			D3D10_SHADER_ENABLE_STRICTNESS, 0, g_pd3dDevice, NULL, NULL,
			&g_pEffect, NULL, NULL);
		if (FAILED(hr))
		{
			MessageBox(NULL, L"Создай first.fx, идиот", L"ERROR", MB_OK);
			return hr;
		}

		// Извлекаем технику отображения
		g_pTehnigue = g_pEffect->GetTechniqueByName("RenderDirectLight");

		g_pWorldVariable = g_pEffect->GetVariableByName("World")->AsMatrix();
		g_pViewVariable = g_pEffect->GetVariableByName("View")->AsMatrix();
		g_pProjectionVariable = g_pEffect->GetVariableByName("Projection")->AsMatrix();

		g_pLightColor = g_pEffect->GetVariableByName("LightColor")->AsVector();
		g_pLightDirection = g_pEffect->GetVariableByName("LightDirection")->AsVector();

		// Описываем формат входных данных
		D3D10_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
			D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
		D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24,
		D3D10_INPUT_PER_VERTEX_DATA, 0 }
		};
		UINT numElements = sizeof(layout) / sizeof(layout[0]);

		// Создаём объект входных данных
		D3D10_PASS_DESC passDesc;
		g_pTehnigue->GetPassByIndex(0)->GetDesc(&passDesc);

		hr = g_pd3dDevice->CreateInputLayout(layout, numElements,
			passDesc.pIAInputSignature, passDesc.IAInputSignatureSize,
			&g_pVertexLayout);
		if (FAILED(hr))
			return hr;

		// Связываем объект входных данных с графическим конвейером
		g_pd3dDevice->IASetInputLayout(g_pVertexLayout);

		// Создаём буффер вершин
		D3DXVECTOR3 vertices[] =
		{
			D3DXVECTOR3(-1, 1, -1), D3DXVECTOR3(0, 1, 0), D3DXVECTOR4(1,0,0,1),
			D3DXVECTOR3(1, 1, -1), D3DXVECTOR3(0, 1, 0), D3DXVECTOR4(1,0,0,1),
			D3DXVECTOR3(1, 1,  1), D3DXVECTOR3(0, 1, 0), D3DXVECTOR4(1,0,0,1),
			D3DXVECTOR3(-1, 1,  1), D3DXVECTOR3(0, 1, 0), D3DXVECTOR4(1,0,0,1),

			D3DXVECTOR3(-1, -1, -1), D3DXVECTOR3(0, -1, 0), D3DXVECTOR4(0,1,0,1),
			D3DXVECTOR3(1, -1, -1), D3DXVECTOR3(0, -1, 0), D3DXVECTOR4(0,1,0,1),
			D3DXVECTOR3(1, -1,  1), D3DXVECTOR3(0, -1, 0), D3DXVECTOR4(0,1,0,1),
			D3DXVECTOR3(-1, -1,  1), D3DXVECTOR3(0, -1, 0), D3DXVECTOR4(0,1,0,1),

			D3DXVECTOR3(-1, -1,  1), D3DXVECTOR3(-1, 0, 0), D3DXVECTOR4(0,0,1,1),
			D3DXVECTOR3(-1, -1, -1), D3DXVECTOR3(-1, 0, 0), D3DXVECTOR4(0,0,1,1),
			D3DXVECTOR3(-1,  1, -1), D3DXVECTOR3(-1, 0, 0), D3DXVECTOR4(0,0,1,1),
			D3DXVECTOR3(-1,  1,  1), D3DXVECTOR3(-1, 0, 0), D3DXVECTOR4(0,0,1,1),

			D3DXVECTOR3(1, -1,  1), D3DXVECTOR3(1, 0, 0), D3DXVECTOR4(1,1,0,1),
			D3DXVECTOR3(1, -1, -1), D3DXVECTOR3(1, 0, 0), D3DXVECTOR4(1,1,0,1),
			D3DXVECTOR3(1,  1, -1), D3DXVECTOR3(1, 0, 0), D3DXVECTOR4(1,1,0,1),
			D3DXVECTOR3(1,  1,  1), D3DXVECTOR3(1, 0, 0), D3DXVECTOR4(1,1,0,1),

			D3DXVECTOR3(-1, -1, -1), D3DXVECTOR3(0, 0, -1), D3DXVECTOR4(1,0,1,1),
			D3DXVECTOR3(1, -1, -1), D3DXVECTOR3(0, 0, -1), D3DXVECTOR4(1,0,1,1),
			D3DXVECTOR3(1,  1, -1), D3DXVECTOR3(0, 0, -1), D3DXVECTOR4(1,0,1,1),
			D3DXVECTOR3(-1,  1, -1), D3DXVECTOR3(0, 0, -1), D3DXVECTOR4(1,0,1,1),

			D3DXVECTOR3(-1, -1,  1), D3DXVECTOR3(0, 0, 1), D3DXVECTOR4(0,1,1,1),
			D3DXVECTOR3(1, -1,  1), D3DXVECTOR3(0, 0, 1), D3DXVECTOR4(0,1,1,1),
			D3DXVECTOR3(1,  1,  1), D3DXVECTOR3(0, 0, 1), D3DXVECTOR4(0,1,1,1),
			D3DXVECTOR3(-1,  1,  1), D3DXVECTOR3(0, 0, 1), D3DXVECTOR4(0,1,1,1),

		};
		D3D10_BUFFER_DESC bd;
		bd.Usage = D3D10_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(vertices);
		bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		D3D10_SUBRESOURCE_DATA initData;
		initData.pSysMem = vertices;
		hr = g_pd3dDevice->CreateBuffer(&bd, &initData, &g_pVertexBuffer);
		if (FAILED(hr))
			return hr;

		UINT stride = sizeof(SimpleVertex);
		UINT offset = 0;
		// Связыываем буфер вершин с графическим конвейером
		g_pd3dDevice->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

		DWORD indices[] =
		{
			3,1,0, 2,1,3,
			6,4,5, 7,4,6,
			11,9,8, 10,9,11,
			14,12,13, 15,12,14,
			19,17,16, 18,17,19,
			22,20,21, 23,20,22
		};
		bd.ByteWidth = sizeof(indices);
		bd.BindFlags = D3D10_BIND_INDEX_BUFFER;
		initData.pSysMem = indices;
		hr = g_pd3dDevice->CreateBuffer(&bd, &initData, &g_pIndexBuffer);
		if (FAILED(hr))
			return hr;

		g_pd3dDevice->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		g_pd3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


		CreateMatrix();

		return S_OK;
	}

	void CreateMatrix2()
	{
		D3DXMatrixIdentity(&g_World);

		D3DXVECTOR3 Eye(0, 2, -5);
		D3DXVECTOR3 At(0, 0, 0);
		D3DXVECTOR3 Up(0, 1, 0);
		D3DXMatrixLookAtLH(&g_View, &Eye, &At, &Up);

		D3DXMatrixPerspectiveFovLH(
			&g_Projection, (float)D3DX_PI / 4,
			pRend->Xscreen / (float)pRend->Yscreen, 0.1f, 100.0f);
	}

	void RenderScene2()
	{
		static float t = 0;
		if (g_driverType == D3D10_DRIVER_TYPE_REFERENCE)
		{
			t += (float)D3DX_PI * 0.0125;
		}
		else
		{
			static DWORD dwTimeStart = 0;
			DWORD dwTimeCur = GetTickCount();
			if (dwTimeStart == 0)
				dwTimeStart = dwTimeCur;
			t = (dwTimeCur - dwTimeStart) / 1000.0f;
		}
		D3DXMatrixRotationY(&g_World, t);

		g_pWorldVariable->SetMatrix((float*)&g_World);
		g_pViewVariable->SetMatrix((float*)&g_View);
		g_pProjectionVariable->SetMatrix((float*)&g_Projection);

		float lightColor[4] = { 0.5f, 0.5f, 0.5f, 1 };
		g_pLightColor->SetFloatVector((float*)&lightColor);

		float lightDir[4] = { 0, 0, -1.5f, 1 };
		g_pLightDirection->SetFloatVector((float*)&lightDir);

		float clearColor[4] = { 0.1f, 0.5f, (sinf(k*10)+1.0f)/2, 1.0f };
		g_pd3dDevice->ClearRenderTargetView(g_pRenderTargetView, clearColor);

		//Рисуем наш треугольник
		D3D10_TECHNIQUE_DESC techDesc;
		g_pTehnigue->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			g_pTehnigue->GetPassByIndex(p)->Apply(0);
			g_pd3dDevice->DrawIndexed(36, 0, 0);
		}
		// Переключаем буферы
		g_pSwapChain->Present(0, 0);
	}

	int Destroy()
	{
		if (g_pd3dDevice)			g_pd3dDevice->ClearState();
		if (g_pVertexBuffer)		g_pVertexBuffer->Release();
		if (g_pVertexLayout)		g_pVertexLayout->Release();
		if (g_pEffect)				g_pEffect->Release();
		if (g_pRenderTargetView)	g_pRenderTargetView->Release();
		if (g_pSwapChain)			g_pSwapChain->Release();
		if (g_pd3dDevice)			g_pd3dDevice->Release();
		if (g_pDepthStencilView)	g_pDepthStencilView->Release();

		return S_OK;
	}

	int Draw()
	{
		//if (clock()-lk > 16)


		if (flow)
			k += speed * lk / 1000.0f;

		lk = clock();
		clock_t t1 = clock();


		float oldfps = fps;
		fps = lk;

		switch (type)
		{
		default:
			RenderScene();
		}

		if (clock() - t1 < 16)
			Sleep(16. - (clock() - t1));

		if (info)
		{
			fps = clock() - t1;
			fps = 1000.f / fps;
			fps = (fps + oldfps) / 2;
			int j = 0;
			WCHAR *buffer = new WCHAR[1000];
			if (Type)
				j = swprintf(buffer, L"   Фрактал: \t%s\n", L"По Мандельброту");
			else
				j = swprintf(buffer, L"   Фрактал: \t%s\n", L"По Жюлиа");
			j += swprintf(buffer + j, L"   Время: \t%f\n", k);
			j += swprintf(buffer + j, L"   Первая формула: \t%d\n", Frac1);
			j += swprintf(buffer + j, L"   Вторая формула: \t%d\n", Frac2);
			j += swprintf(buffer + j, L"   Глубина: \t%d\n", I);
			j += swprintf(buffer + j, L"   X: \t%f\n", center.x);
			j += swprintf(buffer + j, L"   Y: \t%f\n", center.y);
			j += swprintf(buffer + j, L"   Масштаб по Y: \t%f\n", 2 * Y / Width);
			j += swprintf(buffer + j, L"   Скорость времени: \t%f\n", speed);
			j += swprintf(buffer + j, L"   Сила взаимодействия: \t%f\n", 1 / power);
			j += swprintf(buffer + j, L"   Сила цвета: \t%f\n", colorPower);
			j += swprintf(buffer + j, L"   Цветовая палитра #: \t%d\n", Col);
			j += swprintf(buffer + j, L"   Twist: \t%d%d%d%d%d%d%d%d\n", ci[0], ci[1], ci[2], ci[3], ci[4], ci[5], ci[6], ci[7]);
			j += swprintf(buffer + j, L"   FPS: \t%.3f\n", fps);
			POINT cur;
			GetCursorPos(&cur);
			j += swprintf(buffer + j, L"   CX: \t%d\n", (int)cur.x);
			j += swprintf(buffer + j, L"   CY: \t%d\n", (int)cur.y);


			pRend->DrawString(10, 10, buffer);
			pRend->DrawLine(X - 5, Y - 5, X - 5, Y + 5, 0.5f);
			pRend->DrawLine(X - 10, Y, X + 0, Y, 0.5f);

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

		switch (key)
		{
		case VK_LBUTTON:
			if (type == 3)
			{
				LPPOINT p = new POINT;
				GetCursorPos(p);
				mouseDownOn = p->y / (pRend->Yscreen >> 2) + 1;
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
			center.y -= 5 / Width;
			break;
		case VK_DOWN:
			center.y += 5 / Width;
			break;
		case VK_LEFT:
			center.x -= 5 / Width;
			break;
		case VK_RIGHT:
			center.x += 5 / Width;
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
			{
				CatchMessage(VK_MULTIPLY);
				break;
			}
			ModFrac(1, 1);
			break;
		case VK_OEM_MINUS:
			if (GetKeyState(0xC0) & 0x8000)
			{
				CatchMessage(VK_DIVIDE);
				break;
			}
			if (GetKeyState(VK_SHIFT) & 0x8000)
			{
				SwitchFrac(0, Frac2);
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
				SwitchFrac(Frac1, 0);
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
			for (int i = 0; i < 8; i++)
			{
				k = rand() % 8;
				while (b & (1 << k))
					k = rand() % 8;
				b |= (1 << k);
				ci[k] = i;
				//k = 8 - rand()%(8-i);
				//b = ci[i];
				//ci[i] = ci[k];
				//ci[k] = b;
			}
			break;
		}
		case 'I':
			info = info ? false : true;
			break;

		case VK_NUMPAD9:
			if (GetKeyState(VK_SHIFT) & 0x8000)
			{
				k = 0;
				break;
			}
			flow = flow ? false : true;
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
			SwitchFrac(rand() % MAX_FRACS, rand() % MAX_FRACS);
			prevType = Type;
			Type = rand() % 2;
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
			center.x = center.y = 0;
			Width = X / 5.0f;
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
		default:
			break;
		}
		return 0;
	}

	void SaveImage(bool stoped)
	{
	}

};