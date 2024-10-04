#include"myWindow.h"
#include <iostream>
using namespace std;

int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow)
{
	Window win = Window(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
	win.Run();
	return 0;
}