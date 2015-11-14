#include "stdafx.h"
#include "ApplicationCore.h"


namespace sl
{
	//
	//
	//
	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message) {
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			break;
		}

		//case WM_SIZE:
		//	break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

		return 0;
	}


	HWND ApplicationCore::hWnd_ = 0;


	//
	//
	//
	void ApplicationCore::Initialize(int width, int height, const wchar_t* className)
	{
		HINSTANCE hInst = GetModuleHandle(0);

		// Register class
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInst;
		wcex.hIcon = LoadIcon(hInst, IDI_APPLICATION);
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = className;
		wcex.hIconSm = LoadIcon(hInst, IDI_APPLICATION);
		slAssertMsg(0 != RegisterClassEx(&wcex), "Unable to register a window");

		// Create window
		RECT rc = { 0, 0, (LONG)width, (LONG)height };
		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

		hWnd_ = CreateWindow(className, className, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
			rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInst, nullptr);

		slAssert(hWnd_ != 0);

		ShowWindow(hWnd_, SW_SHOWDEFAULT);
	}


	//
	//
	//
	void ApplicationCore::Finalize()
	{
	}
}