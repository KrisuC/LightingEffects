#include "D3D12Utility.h"
#include "Win32Application.h"
#include "Scene.h"

struct Viewport
{
	UINT m_width;
	UINT m_height;
	float m_aspectRatio;
};

class Engine
{
public:
	Engine(UINT width, UINT height, std::wstring name);
	~Engine();

	void OnInit();
	void OnUpdate();
	void OnRender();
	void OnDestroy();
};
