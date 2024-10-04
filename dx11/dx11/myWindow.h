#pragma once
#include <windows.h>
class Window
{
public:
	static float A, S, D, W, Q, E;
	Window(HINSTANCE& hInstance, HINSTANCE& hPrevInstance, LPSTR& lpCmdLine, int& nCmdShow);
	void Run();
private:
	HINSTANCE hInstance;
	HINSTANCE hPrevInstance;
	LPSTR     lpCmdLine;
	int       nCmdShow;
};
