#include "Win32Application.h"
#include "Engine.h"

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Engine engine(1280, 720, L"Lighting Effects");
	return Win32Application::Run(&engine, hInstance, nCmdShow);
}

