#pragma once


namespace sl
{

	class ApplicationCore
	{
	private:
		static HWND hWnd_;

	public:
		static void Initialize(int width, int height, const wchar_t* className);
		static void Finalize();

		static HWND GetWindowHandle() { return hWnd_; }
	};

}