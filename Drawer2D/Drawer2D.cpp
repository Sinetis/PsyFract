// Drawer2D.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Drawer2D.h"
#include "Constants.h"
#include "Controller.h"
#include <commdlg.h>


#define MAX_LOADSTRING 1000

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
int render(1);
LPWSTR AudioFilePath(L"conga_2.wav");

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	ChangeAudio(HWND, UINT, WPARAM, LPARAM);




// The Renderer object
Controller* pCtrl;

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_DRAWER2D, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DRAWER2D));

	// Main message loop:
try
{
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}
catch(std::exception e)
{
	pCtrl->~Controller();
	MessageBox(NULL,LPCWSTR(e.what()),L"Error",MB_OK);
}
catch(LPCWSTR str)
{
	MessageBox(NULL,str,L"Error",MB_OK);
}


	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DRAWER2D));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_DRAWER2D);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_POPUP,//WS_OVERLAPPEDWINDOW,
	   0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 
	   NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   pCtrl = new Controller(hWnd, 
	   GetSystemMetrics(SM_CXSCREEN), 
	   GetSystemMetrics(SM_CYSCREEN), true);
   

   

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	POINT lp;
	switch (message)
	{
	case WM_CREATE:
		//PLAY(L"C:\\conga_2.wav");
		//aud.Load(L"C:\\Obraztsy\\Trolley_Techno_16bit_init.wav");
		//aud.Load(L"C:\\Users\\јнатолий\\Desktop\\‘урье-анализатор\\test.wav");

		//hdc = BeginPaint(hWnd, &ps);
		//pRend->StartRender();
		//EndPaint(hWnd, &ps);
		//aud.Load(AudioFilePath);
		//aud.Play();
		break;
	case WM_PAINT:
	case WM_DISPLAYCHANGE:
		{
		
		break;
		}
	case WM_LBUTTONDOWN:
	case WM_KEYDOWN:
		pCtrl->CatchMessage(wParam);
		
		switch(wParam)
		{
		case VK_ESCAPE:
			//pRend->StopRender();
			PostQuitMessage(0);
			break;

		//case 'A':
		//	{
		//		char filename[MAX_PATH]="";             /*буфер под им€ файла*/
		//		OPENFILENAME of;
		//		ZeroMemory(&of, sizeof(OPENFILENAME));
		//		of.lStructSize = sizeof(OPENFILENAME);
		//		of.hwndOwner = hWnd;                        /*дескриптор родительского окна*/
		//		of.hInstance = hInst;                         /*дескриптор экземпл€ра программы*/
		//		of.lpstrFilter = L"Wave files (*.WAV)\0*.wav\0";/*фильтр файлов (тип)*/
		//		//of.lpstrCustomFilter = NULL;              /*еще один фильтр: нам не надо*/
		//		//of.nMaxCustFilter = 0;                    /*нам не надо*/
		//		of.nFilterIndex = 1;                      /*количество заданных нами фильтров*/
		//		of.lpstrFile = LPWSTR(filename);                  /*адрес буфера под им€ файла*/
		//		of.nMaxFile = MAX_PATH;                   /*размер буфера под им€ файла*/
		//		//of.lpstrFileTitle = NULL;                 /*буфер под рекомендуемый заголовок: нам не надо*/
		//		//of.nMaxFileTitle = 0;                     /*нам не надо*/
		//		//of.lpstrInitialDir = NULL;                /*стартовый каталог: текущий*/
		//		of.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST|OFN_HIDEREADONLY; /*разные флаги*/
		//		if (GetOpenFileName(&of)) 
		//		{
		//			/* действи€ в случае успешного выбора файла */
		//			AudioFilePath = of.lpstrFile;
		//			Audio *t = new Audio();
		//			t->Load(AudioFilePath);
		//			aud.Stop();
		//			t->Play();
		//			pRend->SetAudio(t);
		//			aud.Release();
		//			aud = *t;
		//			//SetDlgItemText(hDlg, IDC_EDIT1, of.lpstrFile);
		//		}
		//	}
		//	break;
		}
		break;
	//case WM_CHAR:
	//	if (wParam == 'a')
	//		DialogBox(hInst, MAKEINTRESOURCE(IDD_CHANGE_AUDIO), hWnd, NULL);
	//	break;

	case WM_DESTROY:
		//aud.Release();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

INT_PTR CALLBACK ChangeAudio(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	int wmId, wmEvent;

	switch (message)
	{
	case WM_ACTIVATE:
		SetDlgItemText(hDlg, IDC_EDIT1, AudioFilePath);
		//return (INT_PTR)TRUE;

	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
			case IDCANCEL:
				EndDialog(hDlg, wmId);
				return (INT_PTR)TRUE;

			case IDOK:
				EndDialog(hDlg, wmId);
				//GetDlgItemText(hDlg, IDC_EDIT1, AudioFilePath, 80);
				return (INT_PTR)TRUE;

			//case IDC_BUTTON1:
			
		}
	}
	return (INT_PTR)FALSE;
}
