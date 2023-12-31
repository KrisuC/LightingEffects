#include "Engine.h"
#include <iostream>

Engine::Engine(uint32_t width, uint32_t height, std::wstring name)
	: m_context(width, height)
	, m_frameIndex(0)
	, m_rtvDescriptorSize(0)
	, m_title(name)
{
	WCHAR assetsPath[512];
	GetAssetsPath(assetsPath, _countof(assetsPath));
	m_assetsPath = assetsPath;
}

void Engine::OnInit()
{
	// Swapchain require hWnd, which is created after Engine::Engine()
	m_context.CreateSwapChain(Win32Application::GetHwnd(), FrameCount, m_context.GetBackBufferWidth(), m_context.GetBackBufferHeight());

	m_rtvDescriptorSize = m_context.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// Create descriptor heap
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	rtvHeapDesc.NumDescriptors = FrameCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(m_context.GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

	// Create a RTV for each frame
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (int32_t i = 0; i < FrameCount; i++)
	{
		// Getting back buffer resource handle
		ThrowIfFailed(m_context.GetSwapChain()->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])));
		m_context.GetDevice()->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, m_rtvDescriptorSize);
	}

	// Create an empty root signature
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(
		0, nullptr, // Parameters
		0, nullptr, // Static Samplers
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
	ThrowIfFailed(m_context.GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));

	// Create the PSO, compling and loading shaders
	ComPtr<ID3DBlob> vertexShader;
	ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
	uint32_t compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	uint32_t compileFlags = 0;
#endif

	// Compiling shaders
	SHADER_COMPILE_ERROR_HELPER(D3DCompileFromFile(SOURCE_PATH L"Shader.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, &pShaderErrorBlob));
	SHADER_COMPILE_ERROR_HELPER(D3DCompileFromFile(SOURCE_PATH L"Shader.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

	// Define vertex input layout
	D3D12_INPUT_ELEMENT_DESC inputElementsDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	// Describe PSO
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = { inputElementsDesc, _countof(inputElementsDesc) };
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	ThrowIfFailed(m_context.GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));

	// Create the vertex buffer
	float aspectRatio = float(m_context.GetBackBufferWidth()) / float(m_context.GetBackBufferHeight());
	Vertex triangleVertices[] =
	{
		{ { 0.0f, 0.25f * aspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
		{ { 0.25f, -0.25f * aspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
		{ { -0.25f, -0.25f * aspectRatio, 0.0f } , { 0.0f, 0.0f, 1.0f, 1.0f } }
	};

	const uint32_t vertexBufferSize = sizeof(triangleVertices);

	// Using a upload heap for simplicity
	ThrowIfFailed(m_context.GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_vertexBuffer)
	));

	// Copy the triangle data to vertex buffer
	uint8_t* pVertexDataBegin = nullptr;
	CD3DX12_RANGE readRange(0, 0); // Not intended to be read from CPU
	ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
	m_vertexBuffer->Unmap(0, nullptr);

	// Initialize vertex buffer view
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);
	m_vertexBufferView.SizeInBytes = vertexBufferSize;

	// Create synchronization objects and wait until assets being uplaod to GPU
	ThrowIfFailed(m_context.GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fenceObject)));
	m_fenceValue = 1;

	// Create an event handle to use for frame synchronization
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_fenceEvent == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}

	// Wait for command list to excute
	WaitForGpuCommandCompletion();
}

Engine::~Engine()
{
}


_Use_decl_annotations_
void Engine::ParseCommandLineArgs(wchar_t* argv[], int32_t argc)
{
}

std::wstring Engine::GetAssetFullPath(const wchar_t* assetName)
{
	return m_assetsPath + assetName;
}

void Engine::WaitForGpuCommandCompletion()
{
	const uint64_t currentFenceValue = m_fenceValue;
	m_fenceValue++;
	// After commands in the command queue before `Signal` are finished, fence will be set to `currentFenceValue`
	ThrowIfFailed(m_context.GetCommandQueue()->Signal(m_fenceObject.Get(), currentFenceValue));
	// If fence not yet reach `currentFenceValue`, set an event and wait for it
	if (m_fenceObject->GetCompletedValue() < currentFenceValue)
	{
		ThrowIfFailed(m_fenceObject->SetEventOnCompletion(currentFenceValue, m_fenceEvent));
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}
}

void Engine::OnResize(uint32_t newWidth, uint32_t newHeight)
{
}

void Engine::OnUpdate()
{
	// Record command list
	m_context.BeginFrame(m_pipelineState.Get());

	ComPtr<ID3D12GraphicsCommandList>& m_commandList = m_context.GetCommandList();

	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	m_commandList->RSSetViewports(1, &m_context.GetViewport());
	m_commandList->RSSetScissorRects(1, &m_context.GetScissorRect());

	// Indicate that backbuffer will be used as render target
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET)
	);

	// Get RTV handle
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// Record commands
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	m_commandList->DrawInstanced(3, 1, 0, 0);

	// Indicate that back buffer will be present
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT
	));

	ThrowIfFailed(m_commandList->Close());

	// Execute the command list
	ID3D12CommandList* ppCommandLists[] = { m_context.GetCommandList().Get() };
	m_context.GetCommandQueue()->ExecuteCommandLists(1, ppCommandLists);

	// Present the frame
	ThrowIfFailed(m_context.GetSwapChain()->Present(1, 0));

	WaitForGpuCommandCompletion();
	m_frameIndex = m_context.GetSwapChain()->GetCurrentBackBufferIndex();

	m_context.EndFrame();
}

void Engine::OnDestroy()
{
	WaitForGpuCommandCompletion();
	CloseHandle(m_fenceEvent);
}

void Engine::OnKeyDown(uint8_t key)
{
}

void Engine::OnKeyUp(uint8_t key)
{
}
