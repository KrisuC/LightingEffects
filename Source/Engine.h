#pragma once

#include "D3D12Utility.h"
#include "D3D12RenderContext.h"
#include "Win32Application.h"

enum
{
	FrameCount = 3,
};

struct Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 color;
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

	uint32_t GetWidth() const { return m_context.GetBackBufferWidth(); }
	uint32_t GetHeight() const { return m_context.GetBackBufferHeight(); }
	const wchar_t* GetTitle() const { return m_title.c_str(); }

	void ParseCommandLineArgs(_In_reads_(argc) wchar_t* argv[], int32_t argc);

	std::wstring GetAssetFullPath(const wchar_t* assetName);

	void WaitForGpuCommandCompletion();

private:
	D3D12GraphicsContext m_context;

	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12PipelineState> m_pipelineState;

	uint32_t m_rtvDescriptorSize;

	// App resource
	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	// Synchronization objects
	uint32_t m_frameIndex;
	HANDLE m_fenceEvent;
	ComPtr<ID3D12Fence> m_fenceObject;
	uint64_t m_fenceValue;

	std::wstring m_assetsPath;
	std::wstring m_title;
};
