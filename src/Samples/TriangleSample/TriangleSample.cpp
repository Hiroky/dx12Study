#include "stdafx.h"
#include "TriangleSample.h"
#include "sl/ApplicationCore.h"
#include "sl/GraphicsCore.h"
#include "sl/RenderContext.h"

namespace
{
	uint windowWidth = 1280;
	uint windowHeight = 720;
}




//
// アプリケーションエントリポイント
//
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	sl::ApplicationCore::Initialize(windowWidth, windowHeight, L"TriangleSample");
	sl::GraphicsCore::Initialize(windowWidth, windowHeight);
	sl::RenderContext context;

	MSG msg = { 0 };
	while (WM_QUIT != msg.message) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			sl::GraphicsCore::BeginFrame();
			{
				sl::GraphicsCore::ResetCommandList(context);
				context.TestFunc();
				sl::GraphicsCore::ExecuteCommandList(context);
				sl::GraphicsCore::Present(1);
			}
			sl::GraphicsCore::EndFrame();
			Sleep(1);
		}
	}

	return (int)msg.wParam;
}
