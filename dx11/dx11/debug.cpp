#include"debug.h"
#include<comdef.h>
#include <string>
void Debug::DebugHr(HRESULT debug) {
	_com_error error(debug);
	auto logStr = error.ErrorMessage();
	std::wstring logWstr = logStr;
	if (FAILED(debug)) {
		MessageBoxW(NULL, logWstr.c_str(), L"error", MB_ICONERROR);
	}
}