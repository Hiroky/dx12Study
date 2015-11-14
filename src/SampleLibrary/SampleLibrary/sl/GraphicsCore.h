#pragma once

#include "Common.h"
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include "RenderContext.h"

namespace sl
{
	class GraphicsCore
	{
	public:
		static void Initialize(uint width, uint height);
		static void Finalize();

		static void BeginFrame();
		static void EndFrame();
		static void Present(uint interval);
		static void ResetCommandList(RenderContext& context);
		static void ExecuteCommandList(RenderContext& context);

		static ID3D12GraphicsCommandList* CreateCommandList();

		// ‰¼
		static void GetRTVHandle(D3D12_CPU_DESCRIPTOR_HANDLE* handle);
		static ID3D12Resource* GetRenderTarget() { return m_renderTargets[m_frameIndex].Get(); }

	private:
		static const UINT FrameCount = 2;

		static D3D12_VIEWPORT m_viewport;
		static D3D12_RECT m_scissorRect;
		static Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
		static Microsoft::WRL::ComPtr<ID3D12Device> m_device;
		static Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
		static Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
		static Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
		static Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
		static Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		static Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
		static UINT m_rtvDescriptorSize;

		// App resources.
		static Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
		static D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

		// Synchronization objects.
		static UINT m_frameIndex;
		static HANDLE m_fenceEvent;
		static Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
		static UINT64 m_fenceValue;
	};
}