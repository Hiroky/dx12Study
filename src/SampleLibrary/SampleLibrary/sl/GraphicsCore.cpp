#include "stdafx.h"
#include "GraphicsCore.h"

//using namespace DirectX;
using namespace Microsoft::WRL;



namespace sl
{
	namespace
	{
		const bool useWarpDevice = false;

		// Helper function for acquiring the first available hardware adapter that supports Direct3D 12.
		// If no such adapter can be found, *ppAdapter will be set to nullptr.
		void GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter)
		{
			IDXGIAdapter1* pAdapter = nullptr;
			*ppAdapter = nullptr;

			for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &pAdapter); ++adapterIndex) {
				DXGI_ADAPTER_DESC1 desc;
				pAdapter->GetDesc1(&desc);

				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
					// Don't select the Basic Render Driver adapter.
					// If you want a software adapter, pass in "/warp" on the command line.
					continue;
				}

				// Check to see if the adapter supports Direct3D 12, but don't create the
				// actual device yet.
				if (SUCCEEDED(D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
					break;
				}
			}

			*ppAdapter = pAdapter;
		}
	}


	D3D12_VIEWPORT						GraphicsCore::m_viewport;
	D3D12_RECT							GraphicsCore::m_scissorRect;
	ComPtr<IDXGISwapChain3>				GraphicsCore::m_swapChain;
	ComPtr<ID3D12Device>				GraphicsCore::m_device;
	ComPtr<ID3D12Resource>				GraphicsCore::m_renderTargets[FrameCount];
	ComPtr<ID3D12CommandAllocator>		GraphicsCore::m_commandAllocator;
	ComPtr<ID3D12CommandQueue>			GraphicsCore::m_commandQueue;
	ComPtr<ID3D12RootSignature>			GraphicsCore::m_rootSignature;
	ComPtr<ID3D12DescriptorHeap>		GraphicsCore::m_rtvHeap;
	ComPtr<ID3D12PipelineState>			GraphicsCore::m_pipelineState;
	UINT								GraphicsCore::m_rtvDescriptorSize;
	ComPtr<ID3D12Resource>				GraphicsCore::m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW			GraphicsCore::m_vertexBufferView;
	UINT								GraphicsCore::m_frameIndex;
	HANDLE								GraphicsCore::m_fenceEvent;
	ComPtr<ID3D12Fence>					GraphicsCore::m_fence;
	UINT64								GraphicsCore::m_fenceValue;



	//
	// グラフィックス初期化
	//
	void GraphicsCore::Initialize(uint width, uint height)
	{
#if defined(_DEBUG)
		// Enable the D3D12 debug layer.
		{
			ComPtr<ID3D12Debug> debugController;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
				debugController->EnableDebugLayer();
			}
		}
#endif

		ComPtr<IDXGIFactory4> factory;
		THROW_IF_FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));

		if (useWarpDevice) {
			ComPtr<IDXGIAdapter> warpAdapter;
			THROW_IF_FAILED(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

			THROW_IF_FAILED(D3D12CreateDevice(
				warpAdapter.Get(),
				D3D_FEATURE_LEVEL_11_0,
				IID_PPV_ARGS(&m_device)
				));
		}
		else {
			ComPtr<IDXGIAdapter1> hardwareAdapter;
			GetHardwareAdapter(factory.Get(), &hardwareAdapter);

			THROW_IF_FAILED(D3D12CreateDevice(
				hardwareAdapter.Get(),
				D3D_FEATURE_LEVEL_11_0,
				IID_PPV_ARGS(&m_device)
				));
		}

		// Describe and create the command queue.
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		THROW_IF_FAILED(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

		// Describe and create the swap chain.
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferCount = FrameCount;
		swapChainDesc.BufferDesc.Width = width;
		swapChainDesc.BufferDesc.Height = height;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.OutputWindow = ApplicationCore::GetWindowHandle();
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.Windowed = TRUE;

		ComPtr<IDXGISwapChain> swapChain;
		THROW_IF_FAILED(factory->CreateSwapChain(
			m_commandQueue.Get(),		// Swap chain needs the queue so that it can force a flush on it.
			&swapChainDesc,
			&swapChain
			));

		THROW_IF_FAILED(swapChain.As(&m_swapChain));

		// This sample does not support fullscreen transitions.
		THROW_IF_FAILED(factory->MakeWindowAssociation(ApplicationCore::GetWindowHandle(), DXGI_MWA_NO_ALT_ENTER));

		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

		// Create descriptor heaps.
		{
			// Describe and create a render target view (RTV) descriptor heap.
			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
			rtvHeapDesc.NumDescriptors = FrameCount;
			rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			THROW_IF_FAILED(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

			m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		}

		// Create frame resources.
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

			// Create a RTV for each frame.
			for (UINT n = 0; n < FrameCount; n++) {
				THROW_IF_FAILED(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
				m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
				rtvHandle.Offset(1, m_rtvDescriptorSize);
			}
		}

		THROW_IF_FAILED(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));

		// Create synchronization objects.
		{
			THROW_IF_FAILED(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
			m_fenceValue = 1;

			// Create an event handle to use for frame synchronization.
			m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			if (m_fenceEvent == nullptr) {
				THROW_IF_FAILED(HRESULT_FROM_WIN32(GetLastError()));
			}
		}
	}


	//
	//
	//
	void GraphicsCore::Finalize()
	{
	}


	//
	// フレーム描画開始
	//
	void GraphicsCore::BeginFrame()
	{
		// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
		// This is code implemented as such for simplicity. More advanced samples 
		// illustrate how to use fences for efficient resource usage.

		// Signal and increment the fence value.
		const UINT64 fence = m_fenceValue;
		THROW_IF_FAILED(m_commandQueue->Signal(m_fence.Get(), fence));
		m_fenceValue++;

		// Wait until the previous frame is finished.
		if (m_fence->GetCompletedValue() < fence) {
			THROW_IF_FAILED(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}

		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();


		// Command list allocators can only be reset when the associated 
		// command lists have finished execution on the GPU; apps should use 
		// fences to determine GPU execution progress.
		THROW_IF_FAILED(m_commandAllocator->Reset());
	}


	//
	// フレーム描画終了
	//
	void GraphicsCore::EndFrame()
	{
	}


	//
	// 表示
	//
	void GraphicsCore::Present(uint interval)
	{	
		THROW_IF_FAILED(m_swapChain->Present(interval, 0));
	}


	//
	// コマンドリストのリセット
	//
	void GraphicsCore::ResetCommandList(RenderContext& context)
	{
		// However, when ExecuteCommandList() is called on a particular command 
		// list, that command list can then be reset at any time and must be before 
		// re-recording.
		THROW_IF_FAILED(context.GetCommandList()->Reset(m_commandAllocator.Get(), m_pipelineState.Get()));
	}


	//
	// コマンド実行
	//
	void GraphicsCore::ExecuteCommandList(RenderContext& context)
	{
		ID3D12CommandList* ppCommandLists[] = { context.GetCommandList() };
		m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	}


	//
	// コマンドリスト生成
	//
	ID3D12GraphicsCommandList* GraphicsCore::CreateCommandList()
	{
		ID3D12GraphicsCommandList* cmdList;
		THROW_IF_FAILED(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&cmdList)));

		// Command lists are created in the recording state, but there is nothing
		// to record yet. The main loop expects it to be closed, so close it now.
		THROW_IF_FAILED(cmdList->Close());

		return cmdList;
	}



	void GraphicsCore::GetRTVHandle(D3D12_CPU_DESCRIPTOR_HANDLE* handle)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
		handle->ptr = rtvHandle.ptr;
	}
}