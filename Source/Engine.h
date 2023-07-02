#pragma once

#include "D3D12Utility.h"
#include "Win32Application.h"
#include "Scene.h"

class Viewport
{
public:
	Viewport(uint32_t width, uint32_t height)
		: m_width(width)
		, m_height(height)
		, m_aspectRatio((float)width / height)
	{
	}

public:
	uint32_t m_width;
	uint32_t m_height;
	float m_aspectRatio;
};

class Engine
{
public:
	Engine(uint32_t width, uint32_t height, std::wstring name);
	~Engine();

	void OnInit();
	void OnResize(uint32_t newWidth, uint32_t newHeight);
	void OnUpdate();
	void OnDestroy();

	void OnKeyDown(uint8_t key);
	void OnKeyUp(uint8_t key);

	uint32_t GetWidth() const { return m_viewport.m_width; }
	uint32_t GetHeight() const { return m_viewport.m_height; }
	const wchar_t* GetTitle() const { return m_title.c_str(); }

	void ParseCommandLineArgs(_In_reads_(argc) wchar_t* argv[], int32_t argc);

private:
	Viewport m_viewport;

	std::wstring m_assertPath;
	std::wstring m_title;
};
