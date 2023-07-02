#include "Win32Application.h"
#include "Engine.h"

HWND Win32Application::m_hWnd = nullptr;

int Win32Application::Run(Engine* pEngine, HINSTANCE hInstance, int nCmdShow)
{
	// Parsing command line args
	int32_t argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	pEngine->ParseCommandLineArgs(argv, argc);
	LocalFree(argv);

	// Initialize window class
	WNDCLASSEX windowClass{};
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = Win32Application::WindowProc;
	windowClass.hInstance = hInstance;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = L"EngineClass";
	RegisterClassEx(&windowClass);

	RECT windowRect = { 0, 0, pEngine->GetWidth(), pEngine->GetHeight() };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE /* bMenu */);

	m_hWnd = CreateWindow(
		windowClass.lpszClassName,
		pEngine->GetTitle(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr,
		nullptr,
		hInstance,
		pEngine /* lpParam, custom data can be retrieved from WinProc */
	);

	// Initialize engine
	pEngine->OnInit();

	ShowWindow(m_hWnd, nCmdShow);

	// Message loop
	MSG msg{};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// Real game loop is here.
		pEngine->OnUpdate();
	}

	// Destroy engine resource
	pEngine->OnDestroy();

	return static_cast<int>(msg.wParam);
}

LRESULT Win32Application::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Engine* pEngine = reinterpret_cast<Engine*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (message)
	{
	case WM_CREATE:
		{
			// Retrieve and save the Engine* passed from CreateWindow
			LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
		}
		return 0;

	case WM_KEYDOWN:
		if (pEngine)
		{
			pEngine->OnKeyDown(static_cast<UINT8>(wParam));
		}
		return 0;

	case WM_KEYUP:
		if (pEngine)
		{
			pEngine->OnKeyUp(static_cast<UINT8>(wParam));
		}
		return 0;

	case WM_SIZE:
		if (pEngine)
		{
			pEngine->OnResize(LOWORD(lParam), HIWORD(lParam));
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	// Handle other type of message
	return DefWindowProc(hWnd, message, wParam, lParam);
}
