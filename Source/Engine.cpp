#include "Engine.h"
#include "D3D12RHI.h"

_Use_decl_annotations_
void Engine::ParseCommandLineArgs(wchar_t* argv[], int32_t argc)
{
}

Engine::Engine(uint32_t width, uint32_t height, std::wstring name)
	: m_viewport(width, height)
	, m_title(name)
{
}

Engine::~Engine()
{
}

void Engine::OnInit()
{
	
}

void Engine::OnResize(uint32_t newWidth, uint32_t newHeight)
{
}

void Engine::OnUpdate()
{
}

void Engine::OnDestroy()
{
}

void Engine::OnKeyDown(uint8_t key)
{
}

void Engine::OnKeyUp(uint8_t key)
{
}
