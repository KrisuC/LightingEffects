#pragma once

#include "D3D12Utility.h"
#include "EngineUtility.h"
#include <dxgi.h>
#include <dxgi1_6.h>
#include <vector>
#include <d3d12.h>

/** Low-level encapsulation */
namespace d3d12rhi_ll
{

/** Enumerations */
enum class CommandListType
{
	Direct = D3D12_COMMAND_LIST_TYPE_DIRECT,
	Compute = D3D12_COMMAND_LIST_TYPE_COMPUTE,
	Copy = D3D12_COMMAND_LIST_TYPE_COPY
};

enum class DescriptorHeapType
{
	CBV_SRV_UAV = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
	Sampler = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
	RTV = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
	DSV = D3D12_DESCRIPTOR_HEAP_TYPE_DSV
};

enum class ShaderStage
{
	Vertex	 = 0,
	Pixel	 = 1,
	Domain	 = 2,
	Hull	 = 3,
	Geometry = 4,
	Compute  = 5
};

enum class HeapType
{
	Default		= D3D12_HEAP_TYPE_DEFAULT,
	Upload		= D3D12_HEAP_TYPE_UPLOAD,
	Readback	= D3D12_HEAP_TYPE_READBACK,
	Custom		= D3D12_HEAP_TYPE_CUSTOM
};

/** Command queue */
class CommandQueue
{
public:
	CommandQueue(ID3D12Device* pDevice, const D3D12_COMMAND_QUEUE_DESC& queueDesc);

	template <CommandListType cmdListType>
	static CommandQueue CreateQueue(ID3D12Device* pDevice);

private:
	ComPtr<ID3D12CommandQueue> m_commandQueue;
};



/** DXGIFactory */
class DXGIFactory
{
public:
	/** Will enabling debug layer here if needed */
	DXGIFactory();

	IDXGIFactory4* GetFactory() const { return m_factory.Get(); }

private:
	ComPtr<IDXGIFactory4> m_factory;
};



/** Device */
class Device
{
public:
	/** Finding the highest performance GPU and create device */
	Device(IDXGIFactory1* pFactory, bool bRequestHighPerformace = true);

	ID3D12Device* operator->() const { return m_device.Get(); }
	ID3D12Device* GetDevice() const { return m_device.Get(); }

	IDXGIAdapter1* GetAdapter() const { return m_adapter.Get(); }

private:

	ComPtr<IDXGIAdapter1> m_adapter;
	ComPtr<ID3D12Device> m_device;
};

/** Render target */
class Resource
{
public:
	Resource() {}

	ID3D12Resource* GetResource() const { return m_resource.Get(); }
	ComPtr<ID3D12Resource>* GetResourceRef() { return &m_resource; }

private:
	ComPtr<ID3D12Resource> m_resource;
};



/** Descriptor heap, for RTV, DSV, CBV... */
class DescriptorHeap
{
public:
	DescriptorHeap(ID3D12Device* pDevice, const D3D12_DESCRIPTOR_HEAP_DESC& heapDesc);

	D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandleByIndex(size_t index) const;

	ID3D12DescriptorHeap* GetHeap() const { return m_heap.Get(); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetGpuHandle() const { return m_cpuHandle; }
	size_t GetIncrementSize() const { return m_descriptorSize; }

private:

	size_t m_descriptorSize;
	// @todo: gpu handle
	D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle;
	ComPtr<ID3D12DescriptorHeap> m_heap;
};



/** Swapchain, associate with an hWnd */
class SwapChain
{
public:
	static DXGI_SWAP_CHAIN_DESC1 CreateDescDefault(uint32_t width, uint32_t height, uint32_t frameCount);

	SwapChain(IDXGIFactory4* pFactory, ID3D12CommandQueue* pCmdQueue, HWND hWnd, const DXGI_SWAP_CHAIN_DESC1& swapChainDesc);

	int32_t GetCurrentBackBufferIndex() { return m_swapChain->GetCurrentBackBufferIndex(); }

	void GetBuffer(uint32_t index, ID3D12Resource* pResource);

	void Present(uint32_t syncInterval = 1u);

	uint32_t GetBufferCount() const { return m_bufferCount; }
	IDXGISwapChain3* Get() const { return m_swapChain.Get(); }

private:

	uint32_t m_bufferCount;
	ComPtr<IDXGISwapChain3> m_swapChain;
};


/** Root signature */
class RootSignature
{
public:
	inline static D3D12_ROOT_SIGNATURE_DESC CreateEmpty();

	RootSignature(ID3D12Device* pDevice, const D3D12_ROOT_SIGNATURE_DESC& rootSignatureDesc);

	ID3D12RootSignature* GetSignature() const { return m_signature.Get(); }

private:
	ComPtr<ID3DBlob> m_signatureBlob;
	ComPtr<ID3D12RootSignature> m_signature;
};



/** Shader */
class ShaderBinary
{
public:
	constexpr const char* GetTargetName(ShaderStage shaderType);

	ShaderBinary(const wchar_t* filePath, const char* entryPoint, ShaderStage type);

	D3D12_SHADER_BYTECODE GetShaderByteCode() const { return {m_shaderBlob->GetBufferPointer(), m_shaderBlob->GetBufferSize() }; }
	ID3DBlob* GetBlob() const { return m_shaderBlob.Get(); }

private:
	ComPtr<ID3DBlob> m_shaderBlob;
};


/** */
class VertexInputLayout
{
public:
	VertexInputLayout() {}

	void AddInputElement(const D3D12_INPUT_ELEMENT_DESC& desc);

	void AddInputElement(const char* semanticName, DXGI_FORMAT format, D3D12_INPUT_CLASSIFICATION inputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA);

	const size_t GetNumInputElement() const { return m_inputElementDesc.size(); }

private:
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputElementDesc;
};

/** */
template<
	D3D12_FILL_MODE FillMode								 = D3D12_FILL_MODE_SOLID,
	D3D12_CULL_MODE CullMode								 = D3D12_CULL_MODE_BACK,
	bool			FrontCounterClockWise					 = FALSE,
	int32_t			DepthBias								 = D3D12_DEFAULT_DEPTH_BIAS,
	bool			DepthClipEnable							 = TRUE,
	bool			MultisampleEnable						 = FALSE,
	bool			AntialiasedLineEnable					 = FALSE,
	uint32_t		ForcedSampleCount						 = 0,
	D3D12_CONSERVATIVE_RASTERIZATION_MODE ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF>
class RasterizerState
{
public:
	RasterizerState() {}

	inline D3D12_RASTERIZER_DESC GetDesc() const
	{
		D3D12_RASTERIZER_DESC desc;
		desc.FillMode				= FillMode;
		desc.CullMode				= CullMode;
		desc.FrontCounterClockwise	= FrontCounterClockWise;
		desc.DepthBias				= DepthBias;
		desc.DepthClipEnable		= DepthClipEnable ? TRUE : FALSE;
		desc.MultisampleEnable		= MultisampleEnable ? TRUE : FALSE;
		desc.AntialiasedLineEnable	= AntialiasedLineEnable ? TRUE : FALSE;
		desc.ForcedSampleCount		= ForcedSampleCount;
		desc.DepthBiasClamp			= DepthBiasClamp;
		desc.SlopeScaledDepthBias	= SlopeScaledDepthBias;
		return desc;
	}

	float DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	float SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
};


/** */
template<
	bool					DepthEnable		  = true,
	D3D12_DEPTH_WRITE_MASK	DepthWriteMask    = D3D12_DEPTH_WRITE_MASK_ALL,
	D3D12_COMPARISON_FUNC	DepthFunc		  = D3D12_COMPARISON_FUNC_LESS_EQUAL,
	bool					StencilEnable	  = false,
	uint8_t					StencilReadMask	  = 0xff,
	uint8_t					StencilWriteMask  = 0xff
>
class DepthStencilState
{
public:
	inline D3D12_DEPTH_STENCIL_DESC GetDesc() const
	{
		D3D12_DEPTH_STENCIL_DESC desc;
		desc.DepthEnable		= DepthEnable ? TRUE : FALSE;
		desc.DepthWriteMask		= DepthWriteMask;
		desc.DepthFunc			= DepthFunc;
		desc.StencilEnable		= StencilEnable ? TRUE : FALSE;
		desc.StencilReadMask	= StencilReadMask;
		desc.StencilWriteMask	= StencilWriteMask;
		desc.FrontFace			= FrontFace;
		desc.BackFace			= BackFace;
	}

	D3D12_DEPTH_STENCILOP_DESC FrontFace{};
	D3D12_DEPTH_STENCILOP_DESC BackFace{};
};


/** SrcColor * SrcAlpha + DestColor * DestAlpha = Final Color */
template<
	bool			BlendEnable				= false,
	bool			LogicOpEnable			= false,
	D3D12_BLEND		SrcBlend				= D3D12_BLEND_ZERO, /** RGB */
	D3D12_BLEND		DestBlend				= D3D12_BLEND_ONE,
	D3D12_BLEND_OP	BlendOp					= D3D12_BLEND_OP_ADD,
	D3D12_BLEND		SrcBlendAlpha			= D3D12_BLEND_ZERO, /** Alpha */
	D3D12_BLEND		DestBlendAlpha			= D3D12_BLEND_ONE,
	D3D12_BLEND_OP	BlendOpAlpha			= D3D12_BLEND_OP_ADD,
	D3D12_LOGIC_OP	LogicOp					= D3D12_LOGIC_OP_NOOP,
	uint8_t			RenderTargetWriteMask	= D3D12_COLOR_WRITE_ENABLE_ALL /** color channels write mask */
>
class RenderTargetBlendDesc
{
public:
	inline D3D12_RENDER_TARGET_BLEND_DESC GetDesc() const
	{
		D3D12_RENDER_TARGET_BLEND_DESC desc;
		desc.BlendEnable			= BlendEnable ? TRUE : FALSE;
		desc.LogicOpEnable			= LogicOpEnable ? TRUE : FALSE;
		desc.SrcBlend				= SrcBlend;
		desc.DestBlend				= DestBlend;
		desc.BlendOp				= BlendOp;
		desc.SrcBlendAlpha			= SrcBlendAlpha;
		desc.DestBlendAlpha			= DestBlendAlpha;
		desc.BlendOpAlpha			= BlendOpAlpha;
		desc.LogicOp				= LogicOp;
		desc.RenderTargetWriteMask	= RenderTargetWriteMask;
		return desc;
	}
};


/** */
template<
	bool AlphaToCoverageEnable = false,
	bool IndependentBlendEnable = false
>
class BlendState
{
public:
	inline D3D12_BLEND_DESC GetDesc() const
	{
		D3D12_BLEND_DESC desc;
		desc.AlphaToCoverageEnable  = AlphaToCoverageEnable;
		desc.IndependentBlendEnable = IndependentBlendEnable;
		desc.RenderTarget = RtBlendDescs;
		return desc;
	}

	D3D12_RENDER_TARGET_BLEND_DESC RtBlendDescs[8]{};
};

enum PrimitiveTopology
{
	Undefined	= D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED,
	Point		= D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT,
	Line		= D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,
	Triangle	= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
	Patch		= D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH
};

/** */
class GraphicsPipelineState
{
public:
	GraphicsPipelineState() : m_psoDesc() 
	{
		m_psoDesc.SampleMask = UINT_MAX;
		m_psoDesc.SampleDesc.Count = 1u;
	}

	void SetInputLayout(const D3D12_INPUT_ELEMENT_DESC* pIntputElementDescs, uint32_t numElements)
	{
		m_psoDesc.InputLayout.pInputElementDescs = pIntputElementDescs;
		m_psoDesc.InputLayout.NumElements = numElements;
	}

	void SetRootSignature(ID3D12RootSignature* pRootSignature)
	{
		m_psoDesc.pRootSignature = pRootSignature;
	}

	void SetVertexShader(ID3DBlob* pShaderBlob)
	{
		m_psoDesc.VS.pShaderBytecode = pShaderBlob->GetBufferPointer();
		m_psoDesc.VS.BytecodeLength = pShaderBlob->GetBufferSize();
	}

	void SetPixelShader(ID3DBlob* pShaderBlob)
	{
		m_psoDesc.PS.pShaderBytecode = pShaderBlob->GetBufferPointer();
		m_psoDesc.PS.BytecodeLength = pShaderBlob->GetBufferSize();
	}

	void SetRasterizerState(const D3D12_RASTERIZER_DESC& rasterizerDesc)
	{
		m_psoDesc.RasterizerState = rasterizerDesc;
	}

	void SetBlendState(const D3D12_BLEND_DESC& blendDesc)
	{
		m_psoDesc.BlendState = blendDesc;
	}

	void SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& depthStencilState)
	{
		m_psoDesc.DepthStencilState = depthStencilState;
	}

	void SetPrimitiveTopology(PrimitiveTopology primitiveTopology)
	{
		m_psoDesc.PrimitiveTopologyType = (D3D12_PRIMITIVE_TOPOLOGY_TYPE)primitiveTopology;
	}

	void SetNumRenderTargets(uint32_t numRenderTargets)
	{
		m_psoDesc.NumRenderTargets = numRenderTargets;
	}

	void SetRvtFormats(DXGI_FORMAT format, uint32_t index)
	{
		m_psoDesc.RTVFormats[index] = format;
	}

	/** Finalizing */
	void Finalize(ID3D12Device* pDevice)
	{
		ThrowIfFailed(pDevice->CreateGraphicsPipelineState(&m_psoDesc, IID_PPV_ARGS(&m_pipelineState)));
	}

	ID3D12PipelineState* GetPSO() const { return m_pipelineState.Get(); }

private:
	D3D12_GRAPHICS_PIPELINE_STATE_DESC m_psoDesc;
	ComPtr<ID3D12PipelineState> m_pipelineState;
};


/** @todo: implement compute commandlist */
class GraphicsCommandList
{
public:
	GraphicsCommandList(ID3D12Device* pDevice, ID3D12CommandAllocator* pCmdAllocator, ID3D12PipelineState* pPSO = nullptr)
	{
		pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE(CommandListType::Direct), pCmdAllocator, pPSO, IID_PPV_ARGS(&m_commandList));
		m_commandList->Close();
	}

	// @todo: Directly calling command from d3d12 for now, abstraction until vulkan is supported.
	ID3D12GraphicsCommandList* operator->() const { return m_commandList.Get(); }
	ID3D12GraphicsCommandList* GetCmdList() const { return m_commandList.Get(); }

private:
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
};


/** */


template<CommandListType cmdListType>
inline CommandQueue CommandQueue::CreateQueue(ID3D12Device* pDevice)
{
	D3D12_COMMAND_QUEUE_DESC queueDesc;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = cmdListType;
	return CommandQueue(pDevice, queueDesc);
}

}
