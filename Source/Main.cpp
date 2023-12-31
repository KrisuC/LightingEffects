#include "Win32Application.h"
#include "Engine.h"

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	const uint32_t width = 1280;
	const uint32_t height = 720;
	const wchar_t* title = L"Lighting Effects";
	Engine engine(width, height, title);
	return Win32Application::Run(&engine, hInstance, nCmdShow);
}

