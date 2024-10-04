#include "myWindow.h"
float Window::A = 0.0, Window::S = 0.0, Window::D = 0.0, Window::W = 0.0, Window::Q = 0.0, Window::E = 0.0;
#include"graphics.h"
#include <corecrt_math.h>

Window::Window(HINSTANCE& hInstance, HINSTANCE& hPrevInstance, LPSTR& lpCmdLine, int& nCmdShow) {
	this->hInstance = hInstance;
	this->hPrevInstance = hPrevInstance;
	this->lpCmdLine = lpCmdLine;
	this->nCmdShow = nCmdShow;
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_CLOSE:
		PostQuitMessage(69);
		break;
	case WM_LBUTTONDOWN:
		const POINTS pt = MAKEPOINTS(lParam);
		break;
	case WM_KEYDOWN:
		if (wParam == 'S') {
			Window::S -= 0.05;
		}
		else if (wParam == 'A') {
			Window::A -= 0.05;
		}
		else if (wParam == 'W') {
			Window::W += 0.05;
		}
		else if (wParam == 'D') {
			Window::D += 0.05;
		}
		else if (wParam == 'Q') {
			Window::Q += 0.05;
		}
		else if (wParam == 'E') {
			Window::E -= 0.05;
		}
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void Window::Run()
{
	const auto pClassName = TEXT("dx11");
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = nullptr;
	wc.hCursor = nullptr;
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = pClassName;
	wc.hIconSm = nullptr;
	RegisterClassEx(&wc);
	HWND hWnd = CreateWindowEx(0, pClassName, TEXT("dx11"), WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_MAXIMIZEBOX,
		200, 200, 640, 480, nullptr, nullptr, hInstance, nullptr);
	ShowWindow(hWnd, SW_SHOW);
	Graphics graphic = Graphics(hWnd);
	graphic.CreatBuffer();

	MSG msg;
	bool isLoop = true;
	float count = 0.0;
	while (isLoop) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT)
		{
			isLoop = false;
		}
		graphic.ClearBuffer(sin(count), cos(count), sin(count));
		graphic.DrawTriangle(A, S, D, W, Q, E);
		graphic.SwapBuffer();
		count += 0.01;
	}
}
