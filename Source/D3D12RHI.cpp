#include "D3D12RHI.h"

inline d3d12rhi_ll::CommandQueue::CommandQueue(ID3D12Device* pDevice, const D3D12_COMMAND_QUEUE_DESC& queueDesc)
{
	ThrowIfFailed(pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&pDevice)));
}

/** Will enabling debug layer here if needed */

inline d3d12rhi_ll::DXGIFactory::DXGIFactory()
{
	UINT dxgiFactoryFlag = 0u;
#if BUILD_DEBUG
	ComPtr<ID3D12Debug> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
		dxgiFactoryFlag |= DXGI_CREATE_FACTORY_DEBUG;
	}
#endif
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlag, IID_PPV_ARGS(&m_factory)));
}

/** Finding the highest performance GPU and create device */

inline d3d12rhi_ll::Device::Device(IDXGIFactory1* pFactory, bool bRequestHighPerformace)
{
	ComPtr<IDXGIFactory6> factory6;
	if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
	{
		uint32_t adapterIndex = 0u;
		const DXGI_GPU_PREFERENCE gpuPreference = bRequestHighPerformace
			? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE
			: DXGI_GPU_PREFERENCE_UNSPECIFIED;

		ComPtr<IDXGIAdapter1> adapter;
		while (SUCCEEDED(factory6->EnumAdapterByGpuPreference(adapterIndex, gpuPreference, IID_PPV_ARGS(&adapter))))
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// If "/warp" is specified, choose software implementation
				continue;
			}

			// See if the adapter can create D3D12 device, but not creating the device yet...
			// @todo: move this to device creation
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device))))
			{
				break;
			}

			++adapterIndex;
		}
		check(adapter.Get() != nullptr);
		m_adapter = adapter.Detach();
	}
}

inline d3d12rhi_ll::DescriptorHeap::DescriptorHeap(ID3D12Device* pDevice, const D3D12_DESCRIPTOR_HEAP_DESC& heapDesc)
{
	ThrowIfFailed(pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_heap)));
	m_descriptorSize = pDevice->GetDescriptorHandleIncrementSize(heapDesc.Type);
	m_cpuHandle = m_heap->GetCPUDescriptorHandleForHeapStart();
}

inline D3D12_CPU_DESCRIPTOR_HANDLE d3d12rhi_ll::DescriptorHeap::GetDescriptorHandleByIndex(size_t index) const
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	handle.ptr = m_cpuHandle.ptr + m_descriptorSize * index;
	return handle;
}

inline DXGI_SWAP_CHAIN_DESC1 d3d12rhi_ll::SwapChain::CreateDescDefault(uint32_t width, uint32_t height, uint32_t frameCount)
{
	DXGI_SWAP_CHAIN_DESC1 desc;
	desc.Width = width;
	desc.Height = height;
	desc.BufferCount = frameCount;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc.SampleDesc.Count = 1;
	return desc;
}

inline d3d12rhi_ll::SwapChain::SwapChain(IDXGIFactory4* pFactory, ID3D12CommandQueue* pCmdQueue, HWND hWnd, const DXGI_SWAP_CHAIN_DESC1& swapChainDesc)
{
	ComPtr<IDXGISwapChain1> swapChain1;
	ThrowIfFailed(pFactory->CreateSwapChainForHwnd(
		pCmdQueue,
		hWnd,
		&swapChainDesc,
		nullptr, // pFullScreenDesc
		nullptr, // pRestrictToOutput
		&swapChain1
	));
	ThrowIfFailed(pFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));
	ThrowIfFailed(swapChain1.As(&m_swapChain)); // writes to m_swapChain
	m_bufferCount = swapChainDesc.BufferCount;
}

inline void d3d12rhi_ll::SwapChain::GetBuffer(uint32_t index, ID3D12Resource* pResource)
{
	ThrowIfFailed(m_swapChain->GetBuffer(index, IID_PPV_ARGS(&pResource)));
}

inline void d3d12rhi_ll::SwapChain::Present(uint32_t syncInterval)
{
	ThrowIfFailed(m_swapChain->Present(syncInterval, 0u /* flags */));
}

inline D3D12_ROOT_SIGNATURE_DESC d3d12rhi_ll::RootSignature::CreateEmpty()
{
	D3D12_ROOT_SIGNATURE_DESC desc;
	desc.NumParameters = 0u;
	desc.pParameters = nullptr;
	desc.NumStaticSamplers = 0u;
	desc.pStaticSamplers = nullptr;
	desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
	return desc;
}

inline d3d12rhi_ll::RootSignature::RootSignature(ID3D12Device* pDevice, const D3D12_ROOT_SIGNATURE_DESC& rootSignatureDesc)
{
	ComPtr<ID3DBlob> errorBlob;
	ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &m_signatureBlob, &errorBlob));
	// @todo: implement outputing errorBlob
	ThrowIfFailed(pDevice->CreateRootSignature(0u, m_signatureBlob->GetBufferPointer(), m_signatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_signature)));
}

inline constexpr const char* d3d12rhi_ll::ShaderBinary::GetTargetName(ShaderStage shaderType)
{
	switch (shaderType)
	{
	case ShaderStage::Vertex:	return "vs_5_0";
	case ShaderStage::Pixel:	return "ps_5_0";
	case ShaderStage::Domain:	return "ds_5_0";
	case ShaderStage::Hull:		return "hs_5_0";
	case ShaderStage::Geometry: return "gs_5_0";
	case ShaderStage::Compute:	return "cs_5_0";
	}
	check(0);
}

inline d3d12rhi_ll::ShaderBinary::ShaderBinary(const wchar_t* filePath, const char* entryPoint, ShaderStage type)
{
#if BUILD_DEBUG
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0u;
#endif
	ThrowIfFailed(D3DCompileFromFile(filePath, nullptr, nullptr, entryPoint, GetTargetName(type), compileFlags, 0u, &m_shaderBlob, nullptr));
}

inline void d3d12rhi_ll::VertexInputLayout::AddInputElement(const D3D12_INPUT_ELEMENT_DESC& desc)
{
	m_inputElementDesc.push_back(desc);
}

inline void d3d12rhi_ll::VertexInputLayout::AddInputElement(const char* semanticName, DXGI_FORMAT format, D3D12_INPUT_CLASSIFICATION inputSlotClass)
{
	D3D12_INPUT_ELEMENT_DESC desc{};
	desc.SemanticName = semanticName;
	desc.Format = format;
	desc.InputSlotClass = inputSlotClass;
	m_inputElementDesc.push_back(desc);
}
