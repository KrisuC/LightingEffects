#pragma once

class D3D12DescriptorHeapManager
{

};

class D3D12GraphicsContext
{
private:
	void CreateDxgiFactory()
	{
		// Initialize the device
		uint32_t dxgiFactoryFlags = 0;

	#if defined(_DEBUG)
		// Enabling debug layer
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	#endif
		// Create dxgi factory
		ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_dxgiFactory)));
	}

	void CreateDevice(bool bRequestHighPerformanceAdapter = true)
	{
		// Find qualified hardware adapter
		ComPtr<IDXGIAdapter1> adapter;
		ComPtr<IDXGIFactory6> factory6;
		if (SUCCEEDED(m_dxgiFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
		{
			for (UINT adapterIndex = 0; ; adapterIndex++)
			{
				SUCCEEDED(factory6->EnumAdapterByGpuPreference(
					adapterIndex,
					bRequestHighPerformanceAdapter ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
					IID_PPV_ARGS(&adapter)
				));
				DXGI_ADAPTER_DESC1 desc;
				adapter->GetDesc1(&desc);
				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					continue;
				}
				// Try creating device
				if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device))))
				{
					break;
				}
			}
		}
	}

	// For now we only create a graphics queue
	void CreateCommandQueues()
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc{};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
		// Create command allocator
		ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
	}

	void CreateCommandAllocator()
	{
		// Create command allocator
		ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
	}

	void CreateCommandList()
	{
		// Create command list
		ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
		// Command lists are created in recording state, manually close
		ThrowIfFailed(m_commandList->Close());
	}

public:
	D3D12GraphicsContext(uint32_t width, uint32_t height)
		: m_backBufferWidth(width)
		, m_backBufferHeight(height)
		, m_viewport(0.f, 0.f, float(width), float(height))
		, m_scissorRect(0, 0, width, height)
	{
		CreateDxgiFactory();
		CreateDevice(true);
		// Command queue should be created before swapchain because swapchain needa a command queue
		// to flush on it.
		CreateCommandQueues();
		CreateCommandAllocator();
		CreateCommandList();
	}

	void CreateSwapChain(HWND hWnd, uint32_t frameCount, uint32_t width, uint32_t height)
	{
		// Create swap chain
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
		swapChainDesc.BufferCount = frameCount;
		swapChainDesc.Width = width;
		swapChainDesc.Height = height;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;

		ComPtr<IDXGISwapChain1> swapChain;
		ThrowIfFailed(m_dxgiFactory->CreateSwapChainForHwnd(
			m_commandQueue.Get(), // Swap chain needs a queue to flush on it
			hWnd,
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain
		));

		// Disable full screen transition
		ThrowIfFailed(m_dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));
		// SwapChain1 created -> SwapChain3
		ThrowIfFailed(swapChain.As(&m_swapChain));
	}


	~D3D12GraphicsContext()
	{
	}

	// Resetting command allocator and command list
	void BeginFrame(ID3D12PipelineState* initialPipelineState = nullptr)
	{
		// Command allocators can be reset only if associated command lists have finished excution
		// So fence is needed
		ThrowIfFailed(m_commandAllocator->Reset());
		// Reset command list before re-recording, after ExecuteCommandList is called
		ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), initialPipelineState));
	}

	void EndFrame()
	{
	}

	ComPtr<ID3D12Device>&				GetDevice() { return m_device; }
	ComPtr<ID3D12CommandQueue>&			GetCommandQueue() { return m_commandQueue; }
	ComPtr<ID3D12GraphicsCommandList>&	GetCommandList() { return m_commandList; }
	ComPtr<IDXGISwapChain3>&			GetSwapChain() { return m_swapChain; }
	
	CD3DX12_VIEWPORT const&	GetViewport() const { return m_viewport; }
	CD3DX12_RECT const&		GetScissorRect() const { return m_scissorRect; }

	uint32_t GetBackBufferWidth() const { return m_backBufferWidth; }
	uint32_t GetBackBufferHeight() const { return m_backBufferHeight; }

private:
	ComPtr<IDXGIFactory4>	m_dxgiFactory;
	ComPtr<ID3D12Device>	m_device;

	CD3DX12_VIEWPORT		m_viewport;
	CD3DX12_RECT			m_scissorRect;
	uint32_t				m_backBufferWidth;
	uint32_t				m_backBufferHeight;
	ComPtr<IDXGISwapChain3> m_swapChain;

	ComPtr<ID3D12CommandAllocator>		m_commandAllocator;
	ComPtr<ID3D12CommandQueue>			m_commandQueue;
	ComPtr<ID3D12GraphicsCommandList>	m_commandList;
};
