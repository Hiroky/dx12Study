#include "stdafx.h"
#include "RenderContext.h"
#include "GraphicsCore.h"

namespace sl
{
	
	RenderContext::RenderContext()
	{
		commandList_ = GraphicsCore::CreateCommandList();
	}

	RenderContext::~RenderContext()
	{
		SAFE_RELEASE(commandList_);
	}


	//
	// ƒeƒXƒgˆ—
	//
	void RenderContext::TestFunc()
	{
		auto* renderTarget = GraphicsCore::GetRenderTarget();

		// Indicate that the back buffer will be used as a render target.
		commandList_->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTarget, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
		GraphicsCore::GetRTVHandle(&rtvHandle);

		// Record commands.
		const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		commandList_->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

		// Indicate that the back buffer will now be used to present.
		commandList_->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

		THROW_IF_FAILED(commandList_->Close());
	}

}
