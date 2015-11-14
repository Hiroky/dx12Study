#pragma once

#include "Common.h"
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_4.h>

namespace sl
{

	class RenderContext
	{
	private:
		ID3D12GraphicsCommandList* commandList_;

	public:
		RenderContext();
		~RenderContext();

		void TestFunc();

		ID3D12GraphicsCommandList* GetCommandList() { return commandList_; }
	};

}